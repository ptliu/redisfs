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

  if ( offset < 0 || blocklist.empty() ) {
    return {};
  }

  BlockIndex first = std::min( blocklist.size(), calcOffsetIdx( offset ) );
  BlockIndex last  = std::min( blocklist.size(), calcOffsetIdx( offset + size - 1 ) + 1 );
  return std::vector<BlockIndex>( blocklist.begin() + first, blocklist.begin() + last );

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

int redisfs::RedisFS::getattr( const char * const path, struct stat * stbuf ) {

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

int redisfs::RedisFS::readdir( const char * const path, void * const buf, const off_t offset ) {

  std::string filename( path );
  std::optional<std::string> val = store->get( filename );
  if ( val ) {
    // TODO ?
    return 0;
  } else {
    return -ENOENT;
  }

}

int redisfs::RedisFS::read( const char * const path, char * const buf, const size_t size, const off_t offset ) {

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

int redisfs::RedisFS::write( const char * const path, const char * const buf, const size_t size, const off_t offset ) {

  if ( offset < 0 ) { // TODO: Is this a valid input?
    throw std::runtime_error( "Negative offset" );
  }

  std::string filename( path );
  std::optional<std::string> val = store->get( filename );
  if ( !val ) {
    return -ENOENT;
  }

  size_t bytes = 0;
  Metadata metadata( *val );

  // Write to existing blocks
  size_t blockIdx = offset / blockSize;
  off_t blockOffset = offset % blockSize;
  while ( bytes < size && blockIdx < metadata.blocks.size() ) {
    
    size_t newSize = std::min( blockSize - blockOffset, size - bytes );
    BlockIndex oldIdx = metadata.blocks[blockIdx];
    std::string oldID = std::to_string( oldIdx );

    std::string blockString;
    if ( newSize == blockSize ) { // Replace whole block
      blockString = std::string( buf + bytes, blockSize );
    } else { // Replace part of block
      std::optional<std::string> block = store->get( oldID );
      if ( !block ) {
        throw CorruptFilesystemError( "Block " + oldID + " is missing." );
      }
      blockString = std::string( *block );
      size_t oldSize = std::min( blockString.size() - blockOffset, newSize );
      blockString.replace( blockOffset, oldSize, buf + bytes, newSize );
    }
    BlockIndex newIdx = std::hash<std::string>{}( blockString );

    // Put block back in block store
    store->set( std::to_string( newIdx ), blockString ); // TODO: deal with conflicts
    if ( oldIdx != newIdx ) { // Gotta check just in case
      store->del( oldID );
      metadata.blocks[blockIdx] = newIdx;
    }

    bytes += newSize;
    blockIdx++;
    blockOffset = 0;

  }

  //append to file
  while ( bytes < size ) {

    size_t newSize = std::min( size - bytes, blockSize );
    std::string blockString( buf + bytes, newSize );
    BlockIndex newIdx = std::hash<std::string>{}( blockString ); // TODO: deal with conflicts
    // Put block in block store
    store->set( std::to_string( newIdx ), blockString );
    metadata.blocks.push_back( newIdx );

    bytes += newSize;

  }

  // TODO: Update metadata

  store->set( filename, metadata.serialize() );

  return bytes;

}
