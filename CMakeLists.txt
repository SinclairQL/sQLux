cmake_minimum_required(VERSION 3.7)

OPTION(SUPPORT_SHADERS "Set to true to enable shader support" OFF)
project(sqlux)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLINUX -DUSE_IOSZ -DDO_GRAB \
	-DNEWSERIAL -DSH_MEM -DXSCREEN \
    -DEVM_SCR -D_GNU_SOURCE -D_XOPEN_SOURCE -DMOUSE -DQVFS")

if (SUPPORT_SHADERS)
    message("Enabling shader support")
    set(INSTALL_LIBRARY "FALSE")
    set(BUILD_DEMOS "FALSE")
    set(BUILD_SHARED "FALSE")
    set(BUILD_STATIC "TRUE")
    set(GPU_DISABLE_WARNINGS "TRUE")
    add_subdirectory(sdl-gpu)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DENABLE_SHADERS")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration -DIPDEV -DSERIAL -DUX_WAIT -DNEWPTY")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIPDEV -DSERIAL -DUX_WAIT -DNEWPTY")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIPDEV -DNO_LOCK -DNO_FORK -D_POSIX -mwindows")
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

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.36.0 REQUIRED COMPONENTS program_options system)

add_custom_target( SubmarineGitVersion
    COMMAND ${CMAKE_COMMAND}
        -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
        -D INPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/version.c.in
        -D OUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/version.c
        -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_version.cmake
)

find_package(SDL2 REQUIRED)

if (SUPPORT_SHADERS)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        set(SDL2_GPU_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/SDL_gpu-MINGW/include")
        set(SDL2_GPU_LIB_DIRS "-L${CMAKE_CURRENT_BINARY_DIR}/SDL_gpu-MINGW/lib")
        set(SDL2_GPU_LIBRARIES "-lSDL2_gpu_s -lopengl32")
    else ()
        set(SDL2_GPU_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/SDL_gpu/include")
        set(SDL2_GPU_LIB_DIRS "-L${CMAKE_CURRENT_BINARY_DIR}/SDL_gpu/lib")
        set(SDL2_GPU_LIBRARIES "-lSDL2_gpu -lGL")
    endif ()
else ()
    set(SDL2_GPU_LIBRARIES "")
    set(SDL2_GPU_INCLUDE_DIRS "")
    set(SDL2_GPU_LIB_DIRS "")
endif ()

include_directories(${CMAKE_CURRENT_LIST_DIR} include/ ${SDL2_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${SDL2_GPU_INCLUDE_DIRS})

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
    src/GPUshaders.c
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

if (SUPPORT_SHADERS)
    add_dependencies(sqlux SDL_gpu)
endif()

if (SDL2_LIBRARIES)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        target_link_libraries(sqlux -Wl,-Bstatic -L/usr/local/${TOOLCHAIN_PREFIX}/lib ${SDL2_GPU_LIB_DIRS} -L. -lmingw32 -lSDL2main -lSDL2 ${SDL2_GPU_LIBRARIES} -lwsock32 -lwinmm -lsetupapi -limm32 -lversion ${Boost_LIBRARIES} -static-libgcc -static-libstdc++)
    else()
        target_link_libraries(sqlux ${SDL2_LIBRARIES} ${Boost_LIBRARIES} ${SDL2_GPU_LIBRARIES})
    endif()
else()
    target_link_libraries(sqlux SDL2::SDL2 ${Boost_LIBRARIES} ${SDL2_GPU_LIBRARIES})
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	target_link_libraries(sqlux -Wl,-Bstatic -lpthread -lssp -Wl,-Bdynamic ${Boost_LIBRARIES})
endif()

set_target_properties(sqlux PROPERTIES LINK_FLAGS_RELEASE -s)

# Ensure that a bare cmake will always exclude shaders
UNSET(SUPPORT_SHADERS CACHE)

install(TARGETS sqlux
	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/
	)
