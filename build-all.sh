#! /bin/sh

ARCH=`arch`

if [ "$ARCH" != "x86_64" ]; then
    echo "Script assumes x86_64 host for building, exiting..."
fi

cmake -B x86_64/
cmake --build x86_64/

cmake -B x86_32/ -DCMAKE_TOOLCHAIN_FILE=x86_32.cmake
cmake --build x86_32/

cmake -B w64 -DCMAKE_TOOLCHAIN_FILE=mingw-w64-x86_64.cmake -DCMAKE_PREFIX_PATH=/usr/local/x86_64-w64-mingw32
cmake --build w64

cmake -B w32 -DCMAKE_TOOLCHAIN_FILE=mingw-w64-i686.cmake -DCMAKE_PREFIX_PATH=/usr/local/i686-w64-mingw32
cmake --build w32

if [ "x$PI_HOST" != "x" ]; then

	rsync --progress --delete --recursive --exclude=x86_64 --exclude=x86_32 --exclude=w64 --exclude=w32 --exclude=armv6 --exclude=armv7 --exclude=arm64 . $PI_HOST:tmp-build/
	ssh $PI_HOST "cd tmp-build;cmake -B armv6/;cmake --build armv6/"
	rsync --progress --delete --recursive $PI_HOST:tmp-build/armv6 .
	ssh $PI_HOST "rm -rv tmp-build"
fi

if [ "x$ARM64_HOST" != "x" ]; then

	rsync --progress --delete --recursive --exclude=x86_64 --exclude=x86_32 --exclude=w64 --exclude=w32 --exclude=armv6 --exclude=armv7 --exclude=arm64 . $ARM64_HOST:tmp-build/
	ssh $ARM64_HOST "cd tmp-build;cmake -B arm64/;cmake --build arm64/"
	rsync --progress --delete --recursive $ARM64_HOST:tmp-build/arm64 .
	ssh $ARM64_HOST "rm -rv tmp-build"
fi

if [ "x$ARMV7_HOST" != "x" ]; then

	rsync --progress --delete --recursive --exclude=x86_64 --exclude=x86_32 --exclude=w64 --exclude=w32 --exclude=armv6 --exclude=armv7 --exclude=arm64 . $ARMV7_HOST:tmp-build/
	ssh $ARMV7_HOST "cd tmp-build;cmake -B armv7/;cmake --build armv7/"
	rsync --progress --delete --recursive $ARMV7_HOST:tmp-build/armv7 .
	ssh $ARMV7_HOST "rm -rv tmp-build"
fi

VERSION=`git describe --tags --always`

mkdir sqlux-$VERSION

cat > sqlux-$VERSION/sqlux.ini<< EOF
SYSROM = MIN198.rom
ROMIM = TK232.rom
RAMTOP = 4096
FAST_STARTUP = 1
DEVICE = FLP1,198ad.img,qdos-native
BOOT_DEV = FLP1
WIN_SIZE = max
FILTER = 1
FIXASPECT = 1
EOF

curl -O http://www.dilwyn.me.uk/games/adventures/198-adventure.zip
unzip 198-adventure.zip "198adDisk ImageFinal.img"
mv "198adDisk ImageFinal.img" sqlux-$VERSION/198ad.img
rm 198-adventure.zip

cp -r roms sqlux-$VERSION/
cp -r docs sqlux-$VERSION/

cp x86_64/sqlux sqlux-$VERSION/sqlux_x86_64
cp x86_32/sqlux sqlux-$VERSION/sqlux_x86_32
cp w64/sqlux.exe sqlux-$VERSION/sqlux_w64.exe
cp w32/sqlux.exe sqlux-$VERSION/sqlux_w32.exe
if [ "x$PI_HOST" != "x" ]; then
	cp armv6/sqlux sqlux-$VERSION/sqlux_pi
fi
if [ "x$ARMV7_HOST" != "x" ]; then
	cp armv7/sqlux sqlux-$VERSION/sqlux_armv7
fi
if [ "x$ARM64_HOST" != "x" ]; then
	cp arm64/sqlux sqlux-$VERSION/sqlux_arm64
fi

zip -r sqlux-$VERSION.zip sqlux-$VERSION/

rm -r sqlux-$VERSION/
