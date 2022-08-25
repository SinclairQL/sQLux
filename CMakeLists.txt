cmake_minimum_required(VERSION 3.7)
cmake_minimum_required(VERSION 3.7)

project(sqlux)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLINUX -DUSE_IOSZ -DDO_GRAB \
	-DNEWSERIAL -DSH_MEM -DXSCREEN \
    -DEVM_SCR -D_GNU_SOURCE -D_XOPEN_SOURCE -DMOUSE -DQVFS")

#Break out ability to build with sound
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSOUND")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSOUND")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration -DIPDEV -DSERIAL -DUX_WAIT -DNEWPTY")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIPDEV -DSERIAL -DUX_WAIT -DNEWPTY")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIPDEV -DNO_LOCK -DNO_FORK -D_POSIX -mwindows")
    set(Boost_USE_STATIC_LIBS ON) 
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -gstabs")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -gstabs")
	else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -ggdb -fsanitize=address")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Ogdb -ggdb -fsanitize=address")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -ggdb -fsanitize=address")
	endif()
else()
	if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
	else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
	endif()
endif()

find_package(Git)

find_package(Boost 1.36.0 REQUIRED COMPONENTS program_options)

add_custom_target( SubmarineGitVersion
    COMMAND ${CMAKE_COMMAND}
        -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
        -D INPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/version.c.in
        -D OUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/version.c
        -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_version.cmake
)


find_package(SDL2 REQUIRED)

include_directories(${CMAKE_CURRENT_LIST_DIR} include/ ${SDL2_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

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
    util.c
    uxfile.c
    vm.c
    xc68.c
    xcodes.c
    xqlmouse.c
    sqlux_bdi.c
    mmodes.c
    ${CMAKE_CURRENT_BINARY_DIR}/version.c
    src/EmulatorInit.cpp
    src/sqlux_windows.c
    src/sqlux_hexdump.c
    src/SDL2main.cpp
    src/SqluxOptions.cpp
    )
set_source_files_properties( ${CMAKE_CURRENT_BINARY_DIR}/version.c
PROPERTIES GENERATED TRUE)

target_compile_features(sqlux PUBLIC cxx_std_17)

add_dependencies(sqlux SubmarineGitVersion)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_sources(sqlux PRIVATE sQLuxLogo.rc)
endif ()

if (SDL2_LIBRARIES)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_link_libraries(sqlux -Wl,-Bstatic -L/usr/local/${TOOLCHAIN_PREFIX}/lib -lmingw32 -lSDL2main -lSDL2 -lwsock32 -lwinmm -lsetupapi -limm32 -lversion ${Boost_LIBRARIES} -static-libgcc -static-libstdc++)
    else()
        target_link_libraries(sqlux ${SDL2_LIBRARIES} ${Boost_LIBRARIES})
    endif()
else()
    target_link_libraries(sqlux SDL2::SDL2 ${Boost_LIBRARIES})
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(sqlux -Wl,-Bstatic -lpthread -Wl,-Bdynamic ${Boost_LIBRARIES})
endif()

set_target_properties(sqlux PROPERTIES LINK_FLAGS_RELEASE -s)

install(TARGETS sqlux
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/
	)
