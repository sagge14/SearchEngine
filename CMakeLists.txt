cmake_minimum_required(VERSION 3.22)
project(untitled3)

set(CMAKE_CXX_STANDARD 17)

set(JSON_BuildTests OFF CACHE INTERNAL "")


set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(untitled3 main.cpp InvertedIndex.cpp InvertedIndex.h SearchServer.cpp SearchServer.h ConverterJSON.cpp ConverterJSON.h Settings.cpp Settings.h)

add_subdirectory(nlohmann_json)

target_link_libraries(untitled3 ${Boost_LIBRARIES} nlohmann_json::nlohmann_json)






