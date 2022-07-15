#include "EmulatorInit.hpp"

extern "C" {
    #include <SDL.h>
    #include "SDL2screen.h"
    #include <unixstuff.h>
}

static SDL_Thread *emuThread = NULL;

int main(int argc, char *argv[])
{
    SetParams(argc, argv);
    
    emulator::init();

    QLSDLScreen();

    SDL_CreateThread(QLRun, "sQLux Emulator", NULL);

    QLSDLProcessEvents();

    return 0;
}