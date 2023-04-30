#include <filesystem>
#include <iostream>

#include "EmulatorInit.hpp"
#include "SqluxOptions.hpp"

extern "C" {
    #include <SDL.h>
    #include "debug.h"
    #include "emulator_options.h"
    #include "QL_sound.h"
    #include "SDL2screen.h"
    #include "unixstuff.h"
    #include "uqlx_cfg.h"
    #include "Xscreen.h"
}

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
extern "C" {
    #include "wasm_support.h"
}
#endif

static SDL_Thread *emuThread = NULL;

void CleanRAMDev()
{
    int i, j;

    for (i = 0; i < MAXDEV; i++) {
        if (qdevs[i].qname && strcmp(qdevs[i].qname, "RAM") == 0) {
            for (j = 0; j < 8; j++) {
                if ((qdevs[i].mountPoints[j] != NULL) &&
                    qdevs[i].clean[j]) {
                    if (V2) {
                        std::cout << "Cleaning: " << qdevs[i].mountPoints[j] << "\n";
                    }
                    std::filesystem::remove_all(qdevs[i].mountPoints[j]);
                }
            }
            break;
        }
    }
}

extern "C" void emu_shutdown()
{
    QLdone = 1;

    SDL_WaitThread(emuThread, NULL);

    QLSDLExit();

    CleanRAMDev();
}

extern "C" void emu_loop() {
    static int init_done = 0;
    int boot_file_ready = 1;

#if __EMSCRIPTEN__
    if(!init_done) {
        boot_file_ready = wasm_does_boot_file_exist();
    }
#endif
    if(boot_file_ready && !init_done) {
        emulator::deviceParse();
        emulator::init();
        QLSDLScreen();
        initSound(emulatorOptionInt("sound"));
        emuThread = SDL_CreateThread(QLRun, "sQLux Emulator", NULL);
        init_done = 1;
    }
    if(init_done) {
        QLSDLProcessEvents();
    }
}

extern "C" int main(int argc, char *argv[])
{
#if __EMSCRIPTEN__
    wasm_init_storage();
#endif
    // set the homedir for the OS first
    SetHome();

    emulatorOptionParse(argc, argv);

    if (!emulator::optionParse(argc, argv)) {
        return 0;
    }

    // Set some things that used to be set as side effects
    char *resString = emulatorOptionString("resolution");
    parse_screen(resString);
    verbose = emulatorOptionInt("verbose");

    // setup the boot_cmd if needed
    char *boot_cmd=emulatorOptionString("boot_cmd");
    if (strlen(boot_cmd)) {
        ux_boot = 2;
        int len = strlen(boot_cmd);
        ux_bname = (char *)malloc(len + 2);
        strcpy(ux_bname, boot_cmd);
        ux_bname[len] = 0x0A;
        ux_bname[len + 1] = 0;
    }


#if __EMSCRIPTEN__
    emscripten_set_main_loop(emu_loop, -1, 1);
#else
    emu_loop();
#endif

    emu_shutdown();

    return 0;
}
