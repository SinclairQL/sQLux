cmake_minimum_required(VERSION 3.10)

option(SUPPORT_SHADERS "Set to true to enable shader support" OFF)
option(GPL2_CODE "Set to enable GPL2 code inclusion" OFF)
option(LTO "Use LTO in compile" OFF)
option(MACOS_APP_BUNDLE "Build a macOS sQLux.app bundle" OFF)

if(APPLE)
  # Must be set before project(). arm64 requires macOS 11 or later anyway.
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING
    "Minimum macOS version")
  if(MACOS_APP_BUNDLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING
      "Architectures of the app bundle")
  endif()
endif()

project(sqlux C)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

set(CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} -DLINUX -DUSE_IOSZ -DDO_GRAB \
	-DNEWSERIAL -DSH_MEM -DXSCREEN \
    -DEVM_SCR -D_GNU_SOURCE -DMOUSE -DQVFS -Wno-incompatible-pointer-types")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -DIPDEV -DSERIAL -DUX_WAIT -DNEWPTY"
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
elseif(APPLE AND MACOS_APP_BUNDLE)
  # Static SDL2 built from source for the bundle's architectures, so the
  # app does not depend on a Homebrew SDL2 at runtime
  include(ExternalProject)
  if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
  endif()
  set(SDL2_VERSION 2.32.10)
  set(SDL2_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/sdl2)
  string(REPLACE ";" "$<SEMICOLON>" SDL2_ARCHS "${CMAKE_OSX_ARCHITECTURES}")
  ExternalProject_Add(sdl2
    URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.tar.gz
    URL_HASH SHA256=5f5993c530f084535c65a6879e9b26ad441169b3e25d789d83287040a9ca5165
    INSTALL_DIR ${SDL2_PREFIX}
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE=Release
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      -DCMAKE_OSX_ARCHITECTURES=${SDL2_ARCHS}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
      -DCMAKE_C_FLAGS=-ffile-prefix-map=${CMAKE_CURRENT_BINARY_DIR}/=
      -DCMAKE_OBJC_FLAGS=-ffile-prefix-map=${CMAKE_CURRENT_BINARY_DIR}/=
      -DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST=OFF
    BUILD_BYPRODUCTS ${SDL2_PREFIX}/lib/libSDL2.a)

  file(MAKE_DIRECTORY ${SDL2_PREFIX}/include/SDL2)
  add_library(SDL2::SDL2 STATIC IMPORTED)
  set_target_properties(SDL2::SDL2 PROPERTIES
    IMPORTED_LOCATION ${SDL2_PREFIX}/lib/libSDL2.a
    INTERFACE_INCLUDE_DIRECTORIES
      "${SDL2_PREFIX}/include;${SDL2_PREFIX}/include/SDL2"
    INTERFACE_LINK_LIBRARIES "-framework CoreVideo;-framework Cocoa;\
-framework IOKit;-framework ForceFeedback;-framework Carbon;\
-framework CoreAudio;-framework AudioToolbox;-framework AVFoundation;\
-framework Foundation;-weak_framework GameController;\
-weak_framework Metal;-weak_framework QuartzCore;\
-weak_framework CoreHaptics")
  add_dependencies(SDL2::SDL2 sdl2)
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
  qsound.c
  qimsi.c
  qimi.c
  ice.c
  thor.c
  mdv.c
  mcs48.c
  ipc.c
  cycles68k.c
  zx8301.c
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
  src/macos_support.c
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

if(APPLE)
  # Keep build machine paths out of the binary
  target_compile_options(sqlux PRIVATE
    "-ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/=")

  # The Apple linker no longer supports -s
  set_target_properties(sqlux PROPERTIES LINK_FLAGS_RELEASE
    "-Wl,-dead_strip -Wl,-x")
else()
  set_target_properties(sqlux PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

if(APPLE AND MACOS_APP_BUNDLE)
  # The bundle version must be x.y.z, so take it from the last release tag
  execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
      --exclude=latest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE SQLUX_GIT_TAG ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" SQLUX_BUNDLE_VERSION
    "${SQLUX_GIT_TAG}")
  if(NOT SQLUX_BUNDLE_VERSION)
    set(SQLUX_BUNDLE_VERSION "0.0.0")
  endif()

  # Defaults copied to ~/Library/Application Support/sQLux on first launch
  set(SQLUX_BUNDLE_RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/macos/sQLux.icns
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/sqlux.ini)
  target_sources(sqlux PRIVATE ${SQLUX_BUNDLE_RESOURCES})
  set_source_files_properties(${SQLUX_BUNDLE_RESOURCES} PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
  set(SQLUX_RESOURCES_DIR $<TARGET_BUNDLE_CONTENT_DIR:sqlux>/Resources)
  foreach(dir roms examples/mdv1 examples/mdv2 examples/mdv3 shaders)
    get_filename_component(name ${dir} NAME)
    add_custom_command(TARGET sqlux POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/${dir} ${SQLUX_RESOURCES_DIR}/${name}
      VERBATIM)
  endforeach()

  set_target_properties(sqlux PROPERTIES
    MACOSX_BUNDLE TRUE
    OUTPUT_NAME sQLux
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist.in
    MACOSX_BUNDLE_EXECUTABLE_NAME sQLux
    MACOSX_BUNDLE_BUNDLE_NAME sQLux
    MACOSX_BUNDLE_GUI_IDENTIFIER org.sinclairql.sqlux
    MACOSX_BUNDLE_ICON_FILE sQLux.icns
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${SQLUX_BUNDLE_VERSION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${SQLUX_BUNDLE_VERSION}
    MACOSX_BUNDLE_COPYRIGHT "sQLux contributors, see COPYRIGHT")

  # Ad-hoc sign the whole bundle so that it runs on Apple Silicon and
  # Gatekeeper sees a consistent seal. Override SQLUX_CODESIGN_IDENTITY with
  # a Developer ID for distribution.
  set(SQLUX_CODESIGN_IDENTITY "-" CACHE STRING "codesign identity")
  add_custom_command(TARGET sqlux POST_BUILD
    COMMAND codesign --force --deep --options runtime --timestamp=none
      --sign "${SQLUX_CODESIGN_IDENTITY}" $<TARGET_BUNDLE_DIR:sqlux>
    VERBATIM)
endif()

if(LTO)
  set_property(TARGET sqlux PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Ensure that a bare cmake will always exclude shaders
unset(SUPPORT_SHADERS CACHE)

if(APPLE AND MACOS_APP_BUNDLE)
  install(TARGETS sqlux BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})
else()
  install(TARGETS sqlux DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
endif()
