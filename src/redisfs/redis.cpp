#include "redisfs/redis.h"

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