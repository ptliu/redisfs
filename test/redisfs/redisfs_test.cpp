#include "redisfs/redisfs.h"

#include <unordered_map>

#include <gtest/gtest.h>

#include "redisfs/utils.hpp"

using Size = redisfs::Size<size_t>;
constexpr size_t BLOCK_SIZE = 64 * Size::KILO;

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

    virtual bool set( const std::string_view & key, const std::string_view & value ) override {

        std::string k( key ), v( value );

        map[k] = v;
        return true;
    
    }

};

class RedisFSTest : public ::testing::Test {

    private:
    static std::pair<char *, size_t> makeItem( size_t size ) {

        void * data = malloc( size ); // Random enough?
        return std::make_pair( ( char * ) data, size );

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

        char * buf = ( char * ) malloc( it.second.second );
        fs.read( it.first.c_str(), buf, it.second.second, 0 );
        EXPECT_EQ( memcmp( buf, it.second.first, it.second.second ), 0 );
        free( buf );

    }

}
