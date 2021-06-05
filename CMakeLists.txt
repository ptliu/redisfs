cmake_minimum_required(VERSION 3.16)

# <------------ Configure CMake --------------->
include(FetchContent)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules" ${CMAKE_MODULE_PATH})

project(CSE223A)
set(CMAKE_CXX_STANDARD 17)

add_executable(redisfs "")

target_include_directories(redisfs PUBLIC include/)
add_subdirectory(src/)

# <------------ add hiredis dependency --------------->
find_package(hiredis CONFIG REQUIRED)
target_link_libraries(redisfs PRIVATE hiredis::hiredis)

# <------------ add redis-plus-plus dependency -------------->
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(redisfs PRIVATE redis++)

# <------------ add fuse3 dependency -------------->
find_package(FUSE3 REQUIRED)
target_link_libraries(redisfs PRIVATE FUSE3::FUSE3)

# <------------ add json dependency -------------->
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(redisfs PRIVATE nlohmann_json::nlohmann_json)