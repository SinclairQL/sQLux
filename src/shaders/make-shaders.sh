#!/usr/bin/env bash
#
# Compile the fragment shaders of the SDL3 GPU renderer and write them as C
# headers, which are committed so that building sQLux needs no shader tools.
#
# Needs glslang and spirv-cross (SPIR-V for Vulkan, MSL for Metal). DXIL for
# Direct3D 12 is written too when shadercross (SDL_shadercross) is found;
# without it Windows uses the shaders through Vulkan.

set -e
cd "$(dirname "$0")"

make_header() {
	xxd -i "$1" | sed \
		-e 's/^unsigned /const unsigned /g' \
		-e 's,^const,static const,' \
		> "$1.h"
}

for f in *.frag; do
	echo "$f"
	glslang -V -S frag -o "$f.spv" "$f"
	make_header "$f.spv"
	spirv-cross --msl --msl-version 20000 --output "$f.msl" "$f.spv"
	make_header "$f.msl"
	if command -v shadercross > /dev/null; then
		shadercross "$f.spv" -s SPIRV -d DXIL -t fragment -o "$f.dxil"
		make_header "$f.dxil"
	fi
done

rm -f ./*.spv ./*.msl ./*.dxil
