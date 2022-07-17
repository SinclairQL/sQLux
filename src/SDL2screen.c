/*
 * Copyright (c) 2020-2021 Graeme Gregory
 *
 * SPDX: Zlib
 */

#include <inttypes.h>
#include <math.h>
#include <SDL.h>
#include <string.h>

#include "debug.h"
#include "QL_hardware.h"
#include "uqlx_cfg.h"
#include "QL68000.h"
#include "SDL2screen.h"
#include "qlmouse.h"
#include "QL_screen.h"
#include "unixstuff.h"
#include "QL_sound.h"

#define SWAP_SHIFT 0x100

static SDL_Window *ql_window = NULL;
static uint32_t ql_windowid = 0;
static SDL_Surface *ql_screen = NULL;
static SDL_Renderer *ql_renderer = NULL;
static SDL_Texture *ql_texture = NULL;
static SDL_Rect dest_rect;
static SDL_DisplayMode sdl_mode;
static SDL_TimerID fiftyhz_timer;
const char *sdl_video_driver;
static char sdl_win_name[128];
static char ql_fullscreen = false;

SDL_atomic_t doPoll;
bool screenWritten = false;	// True if screen memory has been written

SDL_sem* sem50Hz = NULL;

struct QLcolor {
	int r;
	int g;
	int b;
};

struct QLcolor QLcolors[16] = {
	{ 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0xFF }, { 0xFF, 0x00, 0x00 },
	{ 0xFF, 0x00, 0xFF }, { 0x00, 0xFF, 0x00 }, { 0x00, 0xFF, 0xFF },
	{ 0xFF, 0xFF, 0x00 }, { 0xFF, 0xFF, 0xFF },
	{ 0x3f, 0x3f, 0x3f }, { 0x00, 0x00, 0x7f }, { 0x7f, 0x00, 0x00 },
	{ 0x7f, 0x00, 0x7f }, { 0x00, 0x7f, 0x00 }, { 0x00, 0x7f, 0x7f },
	{ 0x7f, 0x7f, 0x00 }, { 0x7f, 0x7f, 0x7f },
};

struct QLcolor QLcolors_gray[16] = {
	{ 0x00, 0x00, 0x00 }, { 0x12, 0x12, 0x12 }, { 0x36, 0x36, 0x36 },
	{ 0x48, 0x48, 0x48 }, { 0xB6, 0xB6, 0xB6 }, { 0xC8, 0xC8, 0xC8 },
	{ 0xEC, 0xEC, 0x00 }, { 0xFF, 0xFF, 0xFF },
	{ 0x3f, 0x3f, 0x3f }, { 0x09, 0x09, 0x09 }, { 0x1B, 0x1B, 0x1B },
	{ 0x24, 0x00, 0x24 }, { 0x5A, 0x5A, 0x5A }, { 0x63, 0x63, 0x63 },
	{ 0x75, 0x75, 0x75 }, { 0x7f, 0x7f, 0x7f },
};

uint32_t SDLcolors[16];

struct SDLQLMap {
	SDL_Keycode sdl_kc;
	int code;
	int qchar;
};

static struct SDLQLMap *sdlqlmap;
static void setKeyboardLayout (void);

/* GIMP RGBA C-Source image dump (sQLuxLogo2.c) */

