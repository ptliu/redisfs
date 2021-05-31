#define FUSE_USE_VERSION 31

#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <sw/redis++/redis++.h>
#include <sw/redis++/redis_cluster.h>
#include <iostream>
#include "nlohmann/json.hpp"


#define BLOCK_SIZE 512000000


using namespace sw::redis;
using json = nlohmann::json;


static struct redis_fs_info {
  ConnectionOptions* options;
  RedisCluster* cluster;
  
} redis_fs_info;

inline size_t calc_offset_idx(off_t offset){
  if(offset < 0){
    return 0;
  }
  return offset / BLOCK_SIZE;
}

inline std::vector<size_t> file_blocks(std::vector<size_t>& blocklist, off_t offset, size_t size){
  std::vector<size_t> blocks;
  if(offset < 0){
    return blocks;
  }

  size_t end = (size_t)offset + size;
  for(size_t i = (size_t)offset; i < end; i += BLOCK_SIZE){
    size_t idx = calc_offset_idx(i);
    if(idx < blocklist.size()){
      blocks.push_back(blocklist[i]);
    
    }
  }
  return blocks;
}

//required fuse functions

static void *test_init(struct fuse_conn_info *conn, struct fuse_config *cfg){
  //TODO
  //(void) conn;
  //cfg->kernel_cache = 0;
  static ConnectionOptions connection_options;
  connection_options.host = "127.0.0.1";  // Required.
  connection_options.port = 7000; // Optional. The default port is 6379.
  static RedisCluster cluster1(connection_options);
  
  redis_fs_info.options = &connection_options;
  redis_fs_info.cluster = &cluster1;
  return (void*)&redis_fs_info;
}

/*
static int test_getattr(const char *path, struct stat *stbuf)
{
  int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
  } else if (strcmp(path+1, options.filename) == 0) {
    stbuf->st_mode = S_IFREG | 0444;
    stbuf->st_nlink = 1;
    stbuf->st_size = strlen(options.contents);
  } else
    res = -ENOENT;

  return res;
}
*/

static int test_getattr(const char *path, struct stat *stbuf,
       struct fuse_file_info *fi)
{
  (void) fi;
  int res = 0;
  std::string filename(path);
  Optional<std::string> val = redis_fs_info.cluster->get(filename);
  memset(stbuf, 0, sizeof(struct stat));
  if(val){
    json file_attr = json::parse(*val);
    stbuf->st_mode = file_attr["st_mode"];
    stbuf->st_nlink = file_attr["st_nlink"];
    
  } else {
    if (strcmp(path, "/") == 0) {
      stbuf->st_mode = S_IFDIR | 0755;
      stbuf->st_nlink = 2;
      return 0;
    }
    
    res = -ENOENT;
  }
  

  return res;
}

static int test_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
       off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
  (void) offset;
  (void) fi;

  int res = 0;
  std::string filename(path);
  Optional<std::string> val = redis_fs_info.cluster->get(filename);
  if(val){
    json file_attr = json::parse(*val);  
  } else {
    res = -ENOENT;
  }

  return res;
}

static int test_open(const char *path, struct fuse_file_info *fi)
{
  int res = 0;
  std::string filename(path);
  Optional<std::string> val = redis_fs_info.cluster->get(filename);
  if(val){
    json file_attr = json::parse(*val); 
  } else {
    //create the file as it doesn't exist
    json file_attr;
    file_attr["blocks"] = {};
    file_attr["st_mode"] = 0755;
    file_attr["st_nlink"] = 1;
    redis_fs_info.cluster->set(filename, file_attr.dump());
  }

  return res;
}

static int test_read(const char *path, char *buf, size_t size, off_t offset,
          struct fuse_file_info *fi)
{
  (void) fi;
  int bytes = 0;
  std::string filename(path);
  Optional<std::string> val = redis_fs_info.cluster->get(filename);

