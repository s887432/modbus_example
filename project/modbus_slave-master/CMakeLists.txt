cmake_minimum_required(VERSION 3.5)

project(simple)

SET(CMAKE_C_COMPILER   /home/user/WorkArea/buildroot_a5d2xpld_opcua_modbus-2023.10/buildroot-at91/output/host/bin/arm-buildroot-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /home/user/WorkArea/buildroot_a5d2xpld_opcua_modbus-2023.10/buildroot-at91/output/host/bin/arm-buildroot-linux-gnueabihf-g++)

#SET(CMAKE_C_COMPILER   /home/user/WorkArea/buildroot_9x60_curiosity-2023.10/buildroot-at91/output/host/bin/arm-buildroot-linux-gnueabi-gcc)
#SET(CMAKE_CXX_COMPILER /home/user/WorkArea/buildroot_9x60_curiosity-2023.10/buildroot-at91/output/host/bin/arm-buildroot-linux-gnueabi-g++)

#SET(CMAKE_C_COMPILER	gcc)
#SET(CMAKE_CXX_COMPILER	g++)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(modbus_slave ${PROJECT_SOURCE_DIR}/../src/modbus_slave.c)
target_link_libraries(modbus_slave libmodbus.so pthread)

add_executable(modbus_master ${PROJECT_SOURCE_DIR}/../src/modbus_master.c)
target_link_libraries(modbus_master libmodbus.so pthread)

