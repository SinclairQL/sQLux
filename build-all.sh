#! /bin/sh

cmake -B x86_64/
cmake --build x86_64/

cmake -B x86_32/ -DCMAKE_TOOLCHAIN_FILE=x86_32.cmake
cmake --build x86_32/

cmake -B w64 -DCMAKE_TOOLCHAIN_FILE=mingw-w64-x86_64.cmake -DCMAKE_PREFIX_PATH=/usr/local/x86_64-w64-mingw32
cmake --build w64

cmake -B w32 -DCMAKE_TOOLCHAIN_FILE=mingw-w64-i686.cmake -DCMAKE_PREFIX_PATH=/usr/local/i686-w64-mingw32
cmake --build w32

if [ "x$PI_HOST" != "x" ]; then

	rsync --progress --delete --recursive --exclude=x86_64 --exclude=x86_32 --exclude=w64 --exclude=w32 --exclude=armv6 --exclude=arm64. $PI_HOST:tmp-build/
	ssh $PI_HOST "cd tmp-build;cmake -B armv6/;cmake --build armv6/"
	rsync --progress --delete --recursive $PI_HOST:tmp-build/armv6 .
	ssh $PI_HOST "rm -rv tmp-build"
fi

if [ "x$ARM64_HOST" != "x" ]; then

	rsync --progress --delete --recursive --exclude=x86_64 exclude=x86_32 --exclude=w64 --exclude=w32 --exclude=armv6 --exclude=arm64 . $ARM64_HOST:tmp-build/
	ssh $ARM64_HOST "cd tmp-build;cmake -B arm64/;cmake --build arm64/"
	rsync --progress --delete --recursive $ARM64_HOST:tmp-build/arm64 .
	ssh $ARM64_HOST "rm -rv tmp-build"
fi
