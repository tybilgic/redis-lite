cmake_minimum_required(VERSION 3.10)
project(redis-lite)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(redis-lite src/DataStore.cpp src/RESPString.cpp src/Server.cpp tests/RESPStringTest.cpp tests/DataStoreTest.cpp)
target_link_libraries(redis-lite gtest gtest_main pthread)

enable_testing()

add_test(NAME RESPStringTest COMMAND redis-lite)
add_test(NAME DataStoreTest COMMAND redis-lite)