static const struct {
  unsigned  	 width;
  unsigned  	 height;
  unsigned  	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
  uint8_t 	 pixel_data[32 * 32 * 4 + 1];
} sqluxlogo = {
  32, 32, 4,
  "\377\377\377\377\000\000\000\377\377\377\377\377\000\000\000\377\377\377\377\377\000\000"
  "\000\377\377\377\377\377\000\000\000\377\377\377\377\377\000\000\000\377\377\377\377\377"
  "\000\000\000\377\377\377\377\377\000\000\000\377\377\377\377\377\000\000\000\377\377\377\377"
  "\377\000\000\000\377\377\377\377\377\000\000\000\377\377\377\377\377\000\000\000\377\377\377"
  "\377\377\000\000\000\377\377\377\377\377\000\000\000\377\377\377\377\377\000\000\000\377\377"
  "\377\377\377\000\000\000\377\377\377\377\377\000\000\000\377\000\000\000\377\377\377\377\377"
  "\000\000\000\377\377\377\377\377\000\000\000\377\377\377\377\377\000\000\000\377\377\377\377"
  "\377\000\000\000\377\377\377\377\377\000\000\000\377\377\377\377\377\000\000\000\377\377\377"
  "\377\377\000\000\000\377\377\377\377\377\000\000\000\377\377\000\000\377\000\000\000\377\377\000"
  "\000\377\000\000\000\377\377\000\000\377\000\000\000\377\377\000\000\377\000\000\000\377\377\000\000\377"
  "\000\000\000\377\377\000\000\377\000\000\000\377\377\000\000\377\000\000\000\377\377\377\377\377\377"
  "\377\377\377\000\000\000\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\000\000\000\377\377\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\000\000\000"
  "\377\000\000\000\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\000\000\000\377\377\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\000\000\000\377\377\377"
  "\377\377\377\377\377\377\000\000\000\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\000\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000"
  "\000\377\000\000\000\377\000\000\000\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\000\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\000\000"
  "\000\377\377\377\377\377\377\377\377\377\000\000\000\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\000\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\000\000\000\377\000\000\000\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\000\000\000\377"
  "\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377"
  "\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377"
  "\377\000\000\377\000\000\000\377\377\377\377\377\377\377\377\377\000\000\000\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\000\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377"
  "\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377"
  "\377\000\000\377\377\000\000\377\377\000\000\377\000\000\000\377\000\000\000\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\000\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\000\000\000\377\377\377\377\377\377\377\377\377"
  "\000\000\000\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\000\000\000\377\377\000\000\377\377\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\000\000\000\377\000\000\000\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\000\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\000\000\000\377\377\377\377\377\377"
  "\377\377\377\000\000\000\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\000\000\000\377\377\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\000\000\000"
  "\377\000\000\000\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\000\000\000\377\377\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\000\000\000\377\377\377"
  "\377\377\377\377\377\377\000\000\000\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\000\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000"
  "\000\377\000\000\000\377\000\000\000\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\000\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\000\000"
  "\000\377\377\377\377\377\377\377\377\377\000\000\000\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\000\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377"
  "\000\000\377\377\000\000\377\000\000\000\377\011\000\000\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\000\000\000\377"
  "\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377"
  "\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377"
  "\377\000\000\377\000\000\000\377\377\377\377\377\377\377\377\377\000\000\000\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\000\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377"
  "\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377"
  "\377\000\000\377\377\000\000\377\377\000\000\377\000\000\000\377\000\000\000\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377\377"
  "\377\377\000\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000\000\377\377\000"
  "\000\377\377\000\000\377\377\000\000\377\000\000\000\377\377\377\377\377\000\000\000\377\000\000\000"
  "\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000"
  "\000\000\377\000\000\000\377\377\000\000\377\000\000\000\377\377\000\000\377\000\000\000\377\377\000\000\377"
  "\000\000\000\377\377\000\000\377\000\000\000\377\377\000\000\377\000\000\000\377\377\000\000\377\000\000\000"
  "\377\377\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000"
  "\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000"
  "\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000"
  "\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000"
  "\377\000\377\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000"
  "\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000"
  "\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377"
  "\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000"
  "\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\000\377\000\377\000\377\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\377\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\377\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\377\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\377\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377"
  "\000\000\000\377\000\377\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\377\000\377\000\377\000\377\000\377\000\377\000\000\000\377\000\000\000"
  "\377\000\377\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000"
  "\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\377\000\377\000\000\000\377"
  "\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\377\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377\000\377\000\377\000\000\000"
  "\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\377\000\377\000\377"
  "\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\377\000\377\000\000\000\377\000"
  "\377\000\377\000\000\000\377\000\377\000\377\000\377\000\377\000\377\000\377\000\377\000\377\000\377"
  "\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\377\000\377\000\377\000\377\000\377\000\377"
  "\000\000\000\377\000\377\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\377\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000"
  "\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000"
  "\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000"
  "\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000"
  "\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000"
  "\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000"
  "\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000"
  "\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377"
  "\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000\000\000\377\000"
  "\000\000\377\000\000\000\377\000\000\000\377",
};

#ifndef SDL_JOYSTICK_DISABLED
typedef struct {
	SDL_Joystick* sdl_id;	// Assigned SDL handle
	int which;		// Index into SDL joysticks list
	int left_axis;		// SDL axis id for left / right
	int up_axis;		// SDL axis id for up / down
} joy_data;

int joy_char[2][5] = { {49, 52, 50, 55, 54}, 	// left, right, up, down, fire
		        {57, 60, 56, 59, 61} };

static joy_data joy[2] = { {NULL, -1, 0, 1},
			   {NULL, -1, 0, 1} };
#endif

static void QLSDLInitJoystick(void);
static void QLSDLOpenJoystick(int index, int which);
static void QLProcessJoystickAxis(Sint32 which, Uint8 axis, Sint16 value);
static void QLProcessJoystickButton(Sint32 which, Sint16 button, Sint16 pressed);
static int QLConvertWhichToIndex(Sint32 which);


