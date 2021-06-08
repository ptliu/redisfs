#pragma once

#include <memory>
#include <string>

#include <sys/stat.h>

#include "redisfs/kvstore.h"
#include "redisfs/metadata.hpp"

namespace redisfs {
    class RedisFS {

        public:
            RedisFS( const std::shared_ptr<KVStore> & store, const size_t blockSize );

            const size_t blockSize;

            int open( const char * path );
            int release( const char * path );

            int getattr( const char * const path, struct stat * stbuf );
            int readdir( const char * const path, void * const buf, const off_t offset );
            int read( const char * const path, char * const buf, const size_t size, const off_t offset );
            int write( const char * const path, const char * const buf, const size_t size, const off_t offset );
        
        private:
            std::shared_ptr<KVStore> store;

            inline BlockIndex calcOffsetIdx( off_t offset ) const;
            inline std::vector<BlockIndex> fileBlocks( std::vector<BlockIndex> & blocklist, off_t offset, size_t size ) const;

    };

}