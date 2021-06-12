#include <chrono>
#include <functional>
#include <iostream>
#include <memory>

#include <cstdlib>

#include "redisfs/kvstore.h"
#include "redisfs/redis.h"
#include "redisfs/utils.hpp"

constexpr size_t MIN_SIZE = redisfs::Size<size_t>::KIBI;
constexpr size_t MAX_SIZE = redisfs::Size<size_t>::MEBI * 256;
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

static void timeFunction( std::function<void()> func, const std::string & title ) {

    Clock::duration total = Clock::duration::zero();
    for ( size_t i = 0; i < RUNS; i++ ) {

        Clock::time_point start = Clock::now();
        func();
        Clock::time_point end = Clock::now();
        total += end - start;

    } 
    Clock::duration average = total / RUNS;
    std::cout << "Average time for " << title << ": " << std::chrono::duration_cast<std::chrono::nanoseconds>( average ).count() << "ns" << std::endl;

}

void testWrite( redisfs::KVStore & store ) {

    for ( size_t size = MIN_SIZE; size <= MAX_SIZE; size *= 2 ) {

        std::string key = std::to_string( size );
        std::string data;
        randomData( data, size );
        timeFunction( [ &store, &key, &data ]() -> void { store.set( key, data ); }, key + " byte write" );

    }

}

void testRead( redisfs::KVStore & store ) {

    for ( size_t size = MIN_SIZE; size <= MAX_SIZE; size *= 2 ) {

        std::string key = std::to_string( size );
        std::string data;
        timeFunction( [ &store, &key, &data ]() -> void { data = *store.get( key ); }, key + " byte read" );

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
        const std::string uri( argv[1] );
    } else if ( argc < 3 ) {
        throw std::runtime_error( "Must provide an URI" );
    } else {
        const std::string uri( argv[2] );
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
    testWrite( *store );
    testRead( *store );
    store->clear();

    return EXIT_SUCCESS;

}