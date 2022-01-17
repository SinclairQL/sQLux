![Build x86-64](https://github.com/SinclairQL/sQLux/actions/workflows/build-native.yml/badge.svg)
![Build ARM](https://github.com/SinclairQL/sQLux/actions/workflows/build-arm.yml/badge.svg)

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

## Building MinGW on Linux

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

## Building MinGW on Windows

Install MSYS2 from here https://www.msys2.org/

Run the mingw64 environment

Install the toolchain and SDL2

    pacman -Sy mingw-w64-x86_64-toolchain
    pacman -Sy mingw-w64-x86_64-SDL2

Create the build directory and compile

    mkdir mingw
    cd mingw
    cmake.exe -G "MinGW Makefiles" ..
    mingw32-make

This will generate a sqlux.exe, to run under windows explorer or shell yo
just need to place SDL2.dll from /mingw64/bin/ in the same directory. In
mingw64 env it will be found automatically.

## MinGW pthreads/win32 threads

The 64bit build using mingw requries the winpthread library for the high definition timer
support.

The 32bit build can be built using win32 theads for XP compatability and therefore does
not include the high resolution timer support.

# uqlxrc

The emulator currently reads your existing uqlxrc file so will re-use any uQlx setup already existing.

NOTE: ROMIM has changed to now only accept 1 rom name

NOTE: RAMTOP is where in memory the top of the ram is, not the amount of ram.
As ram on QL starts at 128k in memory map then to created a 128k QL you
need to set RAMTOP = 256

