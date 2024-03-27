cmake_minimum_required(VERSION 3.5)

project(simple)

SET(CMAKE_C_COMPILER   /home/user/WorkArea/buildroot_a5d2xpld_opcua_modbus-2023.10/buildroot-at91/output/host/bin/arm-buildroot-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /home/user/WorkArea/buildroot_a5d2xpld_opcua_modbus-2023.10/buildroot-at91/output/host/bin/arm-buildroot-linux-gnueabihf-g++)

#SET(CMAKE_C_COMPILER   /home/user/WorkArea/buildroot_9x60_curiosity-2023.10/buildroot-at91/output/host/bin/arm-buildroot-linux-gnueabi-gcc)
#SET(CMAKE_CXX_COMPILER /home/user/WorkArea/buildroot_9x60_curiosity-2023.10/buildroot-at91/output/host/bin/arm-buildroot-linux-gnueabi-g++)

#SET(CMAKE_C_COMPILER	gcc)
#SET(CMAKE_CXX_COMPILER	g++)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(simple_server_02 ${PROJECT_SOURCE_DIR}/../src/simple_server_02.c)
target_link_libraries(simple_server_02 libmodbus.so)

add_executable(simple_client_02 ${PROJECT_SOURCE_DIR}/../src/simple_client_02.c)
target_link_libraries(simple_client_02 libmodbus.so)

