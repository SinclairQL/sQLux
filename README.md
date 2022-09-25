![Build x86-64](https://github.com/SinclairQL/sQLux/actions/workflows/build-native.yml/badge.svg)
![Build ARM](https://github.com/SinclairQL/sQLux/actions/workflows/build-arm.yml/badge.svg)
![Build MSYS2](https://github.com/SinclairQL/sQLux/actions/workflows/build-msys2.yml/badge.svg)

# sQLux (or QL Sux according to DaveP)

sQLux is a [Sinclair QL](https://en.wikipedia.org/wiki/Sinclair_QL) emulator. It runs on Linux (including Rapsberry Pi), Mac and MS Windows. It is based on uQlx but with SDL2 used as its OS layer. 

sQLux adds several features over the uQlx base code. See the [Documentation](docs/sqlux.md) for more details.

sQLux is in active development, with new functionality added regularly. However, the latest releases and packages are suitable for normal use.

# Building
## Dependencies
### Debian/Ubuntu

```
apt install build-essential cmake git libboost-program-options-dev libboost-system-dev libsdl2-dev
```
### Fedora

```
dnf install boost-devel boost-static cmake gcc gcc-c++ git SDL2-devel
```

## Building Linux

sQLux has switched to using cmake as its build system

Basic instructions

    mkdir linux
    cd linux
    cmake ..
    make

## Building MinGW on Linux

Instructions based on debian/ubuntu distro, for other distros you will have to modify as appropriate

Download the SDL2 mingw SDK and adapt the following to your environment/version.
The SDL2 development libraries can be found [here](https://github.com/libsdl-org/SDL/releases): 

    tar xvf SDL2-devel-2.0.18-mingw.tar.gz
    cd SDL2-2.0.18/
    sed -i "s|/opt/local/|/usr/local/|" x86_64-w64-mingw32/lib/cmake/SDL2/sdl2-config.cmake
    sed -i "s|/opt/local/|/usr/local/|" i686-w64-mingw32/lib/cmake/SDL2/sdl2-config.cmake
    sudo mkdir /usr/local/i686-w64-mingw32
    sudo mkdir /usr/local/x86_64-w64-mingw32
    sudo make cross

Download, build and install a mingw version of the boost libraries. Instructions are given [here](https://github.com/libmingw-w64/libboost-mingw-w64)
   
Now the mingw versions of SDL2 and boost is available and we can build sQLux for Win64

    mkdir mingw
    cd mingw
    cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-x86_64.cmake -DCMAKE_PREFIX_PATH=/usr/local/x86_64-w64-mingw32 ..
    make

## Building MinGW on Windows

Install MSYS2 from here https://www.msys2.org/

Run the mingw64 environment

Install the toolchain, SDL2 and boost

    pacman -Sy mingw-w64-x86_64-toolchain
    pacman -Sy mingw-w64-x86_64-cmake
    pacman -Sy mingw-w64-x86_64-SDL2
    pacman -Sy mingw-w64-x86_64-boost

Create the build directory and compile

    mkdir mingw
    cd mingw
    cmake.exe -G "MinGW Makefiles" ..
    mingw32-make

This will generate `sqlux.exe`. Dependencies are statically linked, so `sqlux.exe` will run without the need for additional dlls.

## MinGW pthreads/win32 threads

The 64bit build using mingw requires the winpthread library for the high definition timer
support.

The 32bit build can be built using win32 threads for XP compatibility and therefore does
not include the high resolution timer support.

# uqlxrc

The emulator currently reads an existing uqlxrc file, so it will re-use any uQlx setup already existing.

NOTE: ROMIM has changed to now only accept 1 rom name

NOTE: RAMTOP is where in memory the top of the ram is, not the amount of ram.
As ram on QL starts at 128k in memory map then to created a 128k QL you
need to set RAMTOP = 256

sQLux supports additional parameters over those used for uqlxrc. See the [Documentation](docs/sqlux.md) for more details.

# Shader support
sQLux has support for GPU shaders, to emulate the effects seen when using a CRT display. See the [Shader documentation](docs/shaders.md) for more details on how to build and run sQLux with GPU shaders.