  if(val){
    json file_attr = json::parse(*val);  
    if(!file_attr.contains("blocks")){
      return bytes; //no blocks to read
    }
    std::vector<size_t> blocklist = file_attr["blocks"].get<std::vector<size_t>>();
    std::vector<size_t> blocks = file_blocks(blocklist, offset, size);
    for(auto it = blocks.begin(); it != blocks.end(); it++){
      Optional<std::string> block = redis_fs_info.cluster->get(std::to_string(*it));
      if(block){
        
        memcpy(buf + bytes, (*block).c_str(), (*block).size());
        bytes += (*block).size();
        
      } else {
        //this is weird but i guess we do nothing
      }
    }
  } else {
    bytes = -ENOENT;
  }
  return bytes;
}

static int test_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){

  int bytes = 0;
  std::string filename(path);
  Optional<std::string> val = redis_fs_info.cluster->get(filename);
  if(val){
    json file_attr = json::parse(*val); 
    if(!file_attr.contains("blocks")){
      //create empty list of blocks
      file_attr["blocks"] = {};
    }
    std::vector<size_t> blocklist = file_attr["blocks"].get<std::vector<size_t>>();
    std::vector<size_t> blocks = file_blocks(blocklist, offset, size);
    size_t buffer_offset = 0;
    size_t list_idx = 0;
    for(auto it = blocks.begin(); it != blocks.end(); it++){
      Optional<std::string> block = redis_fs_info.cluster->get(std::to_string(*it));
      if(block){
        std::string block_string = std::string(*block);
        if(buffer_offset == 0){
    
          //special case first block
          size_t first_block_offset = offset % BLOCK_SIZE;
          size_t block_len = std::min(size - first_block_offset, BLOCK_SIZE - first_block_offset);
          std::string modified_block = block_string.replace(first_block_offset, block_len, std::string(buf, block_len));
          size_t block_hash = std::hash<std::string>{}(modified_block);

          //put block back in block store
          redis_fs_info.cluster->set(std::to_string(block_hash), modified_block);
          blocklist[list_idx] = block_hash;
          bytes += block_len;
          if((size_t)bytes >= size){ //bytes is always positive so this is ok
            break;
          }
        } else {
          
          size_t block_len = std::min(size - bytes, (size_t)BLOCK_SIZE); //TODO
          std::string modified_block = block_string.replace(0, block_len, std::string(buf + bytes, block_len));
          size_t block_hash = std::hash<std::string>{}(modified_block);
          //put block back in block store
          redis_fs_info.cluster->set(std::to_string(block_hash), modified_block);
          blocklist[list_idx] = block_hash;
          bytes += block_len;
          if((size_t)bytes >= size){ //bytes is always positive so this is ok
            break;
          }
        }
        bytes += (*block).size();
        
      } else {
        //this is weird and shouldn't happen but i guess we do nothing
      }
      list_idx++;
    }
      //append to file
    for(;(size_t)bytes <= size;){
      size_t block_len = std::min(size - bytes, (size_t)BLOCK_SIZE); //TODO
      std::string new_block = std::string(buf + bytes, block_len);
      size_t block_hash = std::hash<std::string>{}(new_block);
      //put block back in block store
      redis_fs_info.cluster->set(std::to_string(block_hash), new_block);
      blocklist.push_back(block_hash);

      bytes += block_len;
    }
    file_attr["blocks"] = blocklist;
    redis_fs_info.cluster->set(filename, file_attr.dump());
  } else {
    bytes = -ENOENT;
  }
 
  return bytes; //TODO 
}


static const struct fuse_operations test_oper {
  .getattr = test_getattr,
  .open = test_open,
  .read = test_read,
  .write = test_write,
  .readdir = test_readdir,
  .init = test_init,
};


int main(int argc, char * argv[]){

  ConnectionOptions connection_options;
  connection_options.host = "127.0.0.1";  // Required.
  connection_options.port = 7000; // Optional. The default port is 6379.
  RedisCluster cluster1(connection_options);

  int ret;

  ret = fuse_main(argc, argv, &test_oper, NULL);
  return ret;
}
