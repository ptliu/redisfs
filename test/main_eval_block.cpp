#include <cstdlib>

#include "redisfs/kvstore.h"
#include "redisfs/redis.h"

void testRead( redisfs::KVStore & store ) {

    // TODO

}

void testWrite( redisfs::KVStore & store ) {

    // TODO

}

int main( void ) {

    std::string host = "127.0.0.1"; // Required.
    int         port = 7000;        // Optional. The default port is 6379.
    const std::string & uri = host + ":" + std::to_string( port );

    const sw::redis::ConnectionOptions connectionOptions( uri );
    redisfs::redis::RedisStore store( connectionOptions );

    testRead( store );

    store.clear();

    testWrite( store );

    store.clear();

    return EXIT_SUCCESS;

}