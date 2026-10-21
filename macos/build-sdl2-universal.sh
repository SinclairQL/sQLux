#!/bin/sh
# Build a static, universal (arm64 + x86_64) SDL2 for the macOS app bundle.
# Usage: macos/build-sdl2-universal.sh [prefix]   (default: deps/macos)
set -eu

SDL2_VERSION="${SDL2_VERSION:-2.32.10}"
DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-11.0}"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PREFIX="${1:-$ROOT/deps/macos}"
WORK="$ROOT/deps/src"

mkdir -p "$WORK"
cd "$WORK"

if [ ! -d "SDL2-$SDL2_VERSION" ]; then
	curl -fL -o "SDL2-$SDL2_VERSION.tar.gz" \
		"https://github.com/libsdl-org/SDL/releases/download/release-$SDL2_VERSION/SDL2-$SDL2_VERSION.tar.gz"
	tar xzf "SDL2-$SDL2_VERSION.tar.gz"
fi

cmake -S "SDL2-$SDL2_VERSION" -B "SDL2-$SDL2_VERSION/build-universal" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
	-DCMAKE_OSX_DEPLOYMENT_TARGET="$DEPLOYMENT_TARGET" \
	-DCMAKE_INSTALL_PREFIX="$PREFIX" \
	-DCMAKE_C_FLAGS="-ffile-prefix-map=$WORK/=" \
	-DCMAKE_OBJC_FLAGS="-ffile-prefix-map=$WORK/=" \
	-DSDL_SHARED=OFF -DSDL_STATIC=ON -DSDL_TEST=OFF

cmake --build "SDL2-$SDL2_VERSION/build-universal" --parallel "$(sysctl -n hw.ncpu)"
cmake --install "SDL2-$SDL2_VERSION/build-universal"

lipo -info "$PREFIX/lib/libSDL2.a"
