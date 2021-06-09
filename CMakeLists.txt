cmake_minimum_required(VERSION 3.16)

# <------------ Configure CMake --------------->
include(FetchContent)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules" "${CMAKE_MODULE_PATH}")

# <------------ Configure vcpkg integration --------------->
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_files/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/overlays")

project(CSE223A)

# <------------ Configure shared library -------------->
add_library(redisfs_lib)
target_compile_features(redisfs_lib PUBLIC cxx_std_17)
target_include_directories(redisfs_lib PUBLIC include/)

# Redis++ dependency
find_package(redis++ CONFIG REQUIRED)
target_link_libraries(redisfs_lib PUBLIC redis++::redis++_static)

# <------------ Configure main executable -------------->
add_executable(redisfs)
target_link_libraries(redisfs PRIVATE redisfs_lib)

# FUSE3 dependency
find_package(FUSE3 REQUIRED)
target_link_libraries(redisfs PRIVATE FUSE3::FUSE3)

add_subdirectory(src/)

# <------------ Configure test executable -------------->
add_executable(redisfs_test)
target_link_libraries(redisfs_test PRIVATE redisfs_lib)

# Gtest dependency
find_package(GTest CONFIG REQUIRED)
target_link_libraries(redisfs_test PRIVATE GTest::gtest_main)

# <------------ Configure block-level evaluation executable -------------->
add_executable(redisfs_eval_block)
target_link_libraries(redisfs_eval_block PRIVATE redisfs_lib)

add_subdirectory(test/)

# Discover tests with CMake
include(GoogleTest)
gtest_discover_tests(redisfs_test)