#include "redisfs/redisfs.h"

#include <limits>
#include <random>
#include <unordered_map>

#include <gtest/gtest.h>

#include "redisfs/utils.hpp"

using Size = redisfs::Size<size_t>;

constexpr size_t BLOCK_SIZE = 64 * Size::KILO;
constexpr size_t SEED = 4201337;

class MemoryStore : public redisfs::KVStore {

    public:
    std::unordered_map<std::string, std::string> map; // Public since its for testing anyway

    std::optional<std::string> get( const std::string_view & key ) override {
        
        std::string k( key );
        auto it = map.find( k );
        if ( it == map.end() ) {
            return std::nullopt;
        } else {
            return it->second;
        }

    }

    bool set( const std::string_view & key, const std::string_view & value ) override {

        std::string k( key ), v( value );
        map[k] = v;
        return true;
    
    }

    bool del( const std::string_view & key ) override {
                
        std::string k( key );
        return map.erase( k ) == 1;

    }

};

static char * randomData( const size_t size ) {

    static std::default_random_engine generator( SEED );
    static std::uniform_int_distribution<char> distribution( 
        std::numeric_limits<char>::min(), 
        std::numeric_limits<char>::max() 
    );

    char * const data = ( char * ) malloc( size );
    for ( size_t i = 0; i < size; i++ ) {
        data[i] = distribution( generator );
    }

    return data;
    
}

class RedisFSTest : public ::testing::Test {

    private:
    static std::pair<char *, size_t> makeItem( size_t size ) {

        return std::make_pair( randomData( size ), size );

    }

    static std::unordered_map<std::string, std::pair<char *, size_t>> makeItems() {

        std::unordered_map<std::string, std::pair<char *, size_t>> items;

        items["/"] = makeItem( 10 * Size::KILO );
        items["/foo/bar"] = makeItem( 35 * Size::MEGA );
        items["/foo"] = makeItem( 100 );
        items["/tmp"] = makeItem( 139 * Size::MEGA );

        return items;

    }

    protected:
    const std::unordered_map<std::string, std::pair<char *, size_t>> baseItems;
    redisfs::RedisFS fs;

    RedisFSTest() : baseItems( makeItems() ), fs( std::make_shared<MemoryStore>(), BLOCK_SIZE ) {}

    void SetUp() override {

        for ( auto it : baseItems ) {
            fs.open( it.first.c_str() );
            fs.write( it.first.c_str(), it.second.first, it.second.second, 0 );
        }

    }

    void TearDown() override {

        for ( auto it : baseItems ) {
            free( it.second.first );
        }

    }

};

TEST_F( RedisFSTest, TestRead ) {

    for ( auto it : baseItems ) {

        char * const buf = ( char * ) malloc( it.second.second );
        fs.read( it.first.c_str(), buf, it.second.second, 0 );
        EXPECT_EQ( memcmp( buf, it.second.first, it.second.second ), 0 );

        free( buf );

    }

}

TEST_F( RedisFSTest, TestOverwrite ) {

    for ( auto it : baseItems ) {

        const size_t modSize = it.second.second / 4;
        const off_t modIdx = it.second.second / 2;
        char * const mod = randomData( modSize );

        fs.write( it.first.c_str(), mod, modSize, modIdx );

        char * const res = ( char * ) malloc( it.second.second );
        memcpy( res, it.second.first, it.second.second );

        char * const buf = ( char * ) malloc( it.second.second );
        fs.read( it.first.c_str(), buf, it.second.second, 0 );
        EXPECT_EQ( memcmp( buf, res, it.second.second ), 0 );

        free( mod );
        free( res );
        free( buf );

    }

}

TEST_F( RedisFSTest, TestAppend ) {

    for ( auto it : baseItems ) {

        char * const mod = randomData( it.second.second );

        fs.write( it.first.c_str(), mod, it.second.second, it.second.second );

        const size_t newSize = it.second.second * 2;
        char * const res = ( char * ) malloc( newSize );
        memcpy( res, it.second.first, it.second.second );
        memcpy( res + it.second.second, mod, it.second.second );

        char * const buf = ( char * ) malloc( newSize );
        fs.read( it.first.c_str(), buf, newSize, 0 );
        EXPECT_EQ( memcmp( buf, res, newSize ), 0 );

        free( mod );
        free( res );
        free( buf );

    }

}

TEST_F( RedisFSTest, TestOverwriteAndAppend ) {

    for ( auto it : baseItems ) {

        const off_t modIdx = it.second.second / 2;
        char * const mod = randomData( it.second.second );

        fs.write( it.first.c_str(), mod, it.second.second, modIdx );

        const size_t newSize = modIdx + it.second.second;
        char * const res = ( char * ) malloc( newSize );
        memcpy( res, it.second.first, modIdx );
        memcpy( res + modIdx, mod, it.second.second );

        char * const buf = ( char * ) malloc( newSize );
        fs.read( it.first.c_str(), buf, newSize, 0 );
        EXPECT_EQ( memcmp( buf, res, newSize ), 0 );

        free( mod );
        free( res );
        free( buf );

    }

}

TEST_F( RedisFSTest, TestReadOffset ) {

    for ( auto it : baseItems ) {

        const off_t idx = it.second.second / 2;
        const size_t size = it.second.second / 4;

        char * const buf = ( char * ) malloc( size );
        fs.read( it.first.c_str(), buf, size, idx );
        EXPECT_EQ( memcmp( buf, it.second.first + idx, size ), 0 );

        free( buf );

    }

}

TEST_F( RedisFSTest, TestReadInBounds ) {

    constexpr unsigned long CANARY = 0x69FE42A1;

    for ( auto it : baseItems ) {

        char * const fullbuf = ( char * ) malloc( it.second.second + 2 * sizeof( unsigned long ) );

        unsigned long * const startCanary = ( unsigned long * ) fullbuf;
        char * const buf = fullbuf + sizeof( unsigned long );
        unsigned long * const endCanary = ( unsigned long * ) ( buf + it.second.second );
        
        *startCanary = CANARY;
        *endCanary = CANARY;
        
        fs.read( it.first.c_str(), buf, it.second.second, 0 );
        EXPECT_EQ( *startCanary, CANARY );
        EXPECT_EQ( *endCanary, CANARY );

        free( fullbuf );

    }

}
