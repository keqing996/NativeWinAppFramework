cmake_minimum_required(VERSION 3.22)

set(TEST_NAME test-win-window-style)

add_executable(${TEST_NAME} main.cpp)

target_link_libraries(${TEST_NAME} PRIVATE ${CPP_HELPER_LIB_NAME})