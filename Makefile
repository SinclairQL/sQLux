TARGETS = build/sqlux build/compile_commands.json
TARGETS_MINGW = build/sqlux.exe

ALL : ${TARGETS} 
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=yes -B build
	cmake --build build -j 8

debug : ${TARGETS}
	cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=yes -B build-debug
	cmake --build build-debug -j 8

mingw32 : ${TARGETS_MINGW}
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=Toolchain-mingw-w64-i686.cmake -B build-mingw32
	cmake --build build-mingw32 -j 8

mingw64 : ${TARGETS_MINGW}
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=Toolchain-mingw-w64-x86_64.cmake -B build-mingw64
	cmake --build build-mingw64 -j 8

wasm :
	emcmake cmake -DCMAKE_BUILD_TYPE=Release -B build-wasm
	emmake make -C build-wasm

install :
	cmake --install build

${TARGETS} : FORCE
${TARGETS_MINGW} : FORCE

FORCE: ;

clean:
	rm -rf build/*
