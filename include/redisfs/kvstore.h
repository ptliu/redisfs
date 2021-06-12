#pragma once

#include <optional>
#include <string_view>

namespace redisfs {

    class KVStore {

        public:
        virtual std::optional<std::string> get( const std::string_view & key ) = 0;
        virtual bool set( const std::string_view & key, const std::string_view & value ) = 0;
        virtual bool del( const std::string_view & key ) = 0;
        virtual void clear() = 0;

        // List operations
        virtual std::optional<std::string> get( const std::string_view & key, const size_t idx ) = 0;
        virtual size_t push( const std::string_view & key, const std::string_view & value ) = 0;

    };

}