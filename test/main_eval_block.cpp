#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>

#include <cstdlib>

#include "redisfs/kvstore.h"
#include "redisfs/redis.h"
#include "redisfs/utils.hpp"

constexpr size_t MIN_SIZE = redisfs::Size<size_t>::KIBI;
constexpr size_t MAX_SIZE = redisfs::Size<size_t>::MEBI * 64;
constexpr size_t RUNS = 100;
constexpr size_t SEED = 4201337;

using Clock = std::chrono::steady_clock;

static void randomData( std::string & buf, const size_t size ) {

    static std::default_random_engine generator( SEED );
    static std::uniform_int_distribution<char> distribution( 
        std::numeric_limits<char>::min(), 
        std::numeric_limits<char>::max() 
    );

    buf.clear();
    buf.reserve( size );
    for ( size_t i = 0; i < size; i++ ) {
        buf += distribution( generator );
    }
    
}

static void timeFunction( std::function<void()> func, const std::string & type, const size_t size ) {

    Clock::duration total = Clock::duration::zero();
    std::vector<int64_t> times;
    for ( size_t i = 0; i < RUNS; i++ ) {

        Clock::time_point start = Clock::now();
        func();
        Clock::time_point end = Clock::now();
        Clock::duration time = end - start;

        total += time;
        times.push_back( std::chrono::duration_cast<std::chrono::nanoseconds>( time ).count() );

    } 

    int64_t mean = std::chrono::duration_cast<std::chrono::nanoseconds>( total / RUNS ).count();

    std::sort( times.begin(), times.end() );
    int64_t median = times[times.size() / 2];

    std::cerr << size << " byte " << type << ": " << mean << "ns mean, " << median << "ns median" << std::endl;
    std::cout << type << "," << size << "," << mean << "," << median << std::endl;

}

void testWrite( redisfs::KVStore & store ) {

    for ( size_t size = MIN_SIZE; size <= MAX_SIZE; size *= 2 ) {

        std::string key = std::to_string( size );
        std::string data;
        randomData( data, size );
        timeFunction( [ &store, &key, &data ]() -> void { store.set( key, data ); }, "write", size );

    }

}

void testRead( redisfs::KVStore & store ) {

    for ( size_t size = MIN_SIZE; size <= MAX_SIZE; size *= 2 ) {

        std::string key = std::to_string( size );
        std::string data;
        timeFunction( [ &store, &key, &data ]() -> void { data = *store.get( key ); }, "read", size );

    }

}

int main( int argc, char ** argv ) {

    if ( argc < 2 ) {
        throw std::runtime_error( "Must provide a store type" );
    }
    std::string storeType( argv[1] );

    std::shared_ptr<redisfs::KVStore> store;
    if ( storeType == "memory" ) {
        store = std::make_shared<redisfs::MemoryStore>();
    } else if ( argc < 3 ) {
        throw std::runtime_error( "Must provide an URI" );
    } else {
        const std::string uri( argv[2] );
        std::cerr << "Using cluster at URI " << uri << " for evaluation" << std::endl;
        const sw::redis::ConnectionOptions connectionOptions( uri );
        if ( storeType == "cluster" ) {
            store = std::make_shared<redisfs::redis::RedisClusterStore>( connectionOptions );
        } else if ( storeType == "redis" ) {
            store = std::make_shared<redisfs::redis::RedisStore>( connectionOptions );
        } else {
            throw std::runtime_error( "Invalid store type" );
        }
    }

    store->clear();
    std::cout << "type,size,mean,median" << std::endl;
    testWrite( *store );
    testRead( *store );
    store->clear();

    return EXIT_SUCCESS;

}