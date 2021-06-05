# FindFUSE3.cmake
#
# Finds the FUSE3 library.
#
# This will define the following variables
#
#    FUSE3_FOUND
#    FUSE3_INCLUDE_DIRS
#    FUSE3_LIBRARIES
#    FUSE3_VERSION
#
# and the following imported targets
#
#    FUSE3::FUSE3
#

IF (FUSE3_INCLUDE_DIR AND FUSE3_LIBRARY)
    set(FUSE3_FIND_QUIETLY TRUE)
ENDIF ()

# get hints from module
find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_FUSE3 QUIET fuse3)

# find includes
find_path (FUSE3_INCLUDE_DIR 
        NAMES fuse3/fuse.h
        HINTS ${PC_FUSE3_INCLUDEDIR} ${PC_FUSE3_INCLUDE_DIRS}
        )

# find lib
find_library (FUSE3_LIBRARY
        NAMES fuse3
        HINTS ${PC_FUSE3_LIBDIR} ${PC_FUSE3_LIBRARY_DIRS}
        )

IF (PC_FUSE3_FOUND)
        set (FUSE3_VERSION_STRING "${PC_FUSE3_VERSION}")
ELSE ()
        set (FUSE3_VERSION_STRING "?")
ENDIF ()

mark_as_advanced (FUSE3_INCLUDE_DIR FUSE3_LIBRARY)

include ("FindPackageHandleStandardArgs")
find_package_handle_standard_args ("FUSE3"
        REQUIRED_VARS FUSE3_INCLUDE_DIR FUSE3_LIBRARY
        VERSION_VAR FUSE3_VERSION_STRING
        )

IF (FUSE3_FOUND)
    set (FUSE3_INCLUDE_DIRS ${FUSE_INCLUDE_DIR})
    set (FUSE3_LIBRARIES ${FUSE_LIBRARY})
ENDIF ()


IF (FUSE3_FOUND AND NOT TARGET FUSE::FUSE)
    add_library(FUSE3::FUSE3 INTERFACE IMPORTED)
    set_target_properties(FUSE3::FUSE3 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${FUSE3_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${FUSE3_LIBRARY}"
            )
ENDIF ()