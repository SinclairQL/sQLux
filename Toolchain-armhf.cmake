
# Toolchain for armhf(arm) cross-builds on x86_64(amd64)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armhf)
set(CMAKE_CROSSCOMPILING TRUE)

# Cross-compile tooling
# https://packages.ubuntu.com/jammy/amd64/gcc-arm-linux-gnueabihf/filelist
# https://packages.ubuntu.com/jammy/amd64/g++-arm-linux-gnueabihf/filelist
set(CMAKE_C_COMPILER   "/usr/bin/arm-linux-gnueabihf-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/arm-linux-gnueabihf-g++")

# disabling immintrin_h as per https://clang.llvm.org/doxygen/immintrin_8h_source.html
# #if !defined(__i386__) && !defined(__x86_64__)
# #error "This header is only meant to be used on x86 and x64 architecture"
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDL_DISABLE_IMMINTRIN_H")

# armhf pkgs hints: https://packages.ubuntu.com/jammy/armhf/libsdl2-dev/filelist
include_directories(APPEND /usr/include/SDL2)
list(APPEND CMAKE_PREFIX_PATH /usr/lib/arm-linux-gnueabihf/cmake/SDL2)

# armhf pkgs hints: https://packages.ubuntu.com/jammy/armhf/libc6-dev/filelist
include_directories(APPEND /usr/include/arm-linux-gnueabihf)
include_directories(APPEND /usr/include/x86_64-linux-gnu)

list(APPEND CMAKE_PREFIX_PATH /usr/lib/arm-linux-gnueabihf)
list(APPEND CMAKE_PREFIX_PATH /usr/lib/x86_64-linux-gnu)

# see https://github.com/enribus/sQLux-fork/issues/1
#set(CMAKE_C_FLAGS '${CMAKE_C_FLAGS} -D__STRICT_ANSI__ -D"__float128=long double"')
