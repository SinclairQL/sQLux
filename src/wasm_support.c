#include "emscripten.h"
#include "wasm_support.h"

void wasm_reload() {
	MAIN_THREAD_ASYNC_EM_ASM("window.location.reload()");
}

EM_JS(int, wasm_does_boot_file_exist, (), {
    if (!FS.analyzePath(localWinFile).exists) {
        return 0;
    }
    return 1;
});

EM_JS(void, wasm_init_storage, (), {
    var localStoragePath = '/local';
    var defaultWinFile = 'win1.win';
    localWinFile = localStoragePath + '/' + defaultWinFile;
    FS.mkdir(localStoragePath);
    FS.mount(IDBFS, {}, localStoragePath);
    FS.syncfs(
        true, function(err) {
            if (!FS.analyzePath(localWinFile).exists) {
                console.log('Copy default win1 file.');
                var wincontent =
                  	FS.readFile('/default_win1.win');
                FS.writeFile(localWinFile, wincontent);
                FS.syncfs(false, function(err){});
            }
    });
});
