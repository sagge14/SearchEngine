set(BINARY ${CMAKE_PROJECT_NAME}_src)

add_executable(${BINARY} ${all_SRCS})

target_link_libraries(${BINARY} PUBLIC nlohmann_json::nlohmann_json)