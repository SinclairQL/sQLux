#include "EmulatorInit.hpp"

extern "C" {
    #include <SDL.h>
    #include "SDL2screen.h"
    #include <unixstuff.h>
}

int main(int argc, char *argv[])
{
    SetParams(argc, argv);
    
    emulator::init();

    QLSDLScreen();

    QLRun();

    return 0;
}