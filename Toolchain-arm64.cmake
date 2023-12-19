
# Toolchain for arm64(aarch64) cross-builds on x86_64(amd64)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_CROSSCOMPILING TRUE)

# Cross-compile tooling
# https://packages.ubuntu.com/jammy/amd64/gcc-aarch64-linux-gnu/filelist
# https://packages.ubuntu.com/jammy/amd64/g++-aarch64-linux-gnu/filelist
set(CMAKE_C_COMPILER "/usr/bin/aarch64-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/aarch64-linux-gnu-g++")

# disabling immintrin_h as per https://clang.llvm.org/doxygen/immintrin_8h_source.html
# #if !defined(__i386__) && !defined(__x86_64__)
# #error "This header is only meant to be used on x86 and x64 architecture"
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDL_DISABLE_IMMINTRIN_H")

# aarch64 pkgs hints: https://packages.ubuntu.com/jammy/arm64/libsdl2-dev/filelist
include_directories(APPEND /usr/include/SDL2)
include_directories(APPEND /usr/include/aarch64-linux-gnu)

# aarch64 pkgs hints: https://packages.ubuntu.com/jammy/arm64/libc6-dev/filelist
include_directories(APPEND /usr/include/aarch64-linux-gnu)
include_directories(APPEND /usr/include/x86_64-linux-gnu)

list(APPEND CMAKE_PREFIX_PATH /usr/lib/aarch64-linux-gnu)
list(APPEND CMAKE_PREFIX_PATH /usr/lib/x86_64-linux-gnu)
