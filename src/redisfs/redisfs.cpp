#include "redisfs/redisfs.h"

#include <iostream>

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstddef>
#include <cassert>

#include <fcntl.h>

#include "redisfs/exceptions.h"
#include "redisfs/utils.hpp"

redisfs::RedisFS::RedisFS( const std::shared_ptr<KVStore> & store, const size_t blockSize ) : 
    store( store ), blockSize( blockSize ) {}

inline redisfs::BlockIndex redisfs::RedisFS::calcOffsetIdx( off_t offset ) const {

  if ( offset < 0 ) {
    return 0;
  } else {
    return offset / blockSize;
  }

}

inline std::vector<redisfs::BlockIndex> redisfs::RedisFS::fileBlocks( std::vector<BlockIndex> & blocklist, off_t offset, size_t size ) const {

  std::vector<BlockIndex> blocks;
  if ( offset < 0 ) {
    return blocks;
  }

  size_t end = ( size_t ) offset + size;
  for ( size_t i = ( size_t ) offset; i < end; i += blockSize ) {

    size_t idx = calcOffsetIdx( i );
    if ( idx < blocklist.size() ) {
      blocks.push_back( blocklist[i] );
    }

  }
  return blocks;

}

int redisfs::RedisFS::open( const char * path ) {

  std::string filename( path );
  std::optional<std::string> val = store->get( filename );
  if ( !val ) {
    //create the file as it doesn't exist
    Metadata metadata;
    metadata.st.st_mode = 0755;
    metadata.st.st_nlink = 1;

    store->set( filename, metadata.serialize() );
  }

  return 0;

}

int redisfs::RedisFS::release( const char * path ) {

  return 0; //nothing needs to be done

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

int redisfs::RedisFS::getattr( const char * path, struct stat * stbuf ) {

  std::string filename( path );
  std::optional<std::string> val = store->get(filename);
  if ( val ) {
    Metadata metadata( *val );
    *stbuf = metadata.st;
    return 0;
  } else {
    memset( stbuf, 0, sizeof( struct stat ) );
    if ( strcmp( path, "/" ) == 0 ) {
      stbuf->st_mode = S_IFDIR | 0755;
      stbuf->st_nlink = 2;
      return 0;
    } else {
      return -ENOENT;
    }
  }
  
}

int redisfs::RedisFS::readdir( const char * path, void * buf, off_t offset ) {

  std::string filename( path );
  std::optional<std::string> val = store->get( filename );
  if ( val ) {
    // TODO ?
    return 0;
  } else {
    return -ENOENT;
  }

}

int redisfs::RedisFS::read( const char * path, char * buf, size_t size, off_t offset ) {

  std::string filename( path );
  std::optional<std::string> val = store->get( filename );

  if ( val ) {
    Metadata metadata( *val );
    if ( metadata.blocks.empty() ) {
      return 0; // no blocks to read
    }

    int bytes = 0;
    std::vector<BlockIndex> blocks = fileBlocks( metadata.blocks, offset, size );
    for ( auto it = blocks.begin(); it != blocks.end(); it++ ) {

      const std::string blockID = std::to_string( *it );
      std::optional<std::string> block = store->get( blockID );
      if ( block ) {
        memcpy( buf + bytes, block->c_str(), block->size() );
        bytes += block->size();
      } else {
        throw CorruptFilesystemError( "Block " + blockID + " is missing." );
      }

    }
    return bytes;
  } else {
    return -ENOENT;
  }

}

int redisfs::RedisFS::write( const char * path, const char * buf, size_t size, off_t offset ) {

  std::string filename( path );
  std::optional<std::string> val = store->get( filename );
  if ( val ) {
    int bytes = 0;
    Metadata metadata( *val );
    std::vector<BlockIndex> blocks = fileBlocks( metadata.blocks, offset, size );

    size_t buffer_offset = 0;
    size_t list_idx = 0;
    for ( auto it = blocks.begin(); it != blocks.end(); it++ ) {

      std::string blockID = std::to_string( *it );
      std::optional<std::string> block = store->get( blockID );
      if ( block ) {
        std::string block_string = std::string( *block );
        if ( buffer_offset == 0 ) {

          // special case first block
          size_t first_block_offset = offset % blockSize;
          size_t block_len = std::min( size - first_block_offset, blockSize - first_block_offset );
          std::string modified_block = block_string.replace( first_block_offset, block_len, std::string( buf, block_len ) );
          size_t block_hash = std::hash<std::string>{}( modified_block );

          // put block back in block store
          store->set( std::to_string( block_hash ), modified_block );
          metadata.blocks[list_idx] = block_hash;
          bytes += block_len;
          if ( ( size_t ) bytes >= size ) { // bytes is always positive so this is ok
            break;
          }
        } else {

          size_t block_len = std::min( size - bytes, ( size_t ) blockSize ); // TODO
          std::string modified_block = block_string.replace( 0, block_len, std::string( buf + bytes, block_len ) );
          size_t block_hash = std::hash<std::string>{}( modified_block );
          // put block back in block store
          store->set( std::to_string( block_hash ), modified_block );
          metadata.blocks[list_idx] = block_hash;
          bytes += block_len;
          if ( ( size_t ) bytes >= size ) { //bytes is always positive so this is ok
            break;
          }
        }
        bytes += block->size();
      } else {
        throw CorruptFilesystemError( "Block " + blockID + " is missing." );
      }
      list_idx++;

    }

    //append to file
    for ( ; (size_t )bytes <= size; ) {

      size_t block_len = std::min( size - bytes, ( size_t ) blockSize ); //TODO
      std::string new_block = std::string( buf + bytes, block_len );
      size_t block_hash = std::hash<std::string>{}( new_block );
      //put block back in block store
      store->set( std::to_string( block_hash ), new_block );
      metadata.blocks.push_back( block_hash );

      bytes += block_len;

    }

    store->set( filename, metadata.serialize() );

    return bytes;
  } else {
    return -ENOENT;
  }

}
