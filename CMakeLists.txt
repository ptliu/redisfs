cmake_minimum_required(VERSION 3.16)

set(CMAKE_MESSAGE_LOG_LEVEL STATUS)

# <------------ Configure CMake --------------->
include(FetchContent)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules" ${CMAKE_MODULE_PATH})

project(CSE223A)
set(CMAKE_CXX_STANDARD 17)

add_executable(redisfs "")

target_include_directories(redisfs PUBLIC include/)
add_subdirectory(src/)

# <------------ add hiredis dependency --------------->
FetchContent_Declare(
  hiredis
  GIT_REPOSITORY https://github.com/redis/hiredis.git
  GIT_TAG        b6f86f38c2bbf0caa63d489174ac3a9777b97807
)
FetchContent_MakeAvailable(hiredis)
target_link_libraries(redisfs PRIVATE hiredis)

# <------------ add redis-plus-plus dependency -------------->
FetchContent_Declare(
  redis++
  GIT_REPOSITORY https://github.com/sewenew/redis-plus-plus.git
  GIT_TAG        1.2.3
)
FetchContent_MakeAvailable(redis++)
target_link_libraries(redisfs PRIVATE redis++)

# <------------ add fuse3 dependency -------------->
find_package(FUSE3 REQUIRED)
target_link_libraries(redisfs PRIVATE FUSE3::FUSE3)

# <------------ add json dependency -------------->
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent.git
  GIT_TAG        v3.9.1
)
FetchContent_MakeAvailable(json)
target_link_libraries(redisfs PRIVATE nlohmann_json::nlohmann_json)