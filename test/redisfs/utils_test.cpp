#include "redisfs/utils.hpp"

#include <limits>
#include <vector>

#include <cstdint>

#include <gtest/gtest.h>

template<typename T>
class SerializeTest : public testing::Test {
    public:
    static std::vector<T> _range_;
};

TYPED_TEST_SUITE_P( SerializeTest );

TYPED_TEST_P( SerializeTest, SingleValue ) {

    std::vector<TypeParam> & targets = SerializeTest<TypeParam>::_range_;

    for ( TypeParam value : targets ) {

        std::string buf;
        redisfs::appendBytes( buf, value );

        TypeParam extracted;
        redisfs::extractBytes( buf, extracted, 0 );

        EXPECT_EQ( extracted, value );

    }

}

TYPED_TEST_P( SerializeTest, MultipleValues ) {

    std::vector<TypeParam> & targets = SerializeTest<TypeParam>::_range_;

    std::string buf;
    for ( TypeParam value : targets ) {
        redisfs::appendBytes( buf, value );
    }

    size_t idx = 0;
    for ( TypeParam value : targets ) {
        ASSERT_LT( idx, buf.size() );
        TypeParam extracted;
        idx = redisfs::extractBytes( buf, extracted, idx );
        EXPECT_EQ( extracted, value );
    }
    EXPECT_EQ( idx, buf.size() );

}

REGISTER_TYPED_TEST_SUITE_P( SerializeTest, SingleValue, MultipleValues );

// All these macros are probably excessive but fuck it why not

using IntTypes = ::testing::Types<uint8_t,  int8_t,
                                  uint16_t, int16_t,
                                  uint32_t, int32_t,
                                  uint64_t, int64_t>;
INSTANTIATE_TYPED_TEST_SUITE_P( Integers, SerializeTest, IntTypes );

#define SERIALIZE_TEST( type, ... ) template<> std::vector<type> SerializeTest<type>::_range_{ 0, std::numeric_limits<type>::max(), std::numeric_limits<type>::min(), __VA_ARGS__ }

#define SMALL_POSITIVE_INTS  1,  3,  9,  100,  120
#define SMALL_NEGATIVE_INTS -1, -3, -9, -100, -120
#define POSITIVE_INTS SMALL_POSITIVE_INTS,  145,  15151,  125151
#define NEGATIVE_INTS SMALL_NEGATIVE_INTS, -145, -15151, -125151
#define SMALL_INTS SMALL_POSITIVE_INTS, SMALL_NEGATIVE_INTS
#define INTS POSITIVE_INTS, NEGATIVE_INTS

#define SERIALIZE_SMALL_UINT_TEST( type ) SERIALIZE_TEST( type, SMALL_POSITIVE_INTS )
#define SERIALIZE_SMALL_INT_TEST( type ) SERIALIZE_TEST( type, SMALL_INTS )
#define SERIALIZE_UINT_TEST( type ) SERIALIZE_TEST( type, POSITIVE_INTS )
#define SERIALIZE_INT_TEST( type ) SERIALIZE_TEST( type, INTS )

SERIALIZE_SMALL_UINT_TEST( uint8_t );
SERIALIZE_SMALL_INT_TEST( int8_t );
SERIALIZE_SMALL_UINT_TEST( uint16_t );
SERIALIZE_SMALL_INT_TEST( int16_t );
SERIALIZE_UINT_TEST( uint32_t );
SERIALIZE_INT_TEST(  int32_t );
SERIALIZE_UINT_TEST( uint64_t );
SERIALIZE_INT_TEST(  int64_t );

using FloatTypes = ::testing::Types<float, double>;
INSTANTIATE_TYPED_TEST_SUITE_P( Floats, SerializeTest, FloatTypes );

#define FLOATS( type ) INTS, 10000.0, -100000.0, std::numeric_limits<type>::infinity(), -std::numeric_limits<type>::infinity()
#define SERIALIZE_FLOAT_TEST( type ) SERIALIZE_TEST( type, FLOATS( type ) )

SERIALIZE_FLOAT_TEST( float );
SERIALIZE_FLOAT_TEST( double );
