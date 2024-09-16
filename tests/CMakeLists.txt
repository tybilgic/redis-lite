# tests/CMakeLists.txt

enable_testing()

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_LIBRARIES
    redis-lite-core
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})

add_executable(DataStoreTests  DataStoreTest.cpp)
target_link_libraries(DataStoreTests PRIVATE ${TEST_LIBRARIES})
add_test(NAME DataStoreTests COMMAND DataStoreTests )

add_executable(RESPStringTests RESPStringTest.cpp)
target_link_libraries(RESPStringTests PRIVATE ${TEST_LIBRARIES})
add_test(NAME RESPStringTests COMMAND RESPStringTests)
