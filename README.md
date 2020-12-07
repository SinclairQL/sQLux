# sQLux (or QL Sux according to DaveP)

This is an emulator based on uQlx but with SDL2 used as its OS layer.

It is currently very much work in progress so not suitable for people who don't want to get messy coding or debugging the emulator.

The eventual aim is to replace all OS calls to their SDL2 equivalents then port the emulator to various other OS types using SDL.

# Building

sQLux has switch to using cmake as its build system

Basic instructions

    mkdir build
    cd build
    cmake ..
    make

# uqlxrc

The emulator currently reads your existing uqlxrc file so will re-use any uQlx setup already existing.

NOTE: ROMIM has changed to now only accept 1 rom name

