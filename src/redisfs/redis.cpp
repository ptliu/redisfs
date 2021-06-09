#include "redisfs/redis.h"

/* RedisStore */

redisfs::redis::RedisStore::RedisStore( const sw::redis::ConnectionOptions & options ) : redis( options ) {}

std::optional<std::string> redisfs::redis::RedisStore::get( const std::string_view & key ) {
    return redis.get( key );
}
bool redisfs::redis::RedisStore::set( const std::string_view & key, const std::string_view & value ) {
    return redis.set( key, value );
}
bool redisfs::redis::RedisStore::del( const std::string_view & key ) {
    return redis.del( key ) == 1;
}

/* RedisClusterStore */

redisfs::redis::RedisClusterStore::RedisClusterStore( const sw::redis::ConnectionOptions & options ) : cluster( options ) {}

std::optional<std::string> redisfs::redis::RedisClusterStore::get( const std::string_view & key ) {
    return cluster.get( key );
}
bool redisfs::redis::RedisClusterStore::set( const std::string_view & key, const std::string_view & value ) {
    return cluster.set( key, value );
}
bool redisfs::redis::RedisClusterStore::del( const std::string_view & key ) {
    return cluster.del( key ) == 1;
}