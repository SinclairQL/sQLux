/*
 * Copyright (c) 2020-2021 Graeme Gregory
 *
 * SPDX: Zlib
 */

#ifndef _SDL2SCREEN_H
#define _SDL2SCREEN_H

#include <SDL.h>
#include <stdbool.h>

void QLSDLScreen(void);
void QLSDLRenderScreen(void);
void QLSDLProcessEvents(void);
void QLSDLExit(void);
Uint32 QLSDL50Hz(Uint32 interval, void *param);
void QLSDLWritePixels(uint32_t *pixel_ptr32);
void QLSDLUpdateScreenWord(uint32_t, uint16_t);
void QLSDLUpdateScreenLong(uint32_t, uint32_t);

void QLSDLCreatePalette(const SDL_PixelFormat *format);
void QLSDLCreateIcon(SDL_Window *window);

extern unsigned int sdl_keyrow[8];
extern int sdl_shiftstate, sdl_controlstate, sdl_altstate;

extern SDL_atomic_t doPoll;
extern SDL_sem* sem50Hz;
extern bool screenWritten;
extern bool shaders_selected;
extern bool ql_fullscreen;
extern double ql_screen_ratio;


#define USER_CODE_SCREENREFRESH     0
#define USER_CODE_EMUEXIT           1

#endif
