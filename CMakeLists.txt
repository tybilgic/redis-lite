cmake_minimum_required(VERSION 3.10)
project(redis-lite)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include_directories(include)

set(CORE_SOURCES
    src/DataStore.cpp
    src/Server.cpp
    src/RESPString.cpp
    src/RESPParser.cpp)

add_library(redis-lite-core STATIC ${CORE_SOURCES})

add_executable(redis-lite-server src/main.cpp)
target_link_libraries(redis-lite-server PRIVATE redis-lite-core ${CMAKE_THREAD_LIBS_INIT})

add_subdirectory(tests)
