cmake_minimum_required(VERSION 3.30)

option(LTO "Use LTO in compile" OFF)

project(sqlux C)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

include(FetchContent)
FetchContent_Declare(
  args
  GIT_REPOSITORY https://github.com/dmulholl/args.git
  GIT_TAG 3.3.0
)
FetchContent_MakeAvailable(args)
FetchContent_Declare(
  sds
  GIT_REPOSITORY https://github.com/antirez/sds.git
  GIT_TAG 2.0.0
)
FetchContent_MakeAvailable(sds)
FetchContent_Declare(
  inih
  GIT_REPOSITORY https://github.com/benhoyt/inih.git
  GIT_TAG r62
)
FetchContent_MakeAvailable(inih)

set(CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} -DLINUX -DUSE_IOSZ -DDO_GRAB \
	-DNEWSERIAL -DSH_MEM -DXSCREEN \
    -DEVM_SCR -D_GNU_SOURCE -DMOUSE -DQVFS -Wno-incompatible-pointer-types")

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
    "${CMAKE_C_FLAGS} -DIPDEV -DNO_LOCK -DNO_FORK -D_POSIX -mwindows")
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

set(SDL2_GPU_LIBRARIES "-lm")
set(SDL2_GPU_INCLUDE_DIRS "")
set(SDL2_GPU_LIB_DIRS "")

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  include/
  ${sds_SOURCE_DIR}/
  ${args_SOURCE_DIR}/src/
  ${inih_SOURCE_DIR}/
  uqlx/
  ${SDL2_INCLUDE_DIRS}
  ${SDL2_GPU_INCLUDE_DIRS})

add_executable(
  sqlux
  uqlx/Init.c
  uqlx/QDisk.c
  uqlx/QL_basext.c
  uqlx/QL_boot.c
  uqlx/QL_cconv.c
  uqlx/QL_config.c
  uqlx/QL_driver.c
  uqlx/QL_files.c
  uqlx/QL_hardware.c
  uqlx/QL_poll.c
  uqlx/QL_screen.c
  uqlx/QL_serial.c
  uqlx/QL_sound.c
  uqlx/QLip.c
  uqlx/QLserio.c
  uqlx/QLtraps.c
  uqlx/QVFS.c
  src/SDL2screen.c
  uqlx/Xscreen.c
  uqlx/dummies.c
  uqlx/general.c
  uqlx/iexl_general.c
  uqlx/instructions_ao.c
  uqlx/instructions_pz.c
  uqlx/memaccess.c
  uqlx/pty.c
  uqlx/qmtrap.c
  uqlx/trace.c
  uqlx/unixstuff.c
  uqlx/util.c
  uqlx/uxfile.c
  uqlx/xc68.c
  uqlx/xcodes.c
  uqlx/xqlmouse.c
  uqlx/sqlux_bdi.c
  uqlx/mmodes.c
  ${CMAKE_CURRENT_BINARY_DIR}/version.c
  src/emulator_init.c
  src/sqlux_windows.c
  src/sqlux_hexdump.c
  src/SDL2main.c
  src/emulator_options.c
  ${sds_SOURCE_DIR}/sds.c
  ${args_SOURCE_DIR}/src/args.c
  ${inih_SOURCE_DIR}/ini.c)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/version.c
  PROPERTIES GENERATED TRUE)

add_dependencies(sqlux SubmarineGitVersion)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  target_sources(sqlux PRIVATE sQLuxLogo.rc)
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  target_sources(sqlux PRIVATE src/wasm_support.c)
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

install(TARGETS sqlux DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
