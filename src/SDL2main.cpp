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

static SDL_Thread *emuThread = NULL;

extern "C" void emu_shutdown()
{
    QLdone = 1;

    SDL_WaitThread(emuThread, NULL);

    QLSDLExit();
}

extern "C" int main(int argc, char *argv[])
{
    // set the homedir for the OS first
    SetHome();

    if (emulator::optionParse(argc, argv)) {
        return 0;
    }

    // Set some things that used to be set as side effects
    parse_screen(optionString("RESOLUTION"));
    verbose = optionInt("VERBOSE");

    emulator::deviceParse();

    emulator::init();

    QLSDLScreen();

    if (optionInt("SOUND")) {
        sound_enabled = initSound(optionInt("SOUND"));
    }

    emuThread = SDL_CreateThread(QLRun, "sQLux Emulator", NULL);

    QLSDLProcessEvents();

    emu_shutdown();

    return 0;
}
