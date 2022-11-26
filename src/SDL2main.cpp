#include <filesystem>
#include <iostream>

#include "EmulatorInit.hpp"
#include "SqluxOptions.hpp"

extern "C" {
    #include <SDL.h>
    #include "debug.h"
    #include "QL_sound.h"
    #include "SDL2screen.h"
    #include "unixstuff.h"
    #include "uqlx_cfg.h"
    #include "Xscreen.h"
}

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
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

extern "C" int main(int argc, char *argv[])
{
    // set the homedir for the OS first
    SetHome();

    if (!emulator::optionParse(argc, argv)) {
        return 0;
    }

    // Set some things that used to be set as side effects
    char *resString = strdup(optionString("RESOLUTION"));
    parse_screen(resString);
    free(resString);
    verbose = optionInt("VERBOSE");

    // setup the boot_cmd if needed
    char *boot_cmd=optionString("BOOT_CMD");
    if (strlen(boot_cmd)) {
        ux_boot = 2;
        int len = strlen(boot_cmd);
        ux_bname = (char *)malloc(len + 2);
        strcpy(ux_bname, boot_cmd);
        ux_bname[len] = 0x0A;
        ux_bname[len + 1] = 0;
    }
    free(boot_cmd);

    emulator::deviceParse();

    emulator::init();

    QLSDLScreen();

    initSound(optionInt("SOUND"));

    emuThread = SDL_CreateThread(QLRun, "sQLux Emulator", NULL);

#if __EMSCRIPTEN__
    emscripten_set_main_loop(QLSDLProcessEvents, -1, 1);
#else
    QLSDLProcessEvents();
#endif

    emu_shutdown();

    return 0;
}
