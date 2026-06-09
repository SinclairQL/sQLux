/*
 * Copyright (c) 2020-2021 Graeme Gregory
 *
 * SPDX: Zlib
 */

#ifndef _SDL3SCREEN_H
#define _SDL3SCREEN_H

#include <SDL3/SDL.h>
#include <stdbool.h>

int QLSDLScreen(void);
void QLSDLRenderScreen(void);
void QLSDLProcessEvents(void);
void QLSDLExit(void);
void QLSDLUpdateScreenWord(uint32_t, uint16_t);
void QLSDLUpdateScreenLong(uint32_t, uint32_t);
void QLSDLWritePixels(uint32_t *pixelPtr32);

void QLSDLCreatePalette(SDL_PixelFormat format);
void QLSDLCreateIcon(SDL_Window *window);

extern unsigned int sdl_keyrow[8];
extern int sdl_shiftstate, sdl_controlstate, sdl_altstate;

extern SDL_AtomicInt doPoll;
extern SDL_Semaphore *sem50Hz;
extern bool ql_fullscreen;
extern double ql_screen_ratio;

#define USER_CODE_SCREENREFRESH 0
#define USER_CODE_EMUEXIT 1

#endif
