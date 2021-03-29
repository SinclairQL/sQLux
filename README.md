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

Instructions based on debian/ubuntu distro, other distros you will have to modify as appropriate

Download the SDL2 mingw SDK and adapt the following to your environment/version.

    tar xvf SDL2-devel-2.0.14-mingw.tar.gz
    cd cd SDL2-2.0.14/
    sed -i "s|/opt/local/|/usr/local/|" x86_64-w64-mingw32/lib/cmake/SDL2/sdl2-config.cmake
    sed -i "s|/opt/local/|/usr/local/|" i686-w64-mingw32/lib/cmake/SDL2/sdl2-config.cmake
    sudo mkdir /usr/local/i686-w64-mingw32
    sudo mkdir /usr/local/x86_64-w64-mingw32
    sudo make cross

Now mingw version of SDL2 is available and we can build sQLux for Win64

    mkdir mingw
    cd mingw
    cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-x86_64.cmake -DCMAKE_PREFIX_PATH=/usr/local/x86_64-w64-mingw32 ..
    make

# uqlxrc

The emulator currently reads your existing uqlxrc file so will re-use any uQlx setup already existing.

NOTE: ROMIM has changed to now only accept 1 rom name

