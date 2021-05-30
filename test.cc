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
#include <nlohmann/json.hpp>


#define BLOCK_SIZE 512000000

using namespace sw::redis;
using json = nlohmann::json;

static struct options {
  const char *filename;
  const char *contents;
  int show_help;
} options; //fuse options


static struct redis_fs_info {
  ConnectionOptions* options;
  RedisCluster* cluster;
  
} redis_fs_info;


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
    
  } else {
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

  return 0;
}

static int test_open(const char *path, struct fuse_file_info *fi)
{

  std::string filename(path);
  Optional<std::string> val = redis_fs_info.cluster->get(filename);
  if(val){
    json file_attr = json::parse(*val);  
  } else {
    res = -ENOENT;
  }
  if ((fi->flags & O_ACCMODE) != O_RDONLY)
    return -EACCES;

  return 0;
}

static int test_read(const char *path, char *buf, size_t size, off_t offset,
          struct fuse_file_info *fi)
{
  size_t len;
  (void) fi;
  if(strcmp(path+1, options.filename) != 0)
    return -ENOENT;

  len = strlen(options.contents);
  if (offset < len) {
    if (offset + size > len)
      size = len - offset;
    memcpy(buf, options.contents + offset, size);
  } else
    size = 0;

  return size;
}

static int test_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
  return 0; //TODO 
}


static const struct fuse_operations test_oper {
  .getattr = test_getattr,
  .open = test_open,
  .read = test_read,
  .write = test_write,
  .readdir = test_readdir,
};


int main(int argc, char * argv[]){

  ConnectionOptions connection_options;
  connection_options.host = "127.0.0.1";  // Required.
  connection_options.port = 7000; // Optional. The default port is 6379.
  RedisCluster cluster1(connection_options);
  //cluster1.set("k1", "v1");
  auto v = cluster1.get("k1");
  if(v){
    std::cout << *v << std::endl;
  }
  else {
    std::cout << "Got null" << std::endl;
  }

}
