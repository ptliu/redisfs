#pragma once

#include <string>

#include <arpa/inet.h>

namespace redisfs {

    /**
     * @brief SI prefixes for convenience.
     * 
     * @tparam T The type to use.
     */
    template<typename T>
    struct Size {

        static constexpr T KILO = 1000;
        static constexpr T MEGA = KILO * KILO;
        static constexpr T GIGA = KILO * MEGA;
        static constexpr T TERA = KILO * GIGA;

        static constexpr T KIBI = 1024;
        static constexpr T MEBI = KIBI * KIBI;
        static constexpr T GIBI = KIBI * MEBI;
        static constexpr T TEBI = KIBI * GIBI;

    };

    static const bool IS_BIG_ENDIAN = htonl( 1 ) == 1;

    template<typename T>
    inline void appendBytes( std::string & buf, const T val ) {

        char * bytes = ( char * ) &val;
        for ( size_t i = 0; i < sizeof( T ); i++ ) {
            buf += bytes[IS_BIG_ENDIAN ? i : sizeof( T ) - 1 - i];
        }

    }

    template<typename T>
    inline size_t extractBytes( const std::string & buf, T & val, const size_t idx ) {

        char * bytes = ( char * ) &val;
        for ( size_t i = 0; i < sizeof( T ); i++ ) {
            bytes[IS_BIG_ENDIAN ? i : sizeof( T ) - 1 - i] = buf[idx + i];
        }
        return idx + sizeof( T );

    }

}