#pragma once

#include <memory>
#include <string>

#include <sys/stat.h>

#include "redisfs/kvstore.h"
#include "redisfs/metadata.hpp"

//for direct cluster access
#include <sw/redis++/redis++.h>
#include <sw/redis++/redis_cluster.h>


namespace redisfs {
    class RedisFS {

        public:
            RedisFS( const std::shared_ptr<KVStore> & store, const size_t blockSize );

            const size_t blockSize;

            int utimens( const char * path, const struct timespec tv[2] );
            int access( const char * path, int mode );
            int open( const char * path );
            int release( const char * path );
            int create( const char * path, mode_t mode );
            int getattr( const char * const path, struct stat * stbuf );
            int readdir( const char * const path, void * const buf, const off_t offset );
            int read( const char * const path, char * const buf, const size_t size, const off_t offset );
            int write( const char * const path, const char * const buf, const size_t size, const off_t offset );
        
        private:
            std::shared_ptr<KVStore> store;

    };

}