cmake_minimum_required(VERSION 3.7)

project(sqlux)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

set(CMAKE_C_FLAGS "-DLINUX -DUSE_IPC -DQVFS -DUSE_IOSZ -DDO_GRAB -DSERIAL \
        -DNEWSERIAL -DNEWPTY -DSH_MEM -DIPDEV  -DXSCREEN \
        -DUX_WAIT -DHAS_STPCPY -DEVM_SCR -D_GNU_SOURCE -D_XOPEN_SOURCE -DMOUSE")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSOUND")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_LOCK -DNO_FORK")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -ggdb")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast")
endif()

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_NET_INCLUDE_DIRS})

add_executable(sqlux Init.c QDisk.c QL_basext.c QL_boot.c QL_cconv.c QL_config.c QL_driver.c QL_files.c QL_hardware.c QL_poll.c QL_screen.c QL_serial.c QL_sound.c QLip.c QLserio.c QLtraps.c QVFS.c SDL2screen.c Xscreen.c dummies.c general.c iexl_general.c instructions_ao.c instructions_pz.c mach_exception.c memaccess.c pty.c qmtrap.c script.c trace.c unixstuff.c uqlx_cfg.c util.c uxfile.c version.c vm.c xc68.c xcodes.c xipc.c xlmain.c xqlmouse.c sqlux_bdi.c)

if (SDL2_LIBRARIES)
    target_link_libraries(sqlux m ${SDL2_LIBRARIES} ${SDL2_NET_LIBRARIES})
else()
    target_link_libraries(sqlux SDL2::SDL2 ${SDL2_NET_LIBRARIES})
endif()

