# sQLux (or QL Sux according to DaveP)

This is an emulator based on uQlx but with SDL2 used as its OS layer.

It is currently very much work in progress so not suitable for people who don't want to get messy coding or debugging the emulator.

The eventual aim is to replace all OS calls to their SDL2 equivalents then port the emulator to various other OS types using SDL.

# Building
## Building Linux

sQLux has switch to using cmake as its build system

Basic instructions

    mkdir linux
    cd linux
    cmake ..
    make

## Building MinGW

sQLux not fully buildable on MinGW system yet

    mkdir mingw
    cd mingw
    cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-x86_64.cmake -DCMAKE_PREFIX_PATH=/usr/local/x86_64-w64-mingw32 ..
    make

# uqlxrc

The emulator currently reads your existing uqlxrc file so will re-use any uQlx setup already existing.

NOTE: ROMIM has changed to now only accept 1 rom name

