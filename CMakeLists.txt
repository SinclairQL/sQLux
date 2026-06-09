cmake_minimum_required(VERSION 3.30)

include(FetchContent)
include(CheckIPOSupported)

project(sqlux C)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

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

check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)

find_package(Git)
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  find_package(SDL3)
endif()

add_compile_options(
  -DLINUX
  -DUSE_IOSZ
  -DDO_GRAB
  -DNEWSERIAL
  -DSH_MEM
  -DXSCREEN
  -DEVM_SCR
  -D_GNU_SOURCE
  -DMOUSE
  -DQVFS
  -Wno-incompatible-pointer-types
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_compile_options(
    -Wno-implicit-function-declaration
    -DIPDEV
    -DSERIAL
    -DUX_WAIT
    -DNEWPTY
  )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_compile_options(
    -DIPDEV
    -DSERIAL
    -DUX_WAIT
    -DNEWPTY
  )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_compile_options(
    -DIPDEV
    -DNO_LOCK
    -DNO_FORK
    -D_POSIX
    -mwindows
  )
endif()

# Use address sanitizer in Debug mode
if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  add_compile_options(-fsanitize=address)
endif()

add_custom_target(
  SubmarineGitVersion
  COMMAND
  ${CMAKE_COMMAND} -D GIT_EXECUTABLE=${GIT_EXECUTABLE} -D
  INPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/version.c.in -D
  OUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/version.c -D
  CHANGELOG_FILE=${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md -P
  ${CMAKE_CURRENT_SOURCE_DIR}/generate_version.cmake
)

add_custom_target(
  GenerateDisk
  COMMAND
  qltools ${CMAKE_CURRENT_BINARY_DIR}/default_win1.win -fdd
  COMMAND
  qltools ${CMAKE_CURRENT_BINARY_DIR}/default_win1.win -w ${CMAKE_SOURCE_DIR}/examples/mdv1/BOOT
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  add_compile_options(
    -sUSE_SDL=3
  )
  add_link_options(
    -sMAXIMUM_MEMORY=2GB
    -sALLOW_MEMORY_GROWTH=1
    -sPTHREAD_POOL_SIZE=10
    -sUSE_SDL=3
    -lidbfs.js
    --preload-file=${CMAKE_SOURCE_DIR}/sqlux_wasm.ini@./sqlux.ini
    --preload-file=${CMAKE_SOURCE_DIR}/roms/Minerva_1.98a1.bin@./roms/Minerva_1.98a1.bin
    --preload-file=${CMAKE_SOURCE_DIR}/roms/TK232.rom@./roms/TK232.rom
    --preload-file=default_win1.win@./local/win1.win
  )
  configure_file(sqlux_wrapper_sample.html index.html COPYONLY)
endif()

include_directories(
  include/
  ${sds_SOURCE_DIR}/
  ${args_SOURCE_DIR}/src/
  ${inih_SOURCE_DIR}/
  uqlx/
  ${SDL3_INCLUDE_DIRS})

add_executable(
  sqlux
  ${CMAKE_CURRENT_BINARY_DIR}/version.c
  ${args_SOURCE_DIR}/src/args.c
  ${inih_SOURCE_DIR}/ini.c
  ${sds_SOURCE_DIR}/sds.c
  src/SDL3main.c
  src/SDL3screen.c
  src/emulator_init.c
  src/emulator_options.c
  src/sqlux_hexdump.c
  src/sqlux_windows.c
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
  uqlx/Xscreen.c
  uqlx/dummies.c
  uqlx/general.c
  uqlx/iexl_general.c
  uqlx/instructions_ao.c
  uqlx/instructions_pz.c
  uqlx/memaccess.c
  uqlx/mmodes.c
  uqlx/pty.c
  uqlx/qmtrap.c
  uqlx/sqlux_bdi.c
  uqlx/trace.c
  uqlx/unixstuff.c
  uqlx/util.c
  uqlx/uxfile.c
  uqlx/xc68.c
  uqlx/xcodes.c
  uqlx/xqlmouse.c
)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/version.c
  PROPERTIES GENERATED TRUE)

add_dependencies(sqlux SubmarineGitVersion)
if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  add_dependencies(sqlux GenerateDisk)
endif()

if(ipo_supported)
  # LTO breaks wasm build
  if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    message(STATUS "IPO / LTO enabled")
    set_property(TARGET sqlux PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(STATUS "IPO / LTO disabled on Emscripten")
  endif()
else()
  message(STATUS "IPO / LTO not supported: <${ipo_error}>")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  target_sources(sqlux PRIVATE sQLuxLogo.rc)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  target_sources(sqlux PRIVATE src/wasm_support.c)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  target_link_libraries(sqlux SDL3::SDL3-static libssp.a libwsock32.a libwinpthread.a)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
else ()
  target_link_libraries(sqlux SDL3::SDL3 m)
endif()

set_target_properties(sqlux PROPERTIES LINK_FLAGS_RELEASE -s)

install(TARGETS sqlux DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
