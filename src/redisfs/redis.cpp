#include "redisfs/redis.h"

#include <iostream>
#include <regex>
#include <stdexcept>

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
void redisfs::redis::RedisStore::clear() {
    redis.flushdb();
}
std::optional<std::string> redisfs::redis::RedisStore::get( const std::string_view & key, const size_t idx ) {
    std::vector<std::string> values;
    redis.lrange( key, idx, idx, std::back_inserter( values ) );
    if ( values.size() == 0 ) {
        return std::nullopt;
    } else {
        return values[0];
    }
}
size_t redisfs::redis::RedisStore::push( const std::string_view & key, const std::string_view & value ) {
    return redis.rpush( key , value ); 
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
void redisfs::redis::RedisClusterStore::clear() {

    const std::string clusterString = cluster.redis( "0", false ).command<std::string>( "cluster", "nodes" );
    std::stringstream ss( clusterString );
    std::string line;
    while ( std::getline( ss, line, '\n' ) ) {

        static const std::regex re( "(\\d{1,3}(?:\\.\\d{1,3}){3}:\\d+)@\\d+ \\S*(slave|master)\\S*" );
        std::smatch m;
        if ( !std::regex_search( line, m, re ) ) {
            throw std::runtime_error( "Line did not contain a node address." );
        }
        if ( m[2] == "master" ) { // Node is a master
            std::string uri = "tcp://" + m[1].str();
            std::cerr << "Sending reset to " << uri << std::endl;
            
            sw::redis::Redis( sw::redis::ConnectionOptions( uri ) ).flushdb();
        }

    }

}
std::optional<std::string> redisfs::redis::RedisClusterStore::get( const std::string_view & key, const size_t idx ) {
    std::vector<std::string> values;
    cluster.lrange( key, idx, idx, std::back_inserter( values ) );
    if ( values.size() == 0 ) {
        return std::nullopt;
    } else {
        return values[0];
    }
}
size_t redisfs::redis::RedisClusterStore::push( const std::string_view & key, const std::string_view & value ) {
    return cluster.rpush( key , value ); 
}