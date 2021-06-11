#include "redisfs/redisfs.h"

#include <iostream>

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstddef>
#include <cassert>
#include <chrono>
#include <fcntl.h>

#include "redisfs/exceptions.h"
#include "redisfs/utils.hpp"

redisfs::RedisFS::RedisFS( const std::shared_ptr<KVStore> & store, const size_t blockSize ) : 
    store( store ), blockSize( blockSize ) {}


int redisfs::RedisFS::utimens(const char * path, const struct timespec tv[2]){
  std::string filename( path );
  std::optional<std::string> val = store->get( filename );
  if ( !val ) {
    return -ENOENT;
  }

  Metadata metadata( *val );
  //ignores time passed in and just sets to current time
  std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
  if(tv == NULL){
    metadata.st.st_ctim.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>( time.time_since_epoch() ).count() / 1000000000;
    metadata.st.st_ctim.tv_sec = std::chrono::duration_cast<std::chrono::seconds>( time.time_since_epoch() ).count();
    metadata.st.st_atim.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>( time.time_since_epoch() ).count() / 1000000000;
    metadata.st.st_atim.tv_sec = std::chrono::duration_cast<std::chrono::seconds>( time.time_since_epoch() ).count();
    metadata.st.st_mtim.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>( time.time_since_epoch() ).count() / 1000000000;
    metadata.st.st_mtim.tv_sec = std::chrono::duration_cast<std::chrono::seconds>( time.time_since_epoch() ).count();
    return 0;
  }
  metadata.st.st_atim.tv_nsec = tv[0].tv_nsec;
  metadata.st.st_atim.tv_sec = tv[0].tv_sec;
  metadata.st.st_mtim.tv_nsec = tv[1].tv_nsec;
  metadata.st.st_mtim.tv_sec = tv[1].tv_sec;
  metadata.st.st_ctim.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>( time.time_since_epoch() ).count() / 1000000000;
  metadata.st.st_ctim.tv_sec = std::chrono::duration_cast<std::chrono::seconds>( time.time_since_epoch() ).count();
  return 0;
}

int redisfs::RedisFS::access(const char * path, int mode){
  return 0; //no permissions enforced
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

  return 0; // TODO?

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

int redisfs::RedisFS::create(const char *path, mode_t mode){
  std::string filename( path );
  std::optional<std::string> val = store->get(filename);
  if ( val ) {
    return -EEXIST;
  }
  else {
    //file doesn't exist, create it
    Metadata metadata;
    metadata.st.st_mode = S_IFREG | 0755;
    metadata.st.st_nlink = 1;
    //std::string serial_metadata;
    //metadata.serialize();
    store->set(filename, metadata.serialize());
    //TODO: populate the metadata
    return 0;
  }
  return 0;
}

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
      stbuf->st_mode = S_IFDIR | 0777;
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
  if ( !val ) {
    return -ENOENT;
  }
  // TODO ?
  return 0;

}

int redisfs::RedisFS::read( const char * const path, char * const buf, const size_t size, const off_t offset ) {

  if ( offset < 0 ) { // TODO: Is this a valid input?
    throw std::domain_error( "Negative offset" );
  }

  std::string filename( path );
  std::optional<std::string> val = store->get( filename );

  if ( !val ) {
    return -ENOENT;
  }

  int bytes = 0;
  Metadata metadata( *val );

  size_t blockIdx = offset / blockSize;
  off_t blockOffset = offset % blockSize;
  while ( bytes < size && blockIdx < metadata.blocks.size() ) {

    const std::string blockID = std::to_string( metadata.blocks[blockIdx] );
    std::optional<std::string> block = store->get( blockID );
    if ( !block ) {
      throw CorruptFilesystemError( "Block " + blockID + " is missing." );    
    }

    size_t readSize = std::min( block->size() - blockOffset, size - bytes );
    memcpy( buf + bytes, block->c_str() + blockOffset, readSize );

    bytes += readSize;
    blockIdx++;
    blockOffset = 0;

  }

  // TODO: Update metadata?

  return bytes;

}

int redisfs::RedisFS::write( const char * const path, const char * const buf, const size_t size, const off_t offset ) {

  if ( offset < 0 ) { // TODO: Is this a valid input?
    throw std::domain_error( "Negative offset" );
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
