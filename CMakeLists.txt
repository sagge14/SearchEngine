cmake_minimum_required(VERSION 3.25.2)
project(SearchEngine)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

set(BOOST_REQD_SUBMODULES
        "tools/cmake"
        "libs/assert;libs/exception;libs/throw_exception;libs/static_assert"
        "libs/bind;libs/function_types;libs/function"
        "libs/chrono;libs/date_time"
        "libs/concept_check"
        "libs/config;libs/container;libs/container_hash"
        "libs/iterator;libs/utility;libs/type_traits;libs/algorithm;;libs/conversion;libs/numeric/conversion;libs/regex;libs/unordered;libs/tokenizer;libs/move;libs/ratio;libs/lexical_cast"
        "libs/tuple;libs/variant2;libs/typeof;libs/detail;libs/array;libs/type_index;libs/range;libs/optional;libs/smart_ptr;libs/integer;libs/rational"
        "libs/intrusive;libs/io;"
        "libs/core;libs/align;libs/predef;libs/preprocessor;libs/system;libs/winapi;libs/atomic"
        "libs/mpl;libs/fusion;libs/mp11"
        "libs/thread"
        "libs/coroutine"
        "libs/context"
        "libs/describe"
        "libs/functional"
        "libs/pool"
        "libs/asio"
        )

Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
        boost
        GIT_REPOSITORY "https://github.com/boostorg/boost.git"
        GIT_TAG master
        GIT_SUBMODULES ${BOOST_REQD_SUBMODULES}
        GIT_PROGRESS TRUE
        CONFIGURE_COMMAND ""  # tell CMake it's not a cmake project
)
FetchContent_MakeAvailable(Boost)

set(JSON_BuildTests OFF CACHE INTERNAL "")

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.12.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
FetchContent_MakeAvailable(json)


file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        )

include_directories(src)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

include(GoogleTest)
enable_testing()

set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(SearchEngine main.cpp ${all_SRCS})

target_link_libraries(SearchEngine PUBLIC nlohmann_json::nlohmann_json Boost::asio ws2_32 wsock32)

project(SearchEngine_test)

include_directories(src)

add_executable(SearchEngine_test test_main.cpp src/InvertedIndex.cpp src/SearchServer.cpp src/ConverterJSON.cpp)

target_link_libraries(SearchEngine_test PUBLIC gtest gtest_main nlohmann_json::nlohmann_json)

option(TEST_MODE "option for debug" OFF)

if (TEST_MODE)
    add_definitions(-DTEST_MODE)
endif(TEST_MODE)

gtest_discover_tests(SearchEngine_test)

