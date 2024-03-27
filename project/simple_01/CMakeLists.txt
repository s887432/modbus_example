cmake_minimum_required(VERSION 3.5)

project(simple)

SET(CMAKE_C_COMPILER	gcc)
SET(CMAKE_CXX_COMPILER	g++)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(simple_server_01 ${PROJECT_SOURCE_DIR}/../src/simple_server_01.c)
target_link_libraries(simple_server_01 libmodbus.so)

add_executable(simple_client_01 ${PROJECT_SOURCE_DIR}/../src/simple_client_01.c)
target_link_libraries(simple_client_01 libmodbus.so)

