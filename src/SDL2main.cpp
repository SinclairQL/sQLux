#include "EmulatorInit.hpp"

extern "C" {
    #include <SDL.h>
    #include "QL_sound.h"
    #include "SDL2screen.h"
    #include "unixstuff.h"
    #include "uqlx_cfg.h"
}

static SDL_Thread *emuThread = NULL;

int main(int argc, char *argv[])
{
    SetParams(argc, argv);
    
    emulator::init();

    QLSDLScreen();

    sound_enabled = initSound(QMD.sound);

    emuThread = SDL_CreateThread(QLRun, "sQLux Emulator", NULL);

    QLSDLProcessEvents();

    return 0;
}

extern "C" void shutdown()
{
    QLdone = 1;

    SDL_WaitThread(emuThread, NULL);

    QLSDLExit();
}