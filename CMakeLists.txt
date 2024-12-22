cmake_minimum_required(VERSION 3.10)

option(SUPPORT_SHADERS "Set to true to enable shader support" OFF)
option(GPL2_CODE "Set to enable GPL2 code inclusion" OFF)
option(LTO "Use LTO in compile" OFF)

project(sqlux C)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -DLINUX -DUSE_IOSZ -DDO_GRAB \
	-DNEWSERIAL -DSH_MEM -DXSCREEN \
    -DEVM_SCR -D_GNU_SOURCE -DMOUSE -DQVFS")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_C_FLAGS
      "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration -DIPDEV -DSERIAL -DUX_WAIT -DNEWPTY"
  )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DIPDEV -DSERIAL -DUX_WAIT -DNEWPTY")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(CMAKE_C_FLAGS
      "${CMAKE_C_FLAGS} -DIPDEV -DNO_LOCK -DNO_FORK -D_POSIX -mwindows -Wno-incompatible-pointer-types")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -gstabs")
  else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -ggdb -fsanitize=address")
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} -Ogdb -ggdb -fsanitize=address")
  endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast")
endif()

find_package(Git)
add_custom_target(
  SubmarineGitVersion
  COMMAND
    ${CMAKE_COMMAND} -D GIT_EXECUTABLE=${GIT_EXECUTABLE} -D
    INPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/version.c.in -D
    OUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/version.c -D
    CHANGELOG_FILE=${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md -P
    ${CMAKE_CURRENT_SOURCE_DIR}/generate_version.cmake)

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  set(USE_FLAGS "-pthread -s USE_SDL=2 -s USE_PTHREADS=1")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 ${USE_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS} -s MAXIMUM_MEMORY=1GB -s ALLOW_MEMORY_GROWTH=1 -s PTHREAD_POOL_SIZE=2 -lidbfs.js --preload-file ${CMAKE_SOURCE_DIR}/sqlux_wasm.ini@./sqlux.ini --preload-file ${CMAKE_SOURCE_DIR}/roms/MIN198.rom@./roms/MIN198.rom --preload-file ${CMAKE_SOURCE_DIR}/roms/TK232.rom@./roms/TK232.rom --preload-file default_win1.win"
  )
  execute_process(COMMAND qltools ${CMAKE_CURRENT_BINARY_DIR}/default_win1.win
                          -fdd)
  file(GLOB mdv1_files "${CMAKE_SOURCE_DIR}/mdv1/*")
  execute_process(COMMAND qltools ${CMAKE_CURRENT_BINARY_DIR}/default_win1.win
                          -w ${mdv1_files})
  configure_file(sqlux_wrapper_sample.html index.html COPYONLY)
else()
  find_package(SDL2 REQUIRED)
endif()

if(SUPPORT_SHADERS)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SDL2_GPU_INCLUDE_DIRS
        "${CMAKE_CURRENT_BINARY_DIR}/SDL_gpu-MINGW/include")
    set(SDL2_GPU_LIB_DIRS "-L${CMAKE_CURRENT_BINARY_DIR}/SDL_gpu-MINGW/lib")
    set(SDL2_GPU_LIBRARIES "-lSDL2_gpu_s -lopengl32")
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SDL2_GPU_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/SDL_gpu/include")
    set(SDL2_GPU_LIB_DIRS "-L${CMAKE_CURRENT_BINARY_DIR}/SDL_gpu/lib")
    set(SDL2_GPU_LIBRARIES "-lSDL2_gpu -framework OpenGL")
  else()
    set(SDL2_GPU_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/SDL_gpu/include")
    set(SDL2_GPU_LIB_DIRS "-L${CMAKE_CURRENT_BINARY_DIR}/SDL_gpu/lib")
    set(SDL2_GPU_LIBRARIES "-lSDL2_gpu -lGL -lm")
  endif()
else()
  set(SDL2_GPU_LIBRARIES "-lm")
  set(SDL2_GPU_INCLUDE_DIRS "")
  set(SDL2_GPU_LIB_DIRS "")
endif()

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  include/
  sds/
  args/src/
  inih/
  GPL2/
  ${SDL2_INCLUDE_DIRS}
  ${SDL2_GPU_INCLUDE_DIRS})

add_executable(
  sqlux
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
  xc68.c
  xcodes.c
  xqlmouse.c
  sqlux_bdi.c
  mmodes.c
  ${CMAKE_CURRENT_BINARY_DIR}/version.c
  src/emulator_init.c
  src/sqlux_windows.c
  src/sqlux_hexdump.c
  src/SDL2main.c
  src/emulator_options.c
  sds/sds.c
  args/src/args.c
  inih/ini.c)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/version.c
                            PROPERTIES GENERATED TRUE)

add_dependencies(sqlux SubmarineGitVersion)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  target_sources(sqlux PRIVATE sQLuxLogo.rc)
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  target_sources(sqlux PRIVATE src/wasm_support.c)
endif()

if(GPL2_CODE)
  target_sources(sqlux PRIVATE GPL2/w5100.c GPL2/w5100_socket.c)
endif()

if(SUPPORT_SHADERS)
  message("Enabling shader support")
  target_compile_definitions(sqlux PUBLIC ENABLE_SHADERS)
  set(INSTALL_LIBRARY "FALSE")
  set(BUILD_DEMOS "FALSE")
  set(BUILD_SHARED "FALSE")
  set(BUILD_STATIC "TRUE")
  set(DISABLE_GLES_1 "TRUE")
  add_subdirectory(sdl-gpu)
  target_compile_options(SDL_gpu PRIVATE "-w")
  target_compile_options(SDL_gpu PRIVATE "-Wno-incompatible-pointer-types")
  add_dependencies(sqlux SDL_gpu)
endif()

if(SDL2_LIBRARIES)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(
      sqlux
      -Wl,-Bstatic
      -L/usr/local/${TOOLCHAIN_PREFIX}/lib
      ${SDL2_GPU_LIB_DIRS}
      -L.
      -lmingw32
      -lSDL2main
      -lSDL2
      ${SDL2_GPU_LIBRARIES}
      -lwsock32
      -lwinmm
      -lsetupapi
      -limm32
      -lversion
      -static-libgcc
      -static-libstdc++)
  else()
    target_link_libraries(sqlux ${SDL2_GPU_LIB_DIRS} ${SDL2_LIBRARIES}
                          ${SDL2_GPU_LIBRARIES})
  endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  target_link_libraries(sqlux ${SDL2_LIBRARIES})
else()
  target_link_libraries(sqlux SDL2::SDL2 ${SDL2_GPU_LIBRARIES})
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  target_link_libraries(sqlux -Wl,-Bstatic -lpthread -lssp -Wl,-Bdynamic)
endif()

set_target_properties(sqlux PROPERTIES LINK_FLAGS_RELEASE -s)

if(LTO)
  set_property(TARGET sqlux PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Ensure that a bare cmake will always exclude shaders
unset(SUPPORT_SHADERS CACHE)

install(TARGETS sqlux DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
