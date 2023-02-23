set(BINARY ${CMAKE_PROJECT_NAME}_tst)

add_executable(${BINARY} ${all_SRCS} test_main.cpp)

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} PUBLIC gtest gtest_main nlohmann_json::nlohmann_json)