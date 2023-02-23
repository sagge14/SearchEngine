cmake_minimum_required(VERSION 3.22)
project(untitled3)

set(CMAKE_CXX_STANDARD 17)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        )


include_directories(lib)
include_directories(src)

add_subdirectory(src)
add_subdirectory(tst)

add_subdirectory(lib/googletest)
add_subdirectory(lib/nlohmann_json)

set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(untitled3 main.cpp ${all_SRCS})

target_link_libraries(untitled3 PUBLIC gtest gtest_main nlohmann_json::nlohmann_json)