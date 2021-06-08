#pragma once

#include <iomanip>
#include <iostream>
#include <vector>

#include <sys/stat.h>

#define PRINT_STRUCT_FIRST_FIELD( field ) "." << #field << "=" << st.field
#define PRINT_STRUCT_FIELD( field ) ", " << PRINT_STRUCT_FIRST_FIELD( field )

inline std::ostream & operator<<( std::ostream & os, const struct stat & st ) {

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

template <typename T>
inline std::ostream & operator<<( std::ostream & os, const std::vector<T> & vec ) {

    os << "[";
    int idx = 0;
    for ( const T & val : vec ) {
        if ( idx > 0 ) {
            os << ", ";
        }
        os << val;
        idx++;
    }
    return os << "]";

}

inline void printData( std::ostream & os, const char * const buf, const std::string & name, const size_t size, const size_t maxSize = 100 ) {
    
    os << name << ": ";
    for ( size_t i = 0; i < std::min( size, maxSize ); i++ ) {
        os << std::hex << std::setfill( '0' ) << std::setw( 2 ) << +( ( unsigned char ) buf[i] );
    }
    os << std::endl;

}