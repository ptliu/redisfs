#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

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

    class MemoryStore : public redisfs::KVStore {

        public:
        std::optional<std::string> get( const std::string_view & key ) override;
        bool set( const std::string_view & key, const std::string_view & value ) override;
        bool del( const std::string_view & key ) override;
        void clear() override;

        std::optional<std::string> get( const std::string_view & key, const size_t idx ) override;
        size_t push( const std::string_view & key, const std::string_view & value ) override;

        private:
        std::unordered_map<std::string, std::string> map;
        std::unordered_map<std::string, std::vector<std::string>> listMap;

    };

}