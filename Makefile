TARGETS = build/sqlay3 build/sq68ux build/compile_commands.json

ALL : ${TARGETS} 
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=yes -DSUPPORT_SHADERS=yes -B build
	cmake --build build -j 8

${TARGETS} : FORCE

FORCE: ;

clean:
	rm -rf build/*
