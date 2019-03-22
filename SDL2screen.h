#ifndef _SDL2SCREEN_H
#define _SDL2SCREEN_H

#include <SDL2/SDL.h>

int QLSDLScreen(void);
int QLSDLRenderScreen(void);
int QLSDLProcessEvents(void);
void QLSDLExit(void);
Uint32 QLSDL50Hz(Uint32 interval, void *param);

extern unsigned int sdl_keyrow[8];
extern int sdl_shiftstate,sdl_controlstate, sdl_altstate;

extern SDL_atomic_t doPoll;

#endif

