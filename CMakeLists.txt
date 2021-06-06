cmake_minimum_required(VERSION 3.16)

# <------------ Configure CMake --------------->
include(FetchContent)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules" ${CMAKE_MODULE_PATH})
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_files/scripts/buildsystems/vcpkg.cmake
    CACHE STRING "Vcpkg toolchain file")

# <------------ Configure vcpkg integration --------------->
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/overlays")

project(CSE223A)

add_executable(redisfs "")
target_compile_features(redisfs PUBLIC cxx_std_17)

target_include_directories(redisfs PUBLIC include/)
add_subdirectory(src/)

# <------------ add redis-plus-plus dependency -------------->
find_package(redis++ CONFIG REQUIRED)
target_link_libraries(redisfs PRIVATE redis++)

# <------------ add fuse3 dependency -------------->
find_package(FUSE3 REQUIRED)
target_link_libraries(redisfs PRIVATE FUSE3::FUSE3)

# <------------ add json dependency -------------->
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(redisfs PRIVATE nlohmann_json::nlohmann_json)