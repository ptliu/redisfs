#pragma once

#include <string>

#include <sys/stat.h>

#include <sw/redis++/redis++.h>
#include <sw/redis++/redis_cluster.h>

#include "redisfs/metadata.hpp"

namespace redisfs {

    namespace redis = sw::redis;

    class RedisFS {

        public:
            RedisFS( const std::string & uri, const size_t blockSizes );
            RedisFS( const std::string & host, const int port, const size_t blockSize );

            const size_t blockSize;

            int open( const char * path );
            int release( const char * path );

            int getattr( const char * path, struct stat * stbuf );
            int readdir( const char * path, void * buf, off_t offset );
            int read( const char * path, char * buf, size_t size, off_t offset );
            int write( const char * path, const char * buf, size_t size, off_t offset );
        
        private:
            const redis::ConnectionOptions connectionOptions;
            redis::RedisCluster cluster;

            inline BlockIndex calcOffsetIdx( off_t offset ) const;
            inline std::vector<BlockIndex> fileBlocks( std::vector<BlockIndex> & blocklist, off_t offset, size_t size ) const;

    };

}