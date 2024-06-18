# Progress Halted

In real life I am too busy building bare metal clouds so don't expect to see
any new features for a while. I am still happy to review PRs. Also if you want
to take over that could be arranged!

# sQLux (or QL Sux according to DaveP)

sQLux is a [Sinclair QL](https://en.wikipedia.org/wiki/Sinclair_QL) emulator. It runs on Linux (including Rapsberry Pi), Mac and MS Windows. It is based on uQlx but with SDL2 used as its OS layer.

sQLux adds several features over the uQlx base code. See the [Documentation](docs/sqlux.md) for more details.

sQLux is in active development, with new functionality added regularly. However, the latest releases and packages are suitable for normal use.


<a id="releases-anchor"></a>
# Releases

Automatic builds are run and releases are available for the following architectures/O.S.:

- Linux Ubuntu 22.04

  - `aarch64` [arm v8](https://en.wikipedia.org/wiki/AArch64)

  - `armv7  ` (armhf kernel)

- Linux Debian

  - Debian 11 Bullseye

    - `armv7  ` (armhf kernel)

  - Debian 10 Buster

    - `armv7  ` (armhf kernel)

        tested on RaspberryPI 400

- MacOS Intel

- Windows (built w/ MSYS2)
  - 64bit
  - 32bit

Releases are uploaded on Github automatically whenever a new tag, merge or pull request is pushed:

- `tags` on `master` branch: Stable releases
  - The tag string is used as part of the releases description and artifacts names

- `master` branch: Development Releases
  - Every commit or pull into master branch generates a pre-release artifact
  - older Development Releases are overwritten each time a new non-tagged build is run



# Building

Some Basic TL;DR instructions will follow for some architecture/O.S.

For more build tips, see Github Actions commands in `.github/workflows/*.yml` definitions.

## Dependencies
### Debian/Ubuntu

```
apt install build-essential cmake git libsdl2-dev
```
### Fedora

```
dnf install cmake gcc gcc-c++ git SDL2-devel
```

## All Platforms

sQLux uses git submodules, so either clone with the `--recursive` argument.

```
git clone --recursive https://github.com/SinclairQL/sQLux/
```

or after cloning

```
git submodule init
git submodule update
```

## Building Linux

sQLux has switched to using cmake as its build system

1. Install dependencies as per above chapter

1. Build and test
    ```sh
    cmake -B build/
    make
    # then test the binary
    ./build/sqlux --help
    ```

## Building MacOS

1. Install dependencies

   Dependencies are available with [Homebrew](https://brew.sh/)

    ```sh
    brew install cmake sdl2
    ```

2. Build and test

    ```sh
    cmake -B build/
    make
    # then test the binary
    ./build/sqlux --help
    ```

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

Now the mingw version of SDL2 is available and we can build sQLux for Win64

    mkdir mingw
    cd mingw
    cmake -DCMAKE_TOOLCHAIN_FILE=../Toolchain-mingw-w64-x86_64.cmake -DCMAKE_PREFIX_PATH=/usr/local/x86_64-w64-mingw32 ..
    make

## Building MinGW on Windows

Install MSYS2 from here https://www.msys2.org/

Run the mingw64 environment

Install the toolchain and SDL2

    pacman -Sy mingw-w64-x86_64-toolchain
    pacman -Sy mingw-w64-x86_64-cmake
    pacman -Sy mingw-w64-x86_64-SDL2

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

## Developers notes

### Automatic build and upload to Github Releases

1. update `CHANGELOG.md` using [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) best practices

    Use `[Unreleased]`  during normal development phases
    - this will trigger a `latest` release build and upload

    When ready, modify the version to something meaninful.

    Any version different from `[vx.x.x]` or `[vx.x.x-some-text]` will result in:
    - possibly missing Release Body text
    - a `latest` release build and upload.

2. commit

    ```sh
    $ git commit -m "Releasing v1.0.7-rc1"
    [dev-newci-4 51379d4] Releasing v1.0.7-rc1
    2 files changed, 11 insertions(+), 4 deletions(-)
    ```

3. create tag

    Optional.

    If no tag is pushed, the workflow will overwrite/create a `latest` release.
    ```sh
    $ git tag v1.0.7-rc1
    ```


4. push commits && tags at the same time

    ```sh
    $ git push origin dev-branch --tags
    Enumerating objects: 11, done.
    Counting objects: 100% (11/11), done.
    Delta compression using up to 8 threads
    Compressing objects: 100% (5/5), done.
    Writing objects: 100% (6/6), 657 bytes | 657.00 KiB/s, done.
    Total 6 (delta 3), reused 0 (delta 0), pack-reused 0
    remote: Resolving deltas: 100% (3/3), completed with 3 local objects.
    To https://github.com/<repo>
      b66ef20..51379d4  dev-branch -> dev-branch
    * [new tag]         v1.0.7-rc1 -> v1.0.7-rc1
    ```
