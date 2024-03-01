# syntax=docker/dockerfile:1.3-labs
From ubuntu:22.04

SHELL ["/bin/bash", "-ce"]

RUN <<EOF
export DEBIAN_FRONTEND=noninteractive

echo "tzdata tzdata/Areas select Etc" | debconf-set-selections
echo "tzdata tzdata/Zones/Etc select UTC" | debconf-set-selections
rm -f /etc/localtime /etc/timezone

dpkg --add-architecture i386

apt-get -y update
apt-get -y upgrade
apt-get -y install \
  build-essential \
  clang \
  cmake \
  coreutils \
  curl \
  debootstrap \
  gawk \
  git \
  gpg \
  libsdl2-dev \
  lld \
  locales \
  qemu-user-static \
  sudo \
  wget \
  wine \
  wine32 \
  wine64 \
  zip \
  zstd
localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8

mkdir -p /build/sqlux
mkdir -p /build/release

git config --global --add safe.directory /build/sqlux
EOF

# cache image at this point to shorten rebuild times.

RUN <<EOF
git clone https://github.com/HolyBlackCat/quasi-msys2 /quasi-msys2-w64/
cd /quasi-msys2-w64/
echo MINGW64 >msystem.txt
make install _gcc _SDL2
cd /

ln -nfs "/quasi-msys2-w64/root/mingw64" /mingw64

git clone https://github.com/HolyBlackCat/quasi-msys2 /quasi-msys2-w32/
cd /quasi-msys2-w32/
echo MINGW32 >msystem.txt
make install _gcc _SDL2
cd /

ln -nfs "/quasi-msys2-w32/root/mingw32" /mingw32
EOF

RUN --security=insecure <<EOF
mkdir /raspbian
debootstrap --no-check-gpg --foreign --arch=armhf --include=build-essential,cmake,libsdl2-dev bookworm /raspbian http://archive.raspbian.org/raspbian
cp /usr/bin/qemu-arm-static /raspbian/usr/bin
chroot /raspbian /debootstrap/debootstrap --second-stage

mkdir /bookworm
debootstrap --no-check-gpg --foreign --arch=arm64 --include=build-essential,cmake,libsdl2-dev bookworm /bookworm https://deb.debian.org/debian
cp /usr/bin/qemu-aarch64-static /bookworm/usr/bin
chroot /bookworm /debootstrap/debootstrap --second-stage
EOF

COPY ./build-sqlux-release.sh /build/

ENV LANG en_US.utf8
WORKDIR /build

CMD ["/build/build-sqlux-release.sh"]

