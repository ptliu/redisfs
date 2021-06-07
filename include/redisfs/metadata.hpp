#pragma once

#include <vector>

#include <cstring>

#include <sys/stat.h>

#include "redisfs/utils.hpp"

namespace redisfs {

    static const struct stat _ref = {};
    constexpr size_t SERIALIZED_STAT_SIZE = sizeof( _ref.st_mode ) + 
                                            sizeof( _ref.st_nlink ) +
                                            sizeof( _ref.st_uid ) +
                                            sizeof( _ref.st_gid ) +
                                            sizeof( _ref.st_size ) +
                                            sizeof( _ref.st_atim.tv_nsec ) +
                                            sizeof( _ref.st_atim.tv_sec ) +
                                            sizeof( _ref.st_mtim.tv_nsec ) +
                                            sizeof( _ref.st_mtim.tv_sec ) +
                                            sizeof( _ref.st_ctim.tv_nsec ) +
                                            sizeof( _ref.st_ctim.tv_sec );

#if defined( __GNUC__ ) && ( __GNUC__ > 11 || ( __GNUC__ == 11 && __GNUC_MINOR__ >= 1 ) )
    #define SKIP_RESERVE_CHECK
#endif

    inline void reserve( std::string & s, size_t size ) {

    #ifdef SKIP_RESERVE_CHECK
        s.reserve( size )
    #else
        // Needing this check is dumb but until GCC 11.1 string::reserve could
        // reduce the capacity
        if ( s.size() < size ) {
            s.reserve( size );
        }
    #endif

    }

    inline void serialize( const struct stat & stbuf, std::string & serialized ) {

        serialized.clear();
        reserve( serialized, SERIALIZED_STAT_SIZE );

        appendBytes( serialized, stbuf.st_mode );
        appendBytes( serialized, stbuf.st_nlink );
        appendBytes( serialized, stbuf.st_uid );
        appendBytes( serialized, stbuf.st_gid );
        appendBytes( serialized, stbuf.st_size );

        appendBytes( serialized, stbuf.st_atim.tv_nsec );
        appendBytes( serialized, stbuf.st_atim.tv_sec );
        appendBytes( serialized, stbuf.st_mtim.tv_nsec );
        appendBytes( serialized, stbuf.st_mtim.tv_sec );
        appendBytes( serialized, stbuf.st_ctim.tv_nsec );
        appendBytes( serialized, stbuf.st_ctim.tv_sec );

    }

    inline void deserialize( struct stat & stbuf, const std::string & serialized ) {

        memset( &stbuf, 0, sizeof( struct stat ) );

        size_t idx = 0;
        idx = extractBytes( serialized, stbuf.st_mode, idx );
        idx = extractBytes( serialized, stbuf.st_nlink, idx );
        idx = extractBytes( serialized, stbuf.st_uid, idx );
        idx = extractBytes( serialized, stbuf.st_gid, idx );
        idx = extractBytes( serialized, stbuf.st_size, idx );

        idx = extractBytes( serialized, stbuf.st_atim.tv_nsec, idx );
        idx = extractBytes( serialized, stbuf.st_atim.tv_sec, idx );
        idx = extractBytes( serialized, stbuf.st_mtim.tv_nsec, idx );
        idx = extractBytes( serialized, stbuf.st_mtim.tv_sec, idx );
        idx = extractBytes( serialized, stbuf.st_ctim.tv_nsec, idx );
        idx = extractBytes( serialized, stbuf.st_ctim.tv_sec, idx );

    }

    using BlockIndex = size_t;

    typedef struct Metadata {

        struct stat st;
        std::vector<BlockIndex> blocks;

        inline void serialize( std::string & serialized ) {

            size_t nblocks = blocks.size();
            reserve( serialized, SERIALIZED_STAT_SIZE + sizeof( size_t ) + nblocks * sizeof( BlockIndex ) );

            redisfs::serialize( st, serialized );

            appendBytes( serialized, nblocks );
            for ( size_t i = 0; i < nblocks; i++ ) {
                appendBytes( serialized, blocks[i] );
            }

        }

        inline void deserialize( const std::string & serialized ) {

            redisfs::deserialize( st, serialized );

            size_t nblocks;
            size_t idx = SERIALIZED_STAT_SIZE;
            idx = extractBytes( serialized, nblocks, idx );

            blocks.resize( nblocks );
            for ( size_t i = 0; i < nblocks; i++ ) {
                idx = extractBytes( serialized, blocks[i], idx );
            }

        }

        inline std::string serialize() {

            std::string serialized;
            serialize( serialized );
            return serialized;

        }

        inline Metadata() {
            memset( &st, 0, sizeof( struct stat ) );
        }
        inline Metadata( std::string & serialized ) {
            deserialize( serialized );
        }


    } Metadata;

}