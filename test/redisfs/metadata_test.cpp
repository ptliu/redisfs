#include "redisfs/metadata.hpp"

#include <gtest/gtest.h>

static struct stat stats[] {

    {
        .st_dev = 0,
        .st_ino = 0,
#ifndef __x86_64__
        .st_mode = 0,
        .st_nlink = 0,
#else
        .st_nlink = 0,
        .st_mode = 0,
#endif
        .st_uid = 0,
        .st_gid = 0,
        .st_rdev = 0,
        .st_size = 0,
        .st_blksize = 0,
        .st_blocks = 0,

        .st_atim = {
            .tv_sec = 0,
            .tv_nsec = 0
        },
        .st_mtim = {
            .tv_sec = 0,
            .tv_nsec = 0
        },
        .st_ctim = {
            .tv_sec = 0,
            .tv_nsec = 0
        }
    },
    {
        .st_dev = 15,
        .st_ino = 10,
#ifndef __x86_64__
        .st_mode = 94,
        .st_nlink = 1,
#else
        .st_nlink = 14,
        .st_mode = 82,
#endif
        .st_uid = 12,
        .st_gid = 85,
        .st_rdev = 24,
        .st_size = 872,
        .st_blksize = 92,
        .st_blocks = 52,

        .st_atim = {
            .tv_sec = 195,
            .tv_nsec = 811
        },
        .st_mtim = {
            .tv_sec = 128,
            .tv_nsec = 851
        },
        .st_ctim = {
            .tv_sec = 58,
            .tv_nsec = 235
        }
    },

};

#define PRINT_STRUCT_FIRST_FIELD( field ) "." << #field << "=" << st.field
#define PRINT_STRUCT_FIELD( field ) ", " << PRINT_STRUCT_FIRST_FIELD( field )

std::ostream & operator<<( std::ostream & os, const struct stat & st ) {

    return os << "stat{"
              << PRINT_STRUCT_FIRST_FIELD( st_dev )
              << PRINT_STRUCT_FIELD( st_ino )
              << PRINT_STRUCT_FIELD( st_mode )
              << PRINT_STRUCT_FIELD( st_nlink )
              << PRINT_STRUCT_FIELD( st_uid )
              << PRINT_STRUCT_FIELD( st_gid )
              << PRINT_STRUCT_FIELD( st_rdev )
              << PRINT_STRUCT_FIELD( st_size )
              << PRINT_STRUCT_FIELD( st_blksize )
              << PRINT_STRUCT_FIELD( st_blocks )

              << PRINT_STRUCT_FIELD( st_atim.tv_nsec )
              << PRINT_STRUCT_FIELD( st_atim.tv_sec )

              << PRINT_STRUCT_FIELD( st_mtim.tv_nsec )
              << PRINT_STRUCT_FIELD( st_mtim.tv_sec )

              << PRINT_STRUCT_FIELD( st_ctim.tv_nsec )
              << PRINT_STRUCT_FIELD( st_ctim.tv_sec )
              << "}";

}

class SerializeStatTest : public testing::TestWithParam<struct stat> {};

void compareStats( const struct stat & actual, const struct stat & expected ) {

    /* Note: Commented out lines are fields that we don't care about. */
    // EXPECT_EQ( actual.st_dev    , expected.st_dev     );
    // EXPECT_EQ( actual.st_ino    , expected.st_ino     );
    EXPECT_EQ( actual.st_mode   , expected.st_mode    );
    EXPECT_EQ( actual.st_nlink  , expected.st_nlink   );
    EXPECT_EQ( actual.st_uid    , expected.st_uid     );
    EXPECT_EQ( actual.st_gid    , expected.st_gid     );
    // EXPECT_EQ( actual.st_rdev   , expected.st_rdev    );
    EXPECT_EQ( actual.st_size   , expected.st_size    );
    // EXPECT_EQ( actual.st_blksize, expected.st_blksize );
    // EXPECT_EQ( actual.st_blocks , expected.st_blocks  );

    EXPECT_EQ( actual.st_atim.tv_nsec, expected.st_atim.tv_nsec );
    EXPECT_EQ( actual.st_atim.tv_sec , expected.st_atim.tv_sec  );

    EXPECT_EQ( actual.st_mtim.tv_nsec, expected.st_mtim.tv_nsec );
    EXPECT_EQ( actual.st_mtim.tv_sec , expected.st_mtim.tv_sec  );

    EXPECT_EQ( actual.st_ctim.tv_nsec, expected.st_ctim.tv_nsec );
    EXPECT_EQ( actual.st_ctim.tv_sec , expected.st_ctim.tv_sec  );

}

TEST_P( SerializeStatTest, RestoresProperly ) {

    const struct stat & value = GetParam();

    std::string buf;
    redisfs::serialize( value, buf );

    EXPECT_EQ( buf.size(), redisfs::SERIALIZED_STAT_SIZE );

    struct stat restored;
    redisfs::deserialize( restored, buf );

    SCOPED_TRACE( "" );
    compareStats( restored, value );

}

INSTANTIATE_TEST_SUITE_P( Main, SerializeStatTest, testing::ValuesIn( stats ) );