void QLSDLScreen(void)
{
	uint32_t sdl_window_mode;
	int i, w, h, ay;
	SDL_Surface *icon;
	uint32_t rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	int shift = (sqluxlogo.bytes_per_pixel == 3) ? 8 : 0;
	rmask = 0xff000000 >> shift;
	gmask = 0x00ff0000 >> shift;
	bmask = 0x0000ff00 >> shift;
	amask = 0x000000ff >> shift;
#else // little endian, like x86
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = (sqluxlogo.bytes_per_pixel == 3) ? 0 : 0xff000000;
#endif

	snprintf(sdl_win_name, 128, "sQLux - %s, %dK", QMD.sysrom, RTOP / 1024);

	Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_TIMER;
#ifndef SDL_JOYSTICK_DISABLED
	flags |= SDL_INIT_JOYSTICK;
#endif
	if (SDL_Init(flags) < 0) {
		printf("SDL_Init Error: %s\n", SDL_GetError());
		exit(-1);
	}

	sdl_video_driver = SDL_GetCurrentVideoDriver();
	SDL_GetCurrentDisplayMode(0, &sdl_mode);

	if (V1)
		printf("Video Driver %s xres %d yres %d\n", sdl_video_driver,
			sdl_mode.w, sdl_mode.h);

	/* Fix the aspect ratio to more like real hardware */
	if (QMD.aspect) {
		ay = (qlscreen.yres * 3) / 2;
	} else {
		ay = qlscreen.yres;
	}

	/* Ensure width and height are always initialised to sane values */
	w = qlscreen.xres;
	h = ay;

	/* Initialize keyboard table (doesn't belong here, but was convenient...) */
	setKeyboardLayout ();

	if (sdl_video_driver != NULL &&
	    (strcmp(sdl_video_driver, "x11") == 0) ||
	    (strcmp(sdl_video_driver, "cocoa") == 0) ||
	    (strcmp(sdl_video_driver, "windows") == 0) ||
	    (strcmp(sdl_video_driver, "wayland") == 0) &&
	    sdl_mode.w >= 800 &&
	    sdl_mode.h >= 600) {
		sdl_window_mode = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

		if (!strcmp("2x", QMD.winsize)) {
			w = qlscreen.xres * 2;
			h = ay * 2;
		} else if (!strcmp("3x", QMD.winsize)) {
			w = qlscreen.xres * 3;
			h = ay * 3;
		} else if (!strcmp("max", QMD.winsize)) {
			sdl_window_mode |= SDL_WINDOW_MAXIMIZED;
		} else if (!strcmp("full", QMD.winsize)) {
			sdl_window_mode |= SDL_WINDOW_FULLSCREEN_DESKTOP;
			ql_fullscreen = true;
		}
	} else {
		sdl_window_mode = SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	ql_window =
		SDL_CreateWindow(sdl_win_name, SDL_WINDOWPOS_CENTERED,
				 SDL_WINDOWPOS_CENTERED, w, h, sdl_window_mode);

	icon = SDL_CreateRGBSurfaceFrom((void*)sqluxlogo.pixel_data,
				sqluxlogo.width,
				sqluxlogo.height,
				sqluxlogo.bytes_per_pixel * 8,
				sqluxlogo.bytes_per_pixel * sqluxlogo.width,
				rmask, gmask, bmask, amask);

	SDL_SetWindowIcon(ql_window, icon);
	SDL_FreeSurface(icon);

	if (ql_window == NULL) {
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
		exit(-1);
	}

	ql_windowid = SDL_GetWindowID(ql_window);

	ql_renderer = SDL_CreateRenderer(ql_window, -1,
					 SDL_RENDERER_ACCELERATED |
						 SDL_RENDERER_PRESENTVSYNC);

	SDL_RenderSetLogicalSize(ql_renderer, qlscreen.xres, ay);

	dest_rect.x = dest_rect.y = 0;
	dest_rect.w = qlscreen.xres;
	dest_rect.h = ay;

	SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

	if (QMD.filter)
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	ql_screen = SDL_CreateRGBSurfaceWithFormat(
		0, qlscreen.xres, qlscreen.yres, 32, SDL_PIXELFORMAT_RGBA32);

	if (ql_screen == NULL) {
		printf("Error Creating Surface\n");
		exit(-1);
	}

	ql_texture = SDL_CreateTexture(ql_renderer, SDL_PIXELFORMAT_RGBA32,
				       SDL_TEXTUREACCESS_STREAMING,
				       ql_screen->w, ql_screen->h);

	for (i = 0; i < 16; i++) {
		if (QMD.gray) {
			SDLcolors[i] = SDL_MapRGB(ql_screen->format, QLcolors_gray[i].r,
						  QLcolors_gray[i].g, QLcolors_gray[i].b);
		} else {
			SDLcolors[i] = SDL_MapRGB(ql_screen->format, QLcolors[i].r,
						  QLcolors[i].g, QLcolors[i].b);
		}
	}

	SDL_RenderClear(ql_renderer);
	SDL_RenderPresent(ql_renderer);

	QLSDLInitJoystick();

	SDL_AtomicSet(&doPoll, 0);
	sem50Hz = SDL_CreateSemaphore(0);
	fiftyhz_timer = SDL_AddTimer(20, QLSDL50Hz, NULL);
}

void QLSDLUpdateScreenByte(uint32_t offset, uint8_t data)
{
	int t1, t2, i, color;
	uint16_t dataword = 0;

	if (offset & 1) {
		offset--;
		dataword = (uint8_t)ReadByte(qlscreen.qm_lo + offset) << 8;
		dataword |= data & 0xFF;
	} else {
		dataword = (uint16_t)data << 8;
		dataword |=
			(uint8_t)ReadByte((qlscreen.qm_lo + offset) + 1) & 0xFF;
	}

	QLSDLUpdateScreenWord(offset, dataword);
}

void QLSDLUpdateScreenWord(uint32_t offset, uint16_t data)
{
	int t1, t2, i, color;
	uint32_t *pixel_ptr32;

	t1 = data >> 8;
	t2 = data & 0xFF;

	if (SDL_MUSTLOCK(ql_screen)) {
		SDL_LockSurface(ql_screen);
	}

	pixel_ptr32 = ql_screen->pixels + (offset * 16);

	if (display_mode == 8) {
		for (i = 0; i < 8; i += 2) {
			uint32_t x;

			color = ((t1 & 2) << 1) + ((t2 & 3)) + ((t1 & 1) << 3);

			x = SDLcolors[color];

			*(pixel_ptr32 + 7 - (i)) = x;
			*(pixel_ptr32 + 7 - (i + 1)) = x;

			t1 >>= 2;
			t2 >>= 2;
		}
	} else {
		for (i = 0; i < 8; i++) {
			uint32_t x;

			color = ((t1 & 1) << 2) + ((t2 & 1) << 1) +
				((t1 & 1) & (t2 & 1));

			x = SDLcolors[color];

			*(pixel_ptr32 + 7 - i) = x;

			t1 >>= 1;
			t2 >>= 1;
		}
	}

	if (SDL_MUSTLOCK(ql_screen)) {
		SDL_UnlockSurface(ql_screen);
	}
}

void QLSDLUpdateScreenLong(uint32_t offset, uint32_t data)
{
	QLSDLUpdateScreenWord(offset, data >> 16);
	QLSDLUpdateScreenWord(offset + 2, data & 0xFFFF);
}

void QLSDLUpdatePixelBuffer()
{
	uint8_t *scr_ptr = (void *)memBase + qlscreen.qm_lo;
	uint32_t *pixel_ptr32;
	int t1, t2, i, color;

	if (SDL_MUSTLOCK(ql_screen)) {
		SDL_LockSurface(ql_screen);
	}

	pixel_ptr32 = ql_screen->pixels;

	while (scr_ptr <
	       (uint8_t *)((void *)memBase + qlscreen.qm_lo + qlscreen.qm_len)) {
		t1 = *scr_ptr++;
		t2 = *scr_ptr++;

		if (display_mode == 8) {
			for (i = 0; i < 8; i += 2) {
				uint32_t x;

				color = ((t1 & 2) << 1) + ((t2 & 3)) +
					((t1 & 1) << 3);

				x = SDLcolors[color];

				*(pixel_ptr32 + 7 - (i)) = x;
				*(pixel_ptr32 + 7 - (i + 1)) = x;

				t1 >>= 2;
				t2 >>= 2;
			}
		} else {
			for (i = 0; i < 8; i++) {
				uint32_t x;

				color = ((t1 & 1) << 2) + ((t2 & 1) << 1) +
					((t1 & 1) & (t2 & 1));

				x = SDLcolors[color];

				*(pixel_ptr32 + 7 - i) = x;

				t1 >>= 1;
				t2 >>= 1;
			}
		}
		pixel_ptr32 += 8;
	}

	if (SDL_MUSTLOCK(ql_screen)) {
		SDL_UnlockSurface(ql_screen);
	}
}

void QLSDLRenderScreen(void)
{
	void *texture_buffer;
	int pitch;
	int w, h;
	SDL_PixelFormat pixelformat;

	SDL_UpdateTexture(ql_texture, NULL, ql_screen->pixels,
			  ql_screen->pitch);
	SDL_RenderClear(ql_renderer);
	SDL_RenderCopyEx(ql_renderer, ql_texture, NULL, &dest_rect, 0, NULL,
			 SDL_FLIP_NONE);
	SDL_RenderPresent(ql_renderer);
}

void SDLQLFullScreen(void)
{
	int w, h;

	ql_fullscreen ^= 1;

	//if (ql_fullscreen)
	//	SDL_RestoreWindow(ql_window);
	SDL_SetWindowFullscreen(
		ql_window, ql_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	//if (!ql_fullscreen)
	//	SDL_MaximizeWindow(ql_window);

	QLSDLProcessEvents();
}

static void QLSDLInitJoystick(void)
{
#ifndef SDL_JOYSTICK_DISABLED
	// Open joystick 1 and 2, if defined
	QLSDLOpenJoystick(0,QMD.joy1);
	QLSDLOpenJoystick(1,QMD.joy2);
#endif
}

static void QLSDLOpenJoystick(int index, int which)
{
#ifndef SDL_JOYSTICK_DISABLED
	if ((which > 0) && (which < 9)) {
		// Convert from 1-base to 0-base
		int which0 = which - 1;
		int joysticks = SDL_NumJoysticks();

		if (joysticks) {
			// Check for duplicate indexes
			if (QLConvertWhichToIndex(which0) == -1) {
				if (joysticks > which0) {
					joy[index].sdl_id = SDL_JoystickOpen(which0);
					if (joy[index].sdl_id == NULL) {
						if (V1)	printf("Joystick %i initialisation failed\n",
								index + 1);
					}
					else {
						joy[index].which = which0;
						if (V1)	printf("Joystick %i initialised\n",
								index + 1);
					}
				}
				else {
					if (V1) printf("Joystick %i initialisation failed. Index %i too high\n",
							index + 1, which);
				}
			}
			else {
				if (V1) printf("Joystick %i initialisation failed. Duplicate index %i\n",
						index + 1, which);
			}
		}
		else {
			if (V1) printf("No joysticks detected by SDL2\n");
		}
	}
	else {
		if ((V1) && which)
			printf("Joystick %i initialisation failed. Unknown index: %i\n",
				index + 1, which);
	}
#endif
}

/* Store the keys pressed */
unsigned int sdl_keyrow[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int sdl_shiftstate, sdl_controlstate, sdl_altstate;

static void SDLQLKeyrowChg(int code, int press)
{
	int j;

	if (code > -1) {
		j = 1 << (code % 8);

		if (press)
			sdl_keyrow[7 - code / 8] |= j;
		else
			sdl_keyrow[7 - code / 8] &= ~j;
	}
}

static struct SDLQLMap sdlqlmap_DE[] = {
				      { SDLK_LEFT, 49, 0 },
				      { SDLK_UP, 50, 0 },
				      { SDLK_RIGHT, 52, 0 },
				      { SDLK_DOWN, 55, 0 },

				      { SDLK_F1, 57, 0 },
				      { SDLK_F2, 59, 0 },
				      { SDLK_F3, 60, 0 },
				      { SDLK_F4, 56, 0 },
				      { SDLK_F5, 61, 0 },

				      { SDLK_RETURN, 48, 0 },
				      { SDLK_SPACE, 54, 0 },
				      { SDLK_TAB, 19, 0 },
				      { SDLK_ESCAPE, 51, 0 },
				      { SDLK_CAPSLOCK, 33, 0},
				      { SDLK_RIGHTBRACKET, 40, 0 },
				      { SDLK_5, 58, 0 },
				      { SDLK_4, 62, 0 },
				      { SDLK_7, 63, 0 },
				      { SDLK_LEFTBRACKET, 32, 0 },
				      { SDLK_z, 22, 0 },

				      { SDLK_PERIOD, 42, 0 },
				      { SDLK_c, 43, 0 },
				      { SDLK_b, 44, 0 },
				      //{ SDLK_BACKQUOTE, 45, 0 },

				      { SDLK_m, 46, 0 },
				      { SDLK_QUOTE, 47, 0 },
				      { SDLK_BACKSLASH, 53, 0 },
				      { SDLK_k, 34, 0 },
				      { SDLK_s, 35, 0 },
				      { SDLK_f, 36, 0 },
				      { SDLK_EQUALS, 37, 0 },
				      { SDLK_g, 38, 0 },
				      { SDLK_SEMICOLON, 39, 0 },
				      { SDLK_l, 24, 0 },
				      { SDLK_3, 25, 0 },
				      { SDLK_h, 26, 0 },
				      { SDLK_1, 27, 0 },
				      { SDLK_a, 28, 0 },
				      { SDLK_p, 29, 0 },
				      { SDLK_d, 30, 0 },
				      { SDLK_j, 31, 0 },
				      { SDLK_9, 16, 0 },
				      { SDLK_w, 17, 0 },
				      { SDLK_i, 18, 0 },
				      { SDLK_r, 20, 0 },
				      { SDLK_MINUS, 69, 0 }, /* minus */
				      { SDLK_y, 41, 0 },
				      { SDLK_o, 23, 0 },
				      { SDLK_8, 8, 0 },
				      { SDLK_2, 9, 0 },
				      { SDLK_6, 10, 0 },
				      { SDLK_q, 11, 0 },
				      { SDLK_e, 12, 0 },
				      { SDLK_0, 13, 0 },
				      { SDLK_t, 14, 0 },
				      { SDLK_u, 15, 0 },
				      { SDLK_x, 3, 0 },
				      { SDLK_v, 4, 0 },
				      { SDLK_SLASH, 5, 0 },
				      { SDLK_n, 6, 0 },
				      { SDLK_COMMA, 7, 0 },
				      { 180, 45, 0 },       /* accent */
				      { 228, 47, 0 }, 	    /* Ä OK */
				      { 246, 103, 0 },       /* Ö OK */
				      { 252, 32, 0 },       /* Ü OK */
				      { 94, 53, 0 },       /* < OK */
				      { 35, 101, 0 },       /* # OK */
				      { 43, 40, 0 },       /* + OK */
				      { 223, 21, 0 },       /* ß OK */
				      { 60, 109, 0 },       /* ^ */

				      /* Accent ^ and \ is 109 */

				      /*
  {SDLK_Next,-1,220},
  {SDLK_Prior,-1,212},
  {SDLK_Home,-1,193},
  {SDLK_End,-1,201},
		 */
				      { 0, 0, 0 } };

static struct SDLQLMap sdlqlmap_GB[] = {
				      { SDLK_LEFT, 49, 0 },
				      { SDLK_UP, 50, 0 },
				      { SDLK_RIGHT, 52, 0 },
				      { SDLK_DOWN, 55, 0 },

				      { SDLK_F1, 57, 0 },
				      { SDLK_F2, 59, 0 },
				      { SDLK_F3, 60, 0 },
				      { SDLK_F4, 56, 0 },
				      { SDLK_F5, 61, 0 },

				      { SDLK_RETURN, 48, 0 },
				      { SDLK_SPACE, 54, 0 },
				      { SDLK_TAB, 19, 0 },
				      { SDLK_ESCAPE, 51, 0 },
				      { SDLK_CAPSLOCK, 33, 0 },
				      { SDLK_RIGHTBRACKET, 40, 0 },
				      { SDLK_5, 58, 0 },
				      { SDLK_4, 62, 0 },
				      { SDLK_7, 63, 0 },
				      { SDLK_LEFTBRACKET, 32, 0 },
				      { SDLK_z, 41, 0 },

				      { SDLK_PERIOD, 42, 0 },
				      { SDLK_c, 43, 0 },
				      { SDLK_b, 44, 0 },
				      //{ SDLK_BACKQUOTE, 45, 0 },

				      { SDLK_m, 46, 0 },
				      { SDLK_QUOTE, 47, 9 },
				      { SDLK_BACKSLASH, 53, 0 },
				      { SDLK_k, 34, 0 },
				      { SDLK_s, 35, 0 },
				      { SDLK_f, 36, 0 },
				      { SDLK_EQUALS, 37, 0 },
				      { SDLK_g, 38, 0 },
				      { SDLK_SEMICOLON, 39, 0 },
				      { SDLK_l, 24, 0 },
				      { SDLK_3, 25, (SWAP_SHIFT | 45) },
				      { SDLK_h, 26, 0 },
				      { SDLK_1, 27, 0 },
				      { SDLK_a, 28, 0 },
				      { SDLK_p, 29, 0 },
				      { SDLK_d, 30, 0 },
				      { SDLK_j, 31, 0 },
				      { SDLK_9, 16, 0 },
				      { SDLK_w, 17, 0 },
				      { SDLK_i, 18, 0 },
				      { SDLK_r, 20, 0 },
				      { SDLK_MINUS, 21, 0 },
				      { SDLK_y, 22, 0 },
				      { SDLK_o, 23, 0 },
				      { SDLK_8, 8, 0 },
				      { SDLK_2, 9, 47 },
				      { SDLK_6, 10, 0 },
				      { SDLK_q, 11, 0 },
				      { SDLK_e, 12, 0 },
				      { SDLK_0, 13, 0 },
				      { SDLK_t, 14, 0 },
				      { SDLK_u, 15, 0 },
				      { SDLK_x, 3, 0 },
				      { SDLK_v, 4, 0 },
				      { SDLK_SLASH, 5, 0 },
				      { SDLK_n, 6, 0 },
				      { SDLK_COMMA, 7, 0 },
				      { SDLK_HASH, (SWAP_SHIFT | 25), 45},
				      /*
  {SDLK_Next,-1,220},
  {SDLK_Prior,-1,212},
  {SDLK_Home,-1,193},
  {SDLK_End,-1,201},
		 */
				      { 0, 0, 0 } };

static struct SDLQLMap sdlqlmap_default[] = {
				      { SDLK_LEFT, 49, 0 },
				      { SDLK_UP, 50, 0 },
				      { SDLK_RIGHT, 52, 0 },
				      { SDLK_DOWN, 55, 0 },

				      { SDLK_F1, 57, 0 },
				      { SDLK_F2, 59, 0 },
				      { SDLK_F3, 60, 0 },
				      { SDLK_F4, 56, 0 },
				      { SDLK_F5, 61, 0 },

				      { SDLK_RETURN, 48, 0 },
				      { SDLK_SPACE, 54, 0 },
				      { SDLK_TAB, 19, 0 },
				      { SDLK_ESCAPE, 51, 0 },
				      { SDLK_CAPSLOCK, 33, 0 },
				      { SDLK_RIGHTBRACKET, 40, 0 },
				      { SDLK_5, 58, 0 },
				      { SDLK_4, 62, 0 },
				      { SDLK_7, 63, 0 },
				      { SDLK_LEFTBRACKET, 32, 0 },
				      { SDLK_z, 41, 0 },

				      { SDLK_PERIOD, 42, 0 },
				      { SDLK_c, 43, 0 },
				      { SDLK_b, 44, 0 },
				      { SDLK_BACKQUOTE, 45, 0 },

				      { SDLK_m, 46, 0 },
				      { SDLK_QUOTE, 47, 0 },
				      { SDLK_BACKSLASH, 53, 0 },
				      { SDLK_k, 34, 0 },
				      { SDLK_s, 35, 0 },
				      { SDLK_f, 36, 0 },
				      { SDLK_EQUALS, 37, 0 },
				      { SDLK_g, 38, 0 },
				      { SDLK_SEMICOLON, 39, 0 },
				      { SDLK_l, 24, 0 },
				      { SDLK_3, 25, 0 },
				      { SDLK_h, 26, 0 },
				      { SDLK_1, 27, 0 },
				      { SDLK_a, 28, 0 },
				      { SDLK_p, 29, 0 },
				      { SDLK_d, 30, 0 },
				      { SDLK_j, 31, 0 },
				      { SDLK_9, 16, 0 },
				      { SDLK_w, 17, 0 },
				      { SDLK_i, 18, 0 },
				      { SDLK_r, 20, 0 },
				      { SDLK_MINUS, 21, 0 },
				      { SDLK_y, 22, 0 },
				      { SDLK_o, 23, 0 },
				      { SDLK_8, 8, 0 },
				      { SDLK_2, 9, 0 },
				      { SDLK_6, 10, 0 },
				      { SDLK_q, 11, 0 },
				      { SDLK_e, 12, 0 },
				      { SDLK_0, 13, 0 },
				      { SDLK_t, 14, 0 },
				      { SDLK_u, 15, 0 },
				      { SDLK_x, 3, 0 },
				      { SDLK_v, 4, 0 },
				      { SDLK_SLASH, 5, 0 },
				      { SDLK_n, 6, 0 },
				      { SDLK_COMMA, 7, 0 },
				      /*
  {SDLK_Next,-1,220},
  {SDLK_Prior,-1,212},
  {SDLK_Home,-1,193},
  {SDLK_End,-1,201},
		 */
				      { 0, 0, 0 } };

void QLSDProcessKey(SDL_Keysym *keysym, int pressed)
{
	int i = 0;

	/* Handle extended cursor keys */
	/* backspace maps to control left */
	if ((keysym->sym == SDLK_BACKSPACE) && pressed) {
		queueKey(1 << 1, 49, 0);
		return;
	}
	/* Delete maps to control right */
	if ((keysym->sym == SDLK_DELETE) && pressed) {
		queueKey(1 << 1, 52, 0);
		return;
	}
	/* Home maps to alt left */
	if ((keysym->sym == SDLK_HOME) && pressed) {
		queueKey(1 << 0, 49, 0);
		return;
	}
	/* End maps to alt right */
	if ((keysym->sym == SDLK_END) && pressed) {
		queueKey(1 << 0, 52, 0);
		return;
	}
	/* Insert maps to shift F4 */
	if ((keysym->sym == SDLK_INSERT) && pressed) {
		queueKey(1 << 2, 56, 0);
		return;
	}
	/* Page Up maps to shift down */
	if ((keysym->sym == SDLK_PAGEUP) && pressed) {
		queueKey(1 << 2, 50, 0);
		return;
	}
	/* Page Down maps to shift down */
	if ((keysym->sym == SDLK_PAGEDOWN) && pressed) {
		queueKey(1 << 2, 55, 0);
		return;
	}

	switch (keysym->sym) {
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		sdl_shiftstate = pressed;
		return;
	case SDLK_LCTRL:
	case SDLK_RCTRL:
		sdl_controlstate = pressed;
		return;
	case SDLK_LALT:
	case SDLK_RALT:
		sdl_altstate = pressed;
		return;
	case SDLK_F11:
		if (pressed)
			SDLQLFullScreen();
		return;
	}

	while (sdlqlmap[i].sdl_kc != 0) {
		if (keysym->sym == sdlqlmap[i].sdl_kc) {
			int mod = sdl_altstate | sdl_controlstate << 1 |
				  sdl_shiftstate << 2;

			/* Does non control shift generate another key code? */
			int code = ((!sdl_shiftstate) ||
				    sdl_controlstate ||
				    (!sdlqlmap[i].qchar))
				   ? sdlqlmap[i].code : sdlqlmap[i].qchar;

			/* Code requires a change in shift state? */
			if (SWAP_SHIFT & code) {
				code &= ~SWAP_SHIFT;
				mod ^= (0x1 << 2);
			}
			if (pressed) {
				queueKey(mod, code, 0);
			}
			SDLQLKeyrowChg(code, pressed);
			return; // Only one key can be mapped
		}
		i++;
	}
}

static void setKeyboardLayout (void)
{
	if (!strncmp("DE", QMD.kbd, 2)) {
		sdlqlmap = sdlqlmap_DE;
		if (V1) printf("Using DE keymap.\n");
		return;
	} else if (!strncmp("GB", QMD.kbd, 2)) {
		sdlqlmap = sdlqlmap_GB;
		if (V1) printf("Using GB keymap.\n");
		return;
	}

	sdlqlmap = sdlqlmap_default;
	if (V1) printf("Using default keymap. (use KBD=<countrycode> in sqlux.ini to change)\n");
}

static void QLSDLProcessMouse(int x, int y)
{
	int qlx = 0, qly = 0;
	float x_ratio, y_ratio;

	if (SDL_GetWindowFlags(ql_window) & SDL_WINDOW_ALLOW_HIGHDPI) {
		x *= 2;
		y *= 2;
	}

	if (x < dest_rect.x) {
		qlx = 0;
	} else if (x > (dest_rect.w + dest_rect.x)) {
		qlx = qlscreen.xres - 1;
	} else {
		x_ratio = (float)dest_rect.w / (float)qlscreen.xres;

		x -= dest_rect.x;

		qlx = ((float)x / x_ratio);
	}

	if (y < dest_rect.y) {
		qly = 0;
	} else if (y > (dest_rect.h + dest_rect.y)) {
		qly = qlscreen.yres - 1;
	} else {
		y_ratio = (float)dest_rect.h / (float)qlscreen.yres;

		y -= dest_rect.y;

		qly = ((float)y / y_ratio);
	}

	QLMovePointer(qlx, qly);
}

static int QLConvertWhichToIndex(Sint32 which)
{
	for (int i=0; i<2; ++i) {
		if (joy[i].which == which)
			return i;
	}
	return -1;
}

static void QLProcessJoystickAxis(Sint32 which, Uint8 axis, Sint16 value)
{
	int index = QLConvertWhichToIndex(which);

	if (index > -1) {
		int offset = -1;
		if (axis == joy[index].left_axis)
			offset = 0;
		else if (axis == joy[index].up_axis)
			offset = 2;

		if (offset != -1) {
			if (value < -10000) {
				queueKey(0, joy_char[index][offset], 0);
				SDLQLKeyrowChg(joy_char[index][offset], 1);
				SDLQLKeyrowChg(joy_char[index][offset + 1], 0);
			}
			else if (value > 10000) {
				queueKey(0, joy_char[index][offset + 1], 0);
				SDLQLKeyrowChg(joy_char[index][offset + 1], 1);
				SDLQLKeyrowChg(joy_char[index][offset], 0);
			}
			else {
				SDLQLKeyrowChg(joy_char[index][offset], 0);
				SDLQLKeyrowChg(joy_char[index][offset + 1], 0);
			}
		}
	}
}

static void QLProcessJoystickButton(Sint32 which, Sint16 button, Sint16 pressed)
{
	int index = QLConvertWhichToIndex(which);

	if (index > -1) {
		// Allow any button to represent fire
		if (pressed)
			queueKey(0, joy_char[index][4], 0);
		SDLQLKeyrowChg(joy_char[index][4], pressed);
	}
}

void QLSDLProcessEvents(void)
{
	SDL_Event event;
	int keypressed;
	int w, h;

	while (1) {
		SDL_WaitEvent(&event);

		switch (event.type) {
		case SDL_KEYDOWN:
			QLSDProcessKey(&event.key.keysym, 1);
			break;
		case SDL_KEYUP:
			QLSDProcessKey(&event.key.keysym, 0);
			break;
#ifndef SDL_JOYSTICK_DISABLED
		case SDL_JOYAXISMOTION:
			QLProcessJoystickAxis(event.jaxis.which,
					      event.jaxis.axis,
					      event.jaxis.value);

			break;
		case SDL_JOYBUTTONDOWN:
			QLProcessJoystickButton(event.jbutton.which,
						event.jbutton.button,
						1);
			break;
		case SDL_JOYBUTTONUP:
			QLProcessJoystickButton(event.jbutton.which,
						event.jbutton.button,
						0);
			break;
#endif
		case SDL_QUIT:
			return;
			break;
		case SDL_MOUSEMOTION:
			QLSDLProcessMouse(event.motion.x, event.motion.y);
			//inside=1;
			break;
		case SDL_MOUSEBUTTONDOWN:
			QLButton(event.button.button, 1);
			break;
		case SDL_MOUSEBUTTONUP:
			QLButton(event.button.button, 0);
			break;
		case SDL_WINDOWEVENT:
			if (event.window.windowID == ql_windowid) {
				switch (event.window.event) {
				case SDL_WINDOWEVENT_ENTER:
					SDL_ShowCursor(SDL_DISABLE);
					break;
				case SDL_WINDOWEVENT_LEAVE:
					SDL_ShowCursor(SDL_ENABLE);
					break;
				case SDL_WINDOWEVENT_RESIZED:
					QLSDLRenderScreen();
					break;
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					QLSDLRenderScreen();
					break;
				}
			}
			break;
		case SDL_USEREVENT:
			switch (event.user.code) {
			case USER_CODE_SCREENREFRESH:
				QLSDLUpdatePixelBuffer();
				QLSDLRenderScreen();
				break;
			case USER_CODE_EMUEXIT:
				return;
			}
			break;
		default:
			break;
		}
	}
}

void QLSDLExit(void)
{
#ifdef SOUND
	closeSound();
#endif

#ifndef SDL_JOYSTICK_DISABLED
	for (int i=0; i<2; ++i) {
		if (joy[i].sdl_id)
			SDL_JoystickClose(joy[i].sdl_id);
	}
#endif

	SDL_RemoveTimer(fiftyhz_timer);
}

Uint32 QLSDL50Hz(Uint32 interval, void *param)
{
	SDL_Event event;
	SDL_AtomicSet(&doPoll, 1);

	schedCount = 0;

	if (sem50Hz && !SDL_SemValue(sem50Hz)) {
		SDL_SemPost(sem50Hz);
	}

	if (screenWritten) {
		screenWritten = false;
		event.user.type = SDL_USEREVENT;
		event.user.code = USER_CODE_SCREENREFRESH;
		event.user.data1 = NULL;
		event.user.data2 = NULL;

		event.type = SDL_USEREVENT;

		SDL_PushEvent(&event);
	}

	return interval;
}
