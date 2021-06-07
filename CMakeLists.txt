cmake_minimum_required(VERSION 3.16)

# <------------ Configure CMake --------------->
include(FetchContent)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules" "${CMAKE_MODULE_PATH}")

# <------------ Configure vcpkg integration --------------->
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_files/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/overlays")

project(CSE223A)

add_library(redisfs_lib)
target_compile_features(redisfs_lib PUBLIC cxx_std_17)
target_include_directories(redisfs_lib PUBLIC include/)

# <------------ add redis-plus-plus dependency -------------->
find_package(redis++ CONFIG REQUIRED)
target_link_libraries(redisfs_lib PUBLIC redis++::redis++_static)

add_executable(redisfs)
target_link_libraries(redisfs PRIVATE redisfs_lib)

# <------------ add fuse3 dependency -------------->
find_package(FUSE3 REQUIRED)
target_link_libraries(redisfs PRIVATE FUSE3::FUSE3)

add_subdirectory(src/)
