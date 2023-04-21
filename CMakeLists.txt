cmake_minimum_required(VERSION 3.22)
project(SearchEngine)

set(CMAKE_CXX_STANDARD 20)

set(Boost_INCLUDE_DIR C:/boost_1_82_0/)
set(Boost_LIBRARY_DIR C:/boost_1_82_0/libs)
set(JSON_BuildTests OFF CACHE INTERNAL "")

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

find_package(Boost COMPONENTS REQUIRED)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        )

include_directories(lib)
include_directories(src)

add_subdirectory(lib/googletest)
add_subdirectory(lib/nlohmann_json)

include(GoogleTest)
enable_testing()

set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(SearchEngine main.cpp ${all_SRCS})

project(SearchEngine_test)

add_executable(SearchEngine_test test_main.cpp ${all_SRCS})

target_link_libraries(SearchEngine PUBLIC nlohmann_json::nlohmann_json ${Boost_LIBRARIES} ws2_32 wsock32)

target_link_libraries(SearchEngine_test PRIVATE gtest gtest_main nlohmann_json::nlohmann_json)

option(TEST_MODE "option for debug" OFF)

if (TEST_MODE)
    add_definitions(-DTEST_MODE)
endif(TEST_MODE)

gtest_discover_tests(SearchEngine_test)

