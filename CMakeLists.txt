cmake_minimum_required(VERSION 3.7)

project(sqlux)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLINUX -DUSE_IOSZ -DDO_GRAB \
	-DNEWSERIAL -DSH_MEM -DXSCREEN \
    -DEVM_SCR -D_GNU_SOURCE -D_XOPEN_SOURCE -DMOUSE -DQVFS")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration -DSOUND -DIPDEV -DSERIAL -DUX_WAIT -DNEWPTY -DHAS_STPCPY")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSOUND -DIPDEV -DSERIAL -DUX_WAIT -DNEWPTY -DHAS_STPCPY")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSOUND -DIPDEV -DNO_LOCK -DNO_FORK -D_POSIX -mwindows")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -gstabs")
	else()
    		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -ggdb")
	endif()
else()
	if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1")
	else()
    		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast")
	endif()
endif()

find_package(Git)

add_custom_target( SubmarineGitVersion
    COMMAND ${CMAKE_COMMAND}
        -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
        -D INPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/version.c.in
        -D OUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/version.c
        -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_version.cmake
)


find_package(SDL2 REQUIRED)

include_directories(${CMAKE_CURRENT_LIST_DIR} include/ ${SDL2_INCLUDE_DIRS})

add_executable(sqlux
    Init.c
    QDisk.c
    QL_basext.c
    QL_boot.c
    QL_cconv.c
    QL_config.c
    QL_driver.c
    QL_files.c
    QL_hardware.c
    QL_poll.c
    QL_screen.c
    QL_serial.c
    QL_sound.c
    QLip.c
    QLserio.c
    QLtraps.c
    QVFS.c
    src/SDL2screen.c
    Xscreen.c
    dummies.c
    general.c
    iexl_general.c
    instructions_ao.c
    instructions_pz.c
    memaccess.c
    pty.c
    qmtrap.c
    trace.c
    unixstuff.c
    uqlx_cfg.c
    util.c
    uxfile.c
    vm.c
    xc68.c
    xcodes.c
    xlmain.c
    xqlmouse.c
    sqlux_bdi.c
    mmodes.c
    ${CMAKE_CURRENT_BINARY_DIR}/version.c
    src/sqlux_windows.c
    src/sqlux_hexdump.c
    )
set_source_files_properties( ${CMAKE_CURRENT_BINARY_DIR}/version.c
PROPERTIES GENERATED TRUE)

add_dependencies(sqlux SubmarineGitVersion)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_sources(sqlux PRIVATE sQLuxLogo.rc)
endif ()

if (SDL2_LIBRARIES)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_link_libraries(sqlux -Wl,-Bstatic -L/usr/local/x86_64-w64-mingw32/lib -lSDL2main -lSDL2 -Wl,-Bdynamic -lwsock32 -lwinmm -lsetupapi -limm32 -lversion -static-libgcc)
    else()
        target_link_libraries(sqlux ${SDL2_LIBRARIES})
    endif()
else()
    target_link_libraries(sqlux SDL2::SDL2)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(sqlux -Wl,-Bstatic -lpthread -Wl,-Bdynamic)
endif()

install(TARGETS sqlux
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/
	)
