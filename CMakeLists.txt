cmake_minimum_required(VERSION 3.22)
project(SearchEngine)

set(CMAKE_CXX_STANDARD 17)

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

target_link_libraries(SearchEngine PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(SearchEngine_test PRIVATE gtest gtest_main nlohmann_json::nlohmann_json)

gtest_discover_tests(SearchEngine_test)