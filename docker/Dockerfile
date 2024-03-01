# syntax=docker/dockerfile:1.3-labs
From ubuntu:22.04

SHELL ["/bin/bash", "-c"]

RUN <<EOF
set -e
export DEBIAN_FRONTEND=noninteractive

echo "tzdata tzdata/Areas select Etc" | debconf-set-selections
echo "tzdata tzdata/Zones/Etc select UTC" | debconf-set-selections
rm -f /etc/localtime /etc/timezone

apt-get -y update
apt-get -y upgrade
apt-get -y install build-essential git cmake libsdl2-dev libboost-system-dev \
    libboost-program-options-dev zip curl wget gpg zstd gawk wine clang lld \
    debootstrap coreutils qemu-user-static
apt-get install -y locales
localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8

mkdir -p /build/sqlux
mkdir -p /build/release

git config --global --add safe.directory /build/sqlux
EOF

# cache image at this point to shorten rebuild times.

RUN <<EOF
set -e

git clone https://github.com/HolyBlackCat/quasi-msys2 /quasi-msys2-w64/
cd /quasi-msys2-w64/
make install _gcc _SDL2 _boost
cd /

ln -nfs "/quasi-msys2-w64/root/mingw64" /mingw64

EOF

RUN --security=insecure <<EOF
set -e

mkdir /raspbian
debootstrap --no-check-gpg --foreign --arch=armhf --include=build-essential,cmake,libsdl2-dev,libboost-system-dev,libboost-program-options-dev bullseye /raspbian http://archive.raspbian.org/raspbian
cp /usr/bin/qemu-arm-static /raspbian/usr/bin
chroot /raspbian /debootstrap/debootstrap --second-stage

mkdir /bullseye
debootstrap --no-check-gpg --foreign --arch=arm64 --include=build-essential,cmake,libsdl2-dev,libboost-system-dev,libboost-program-options-dev bullseye /bullseye https://deb.debian.org/debian
cp /usr/bin/qemu-aarch64-static /raspbian/usr/bin
chroot /bullseye /debootstrap/debootstrap --second-stage

EOF

COPY ./build-sqlux-release.sh /build/

ENV LANG en_US.utf8
WORKDIR /build

ENTRYPOINT ["/build/build-sqlux-release.sh"]

