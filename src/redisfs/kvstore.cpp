#include "redisfs/kvstore.h"

std::optional<std::string> redisfs::MemoryStore::get( const std::string_view & key ) {
    
    std::string k( key );
    auto it = map.find( k );
    if ( it == map.end() ) {
        return std::nullopt;
    } else {
        return it->second;
    }

}

bool redisfs::MemoryStore::set( const std::string_view & key, const std::string_view & value ) {

    std::string k( key ), v( value );
    map[k] = v;
    return true;

}

bool redisfs::MemoryStore::del( const std::string_view & key ) {
            
    std::string k( key );
    return map.erase( k ) == 1;

}

void redisfs::MemoryStore::clear() {

    map.clear();
    listMap.clear();

}

std::optional<std::string> redisfs::MemoryStore::get( const std::string_view & key, const size_t idx ) {

    std::string k( key );
    auto it = listMap.find( k );
    if ( it == listMap.end() || it->second.size() <= idx ) {
        return std::nullopt;
    } else  {
        return it->second[idx];
    }

}

size_t redisfs::MemoryStore::push( const std::string_view & key, const std::string_view & value ) {

    std::string k( key ), v( value );
    std::vector<std::string> & vec = listMap[k];
    vec.push_back( v );
    return vec.size();

}
