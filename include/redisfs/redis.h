#pragma once

#include <sw/redis++/redis++.h>
#include <sw/redis++/redis_cluster.h>

#include "redisfs/kvstore.h"

namespace redisfs {

    namespace redis {

        /**
         * @brief Store that redirects everything to a Redis server.
         */
        class RedisStore : public KVStore {

            private:
            sw::redis::Redis redis;

            public:
            RedisStore( const sw::redis::ConnectionOptions & options );

            std::optional<std::string> get( const std::string_view & key ) override;
            bool set( const std::string_view & key, const std::string_view & value ) override;
            bool del( const std::string_view & key ) override;
            void clear() override;

        };

        /**
         * @brief Store that redirects everything to a Redis cluster.
         */
        class RedisClusterStore : public KVStore {

            private:
            sw::redis::RedisCluster cluster;

            public:
            RedisClusterStore( const sw::redis::ConnectionOptions & options );

            std::optional<std::string> get( const std::string_view & key ) override;
            bool set( const std::string_view & key, const std::string_view & value ) override;
            bool del( const std::string_view & key ) override;
            void clear() override;

        };

    }

}