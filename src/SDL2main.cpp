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
    emulator::optionParse(argc, argv);

    parse_screen(optionString("resolution"));

    // Set some things that used to be set as side effects
    SetHome();
    verbose = optionInt("verbose");

    emulator::init();

    QLSDLScreen();

    sound_enabled = initSound(optionInt("sound"));

    emuThread = SDL_CreateThread(QLRun, "sQLux Emulator", NULL);

    QLSDLProcessEvents();

    emu_shutdown();

    return 0;
}
