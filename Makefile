TARGETS = build/sqlux build/compile_commands.json
TARGETS_MINGW = build/sqlux.exe

ALL : ${TARGETS} 
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=yes -DSUPPORT_SHADERS=yes -B build
	cmake --build build -j 8

mingw32 : ${TARGETS_MINGW}
	cmake -DCMAKE_TOOLCHAIN_FILE=Toolchain-mingw-w64-i686.cmake -B build-mingw32
	cmake --build build-mingw32 -j 8

mingw64 : ${TARGETS_MINGW}
	cmake -DCMAKE_TOOLCHAIN_FILE=Toolchain-mingw-w64-x86_64.cmake -B build-mingw64
	cmake --build build-mingw64 -j 8

wasm : ${TARGETS}
	emcmake cmake -B build-wasm
	make -C build-wasm

# Universal (arm64 + x86_64) sQLux.app with a statically linked SDL2
macos :
	cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DMACOS_APP_BUNDLE=ON \
		-B build-macos
	cmake --build build-macos

install :
	cmake --install build

${TARGETS} : FORCE
${TARGETS_MINGW} : FORCE

.PHONY: macos

FORCE: ;

clean:
	rm -rf build*
