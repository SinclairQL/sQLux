/*
 * Copyright (c) 2020-2021 Graeme Gregory
 *
 * SPDX: Zlib
 */

#include "GPUshaders.h"		// Needs to be before math.h
#include <inttypes.h>
#include <math.h>
#include <SDL.h>
#include <string.h>

#include "debug.h"
#include "emulator_options.h"
#include "QL_hardware.h"
#include "QL68000.h"
#include "SDL2screen.h"
#include "qlkeys.h"
#include "qlmouse.h"
#include "QL_screen.h"
#include "unixstuff.h"
#include "QL_sound.h"

#define SWAP_SHIFT 0x100
#define SWAP_CNTRL 0x200

static SDL_Window *ql_window = NULL;
static uint32_t ql_windowid = 0;
static SDL_Surface *ql_screen = NULL;
static SDL_Renderer *ql_renderer = NULL;
static SDL_Texture *ql_texture = NULL;
static SDL_Rect dest_rect;
static SDL_TimerID fiftyhz_timer;
static bool renderer_idle = true;
static const char *sdl_video_driver;
static char sdl_win_name[128];
bool ql_fullscreen = false;
double ql_screen_ratio = 1.0;

SDL_atomic_t doPoll;
bool shaders_selected = false;

SDL_sem* sem50Hz = NULL;

typedef enum {
    KEY_US,
    KEY_GB,
    KEY_DE,
    KEY_ES,
} KeyboardType;

typedef enum {
    KEY_ACTION_NONE,
    KEY_ACTION_DIA,
    KEY_ACTION_CIR,
} KeyboardAction;

typedef struct DeadKey {
	int id;					// Dead key id
	bool ignore;			// True if dead key release should not be sent to ROM
	int replace_code;		// Key code sent to ROM to generate accented char
	KeyboardAction action;	// Indicates if dead key active, and if so, the type of accent to be added
} DeadKey;

KeyboardType keyboard = KEY_US;
DeadKey dkey;

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

struct QLcolor QLcolors_unsat[16] = {
	{ 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0xB0 }, { 0xB0, 0x00, 0x00 },
	{ 0xB0, 0x00, 0xB0 }, { 0x00, 0xB0, 0x00 }, { 0x00, 0xB0, 0xB0 },
	{ 0xB0, 0xB0, 0x00 }, { 0xB0, 0xB0, 0xB0 },
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
};

struct SDLQLMap_f {
    int mod;
	SDL_Keycode sdl_kc;
	int code;
};

#define MOD_NONE        (0x0)
#define MOD_ALT         (1 << 0)
#define MOD_CTRL        (1 << 1)
#define MOD_SHIFT       (1 << 2)
#define MOD_WILD        (1 << 3)
#define MOD_GRF         (1 << 4)
#define MOD_CSFT        (MOD_CTRL | MOD_SHIFT)

static struct SDLQLMap_f *sdlqlmap = NULL;
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

static bool QLSDLCreateDisplay(int w , int h, int ly, uint32_t* id,
				const char* name, uint32_t sdl_window_mode);
static void QLSDLUpdateScreen();
static void QLSDLUpdatePixelBuffer();

static void QLSDLInitJoystick(void);
static void QLSDLOpenJoystick(int index, int which);
static void QLProcessJoystickAxis(Sint32 which, Uint8 axis, Sint16 value);
static void QLProcessJoystickButton(Sint32 which, Sint16 button, Sint16 pressed);
static int QLConvertWhichToIndex(Sint32 which);

void QLSDLScreen(void)
{
	SDL_DisplayMode sdl_mode;
	uint32_t sdl_window_mode;
	int i, w, h;
	double ay;
	const char *sysrom = emulatorOptionString("sysrom");
	const char * win_size, *shader_str;

	snprintf(sdl_win_name, 128, "sQLux - %s, %dK", sysrom, RTOP / 1024);

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

	/* Fix the aspect ratio to more like real hardware
	   Note 1.355 is the ratio used in QL Roms (see Minerva disassembly) */
	int aspect = emulatorOptionInt("fixaspect");
	if (aspect == 1) {
		ql_screen_ratio = (3.0 / 2.0);
	} else if (aspect == 2) {
		ql_screen_ratio = 1.355;
	}
	else {
		ql_screen_ratio = 1.0;
	}

	/* Ensure width and height are always initialised to sane values */
	ay = (double)(qlscreen.yres * ql_screen_ratio);
	w = qlscreen.xres;
	h = (int)lrint(ay);

	/* Initialize keyboard table (doesn't belong here, but was convenient...) */
	setKeyboardLayout ();

	if (sdl_video_driver != NULL &&
	    (strcmp(sdl_video_driver, "x11") == 0) ||
	    (strcmp(sdl_video_driver, "cocoa") == 0) ||
	    (strcmp(sdl_video_driver, "windows") == 0) ||
	    (strcmp(sdl_video_driver, "emscripten") == 0) ||
	    (strcmp(sdl_video_driver, "wayland") == 0) &&
	    sdl_mode.w >= 800 &&
	    sdl_mode.h >= 600) {
		sdl_window_mode = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

		win_size = emulatorOptionString("win_size");
		if (!strcmp("2x", win_size)) {
			w = qlscreen.xres * 2;
			h = lrint(ay * 2.0);
		} else if (!strcmp("3x", win_size)) {
			w = qlscreen.xres * 3;
			h = lrint(ay * 3.0);
		} else if (!strcmp("max", win_size)) {
			sdl_window_mode |= SDL_WINDOW_MAXIMIZED;
		} else if (!strcmp("full", win_size)) {
			sdl_window_mode |= SDL_WINDOW_FULLSCREEN_DESKTOP;
			ql_fullscreen = true;
		}
	} else {
		sdl_window_mode = SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	int shader = 0;
#ifdef ENABLE_SHADERS
	shader = emulatorOptionInt("shader");
	if ((shader == 1) || (shader == 2)) {
		shaders_selected = true;
	}
#endif

	bool created = false;
	if (shaders_selected) {
		shader_str = emulatorOptionString("shader_file");
		created = QLGPUCreateDisplay(w , h, (int)lrint(ay),
				&ql_windowid, sdl_win_name, sdl_window_mode,
				shader, shader_str);
	} else {
		created = QLSDLCreateDisplay(w , h, (int)lrint(ay),
				&ql_windowid, sdl_win_name, sdl_window_mode);
	}

	if (!created) {
		printf("Window creation failed\n");
		exit(-1);
	}

	SDL_SetHint(SDL_HINT_GRAB_KEYBOARD, "1");
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

	QLSDLInitJoystick();

	SDL_AtomicSet(&doPoll, 0);
	sem50Hz = SDL_CreateSemaphore(0);
	fiftyhz_timer = SDL_AddTimer(20, QLSDL50Hz, NULL);
}

static bool QLSDLCreateDisplay(int w , int h, int ly, uint32_t* id,
				const char* name, uint32_t sdl_window_mode)
{
	ql_window =
		SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED,
				 SDL_WINDOWPOS_CENTERED, w, h, sdl_window_mode);

	if (ql_window == NULL) {
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
		return false;
	}

	QLSDLCreateIcon(ql_window);
	ql_windowid = SDL_GetWindowID(ql_window);

	ql_renderer = SDL_CreateRenderer(ql_window, -1,
					 SDL_RENDERER_ACCELERATED |
						 SDL_RENDERER_PRESENTVSYNC);

	SDL_RenderSetLogicalSize(ql_renderer, qlscreen.xres, ly);

	dest_rect.x = dest_rect.y = 0;
	dest_rect.w = qlscreen.xres;
	dest_rect.h = ly;

	if (emulatorOptionInt("filter"))
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	ql_screen = SDL_CreateRGBSurfaceWithFormat(
		0, qlscreen.xres, qlscreen.yres, 32, SDL_PIXELFORMAT_RGBA32);

	if (ql_screen == NULL) {
		printf("Error Creating Surface\n");
		return false;
	}

	ql_texture = SDL_CreateTexture(ql_renderer, SDL_PIXELFORMAT_RGBA32,
				       SDL_TEXTUREACCESS_STREAMING,
				       ql_screen->w, ql_screen->h);

	if (ql_texture == NULL) {
		printf("Error Creating texture\n");
		return false;
	}
	QLSDLCreatePalette(ql_screen->format);
	return true;
}

void QLSDLCreateIcon(SDL_Window* window)
{
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
	icon = SDL_CreateRGBSurfaceFrom((void*)sqluxlogo.pixel_data,
			sqluxlogo.width,
			sqluxlogo.height,
			sqluxlogo.bytes_per_pixel * 8,
			sqluxlogo.bytes_per_pixel * sqluxlogo.width,
			rmask, gmask, bmask, amask);

	SDL_SetWindowIcon(window, icon);
	SDL_FreeSurface(icon);
}

void QLSDLCreatePalette(const SDL_PixelFormat* format)
{
	int option = emulatorOptionInt("palette");
	for (int i = 0; i < 16; i++) {
		if (option == 2) {
			SDLcolors[i] = SDL_MapRGB(format, QLcolors_gray[i].r,
						  QLcolors_gray[i].g, QLcolors_gray[i].b);
		} else if (option == 1) {
			SDLcolors[i] = SDL_MapRGB(format, QLcolors_unsat[i].r,
						  QLcolors_unsat[i].g, QLcolors_unsat[i].b);
		} else {
			SDLcolors[i] = SDL_MapRGB(format, QLcolors[i].r,
						  QLcolors[i].g, QLcolors[i].b);
		}
	}
}

void QLSDLWritePixels(uint32_t *pixel_ptr32)
{
	uint8_t *scr_ptr = (void *)memBase + qlscreen.qm_lo;
	int t1, t2, i, color;

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
}

static void QLSDLUpdatePixelBuffer()
{
	if (SDL_MUSTLOCK(ql_screen)) {
		SDL_LockSurface(ql_screen);
	}

	QLSDLWritePixels(ql_screen->pixels);


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

	ql_fullscreen = !ql_fullscreen;

	if (shaders_selected) {
		QLGPUSetFullscreen();
	}
	else {
		SDL_SetWindowFullscreen(ql_window,
					ql_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	}
}

static void QLSDLUpdateScreen()
{
	renderer_idle = false;
	if (shaders_selected) {
		QLGPUUpdateDisplay();
	}
	else {
		QLSDLUpdatePixelBuffer();
		QLSDLRenderScreen();
	}
	renderer_idle = true;
}

static void QLSDLInitJoystick(void)
{
#ifndef SDL_JOYSTICK_DISABLED
	// Open joystick 1 and 2, if defined
	QLSDLOpenJoystick(0, emulatorOptionInt("joy1"));
	QLSDLOpenJoystick(1, emulatorOptionInt("joy2"));
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
int sdl_shiftstate, sdl_controlstate, sdl_altstate, sdl_grfstate;
int usegrfstate = 0;

static void SDLQLKeyrowChg(int code, int press)
{
	code &=0xff; 				// Make sure that array bounds are not exceeded
	int row = 7 - code / 8;
	int col = 0x1 << (code % 8);

	if (press)
		sdl_keyrow[row] |= col;
	else
		sdl_keyrow[row] &= ~col;
}

// Adjust for Windows and X11 generating different scan codes for dead keys
#ifdef __WIN32__
#define SDL_DEADKEY_1 SDLK_BACKQUOTE
#else
#define SDL_DEADKEY_1 SDLK_SLASH
#endif
#define SDL_DEADKEY_2 180

static struct SDLQLMap_f sdlqlmap_DE[] = {
    { MOD_WILD,     SDLK_z,             QL_Y },
    { MOD_WILD,     SDLK_MINUS,         QL_SS },        /* minus */
    { MOD_WILD,     SDLK_y,             QL_Z },
    { MOD_WILD,     0xB4,               QL_POUND },     /* accent */
    { MOD_WILD,     0xE4,               QL_QUOTE },     /* Ä OK */
    { MOD_WILD,     0xF6,               0x67 },         /* Ö OK */
    { MOD_WILD,     0xFC,               QL_LBRACKET },  /* Ü OK */
    { MOD_WILD,     0x5E,               QL_BACKSLASH }, /* < OK */
    { MOD_WILD,     0x23,               0x65 },         /* # OK */
    { MOD_WILD,     0x2B,               QL_RBRACKET },  /* + OK */
    { MOD_WILD,     0xDF,               QL_MINUS },     /* ß OK */
    { MOD_WILD,     0x3C,               0x6D },         /* ^ */
    /* Accent ^ and \ is 0x6D */

    { 0x0, 0x0, 0x0 }
};

static struct SDLQLMap_f sdlqlmap_GB[] = {
    { MOD_NONE,     SDLK_BACKQUOTE,     (SWAP_SHIFT | QL_3) },  // For UK Mac
    { MOD_SHIFT,    SDLK_3,             (SWAP_SHIFT | QL_POUND) },
    { MOD_SHIFT,    SDLK_QUOTE,         QL_2 },
    { MOD_SHIFT,    SDLK_2,             QL_QUOTE },
    { MOD_NONE,     SDLK_HASH,          (SWAP_SHIFT | QL_3) },
    { MOD_SHIFT,    SDLK_HASH,		QL_POUND },
    { 0x0, 0x0, 0x0 }
};

static struct SDLQLMap_f sdlqlmap_ES[] = {
    { MOD_SHIFT,    SDLK_1,             QLSH_PERIOD }, // !
    { MOD_SHIFT,    SDLK_2,             (QL_LBRACKET) }, // "
    { MOD_SHIFT,    SDLK_3,             (SWAP_SHIFT | QL_PERIOD) }, // .
    { MOD_SHIFT,    SDLK_6,             QL_7 }, // &
    { MOD_SHIFT,    SDLK_7,             QL_6 }, // /
    { MOD_SHIFT,    SDLK_8,             QL_9 }, // (
    { MOD_SHIFT,    SDLK_9,             QL_0 }, // )
    { MOD_SHIFT,    SDLK_0,             (SWAP_SHIFT | QL_EQUAL) }, // =
    { MOD_SHIFT,    SDLK_PERIOD,        QL_QUOTE }, // :
    { MOD_SHIFT,    SDLK_COMMA,         (SWAP_SHIFT | QL_QUOTE) }, // ;
    { MOD_NONE,     161,                (SWAP_SHIFT | QL_1) }, // ¡
    { MOD_SHIFT,    161,                QLSH_2 }, // ¿
    { MOD_NONE,     186,                (SWAP_SHIFT | SWAP_CNTRL | QL_Z) }, // º
    { MOD_SHIFT,    186,                (SWAP_CNTRL | QL_Z) }, // º as no ª
    { MOD_NONE,     SDLK_QUOTE,         (SWAP_CNTRL | QL_LBRACKET) }, // '
    { MOD_SHIFT,    SDLK_QUOTE,         QLSH_COMMA }, // ?
    { MOD_NONE,     SDLK_PLUS,          (SWAP_SHIFT | QL_EQUAL) }, // +
    { MOD_SHIFT,    SDLK_PLUS,          QL_8 }, // *
    { MOD_NONE,     SDL_DEADKEY_2,      QL_LBRACKET }, // ´
    { MOD_NONE,     SDL_DEADKEY_1,      QL_RBRACKET }, // `
    { MOD_NONE,     231,                (SWAP_SHIFT | QL_POUND) }, // ç
    { MOD_SHIFT,    231,                (SWAP_CNTRL | QL_H) }, // Ç
    { MOD_WILD,     241,                QL_SEMICOLON }, // ñ Ñ û
    { MOD_WILD,     SDLK_LESS,          QL_SLASH }, // < >
    { MOD_CTRL,     SDLK_PLUS,          (SWAP_SHIFT | QL_RBRACKET) }, // down arrow
    { MOD_CSFT,     SDLK_PLUS,          QL_LBRACKET }, // Up arrow
    { MOD_CTRL,     231,                QL_QUOTE }, // right arrow
    { MOD_CSFT,     231,                QL_QUOTE }, // left arrow
    { MOD_GRF,      SDLK_1,             (SWAP_CNTRL | QL_0) }, // |
    { MOD_GRF,      SDLK_2,             (SWAP_CNTRL | QL_8) }, // @
    { MOD_GRF,      SDLK_3,             (SWAP_SHIFT | QL_3) }, // #
    { MOD_GRF,      SDL_DEADKEY_1,      QL_POUND }, // [
    { MOD_GRF,      SDLK_PLUS,          QL_BACKSLASH }, // ]
    { MOD_GRF,      231,                (SWAP_CNTRL | QL_POUND) }, // }
    { MOD_GRF,      SDL_DEADKEY_2,      (SWAP_CNTRL | QL_EQUAL) }, // {
    { MOD_GRF,      186,                (SWAP_CNTRL | QL_9) }, // backslash
    { MOD_GRF,      SDLK_z,             (SWAP_CNTRL | SWAP_SHIFT | QL_X) }, // «
    { MOD_GRF,      SDLK_x,             (SWAP_CNTRL | SWAP_SHIFT | QL_Y) }, // »
    { 0x0, 0x0, 0x0 }
};

static struct SDLQLMap sdlqlmap_default[] = {
    { SDLK_LEFT,            QL_LEFT },
    { SDLK_UP,              QL_UP },
    { SDLK_RIGHT,           QL_RIGHT },
    { SDLK_DOWN,            QL_DOWN },

    { SDLK_F1,              QL_F1 },
    { SDLK_F2,              QL_F2 },
    { SDLK_F3,              QL_F3 },
    { SDLK_F4,              QL_F4 },
    { SDLK_F5,              QL_F5 },

    { SDLK_RETURN,          QL_ENTER },
    { SDLK_SPACE,           QL_SPACE },
    { SDLK_TAB,             QL_TAB },
    { SDLK_ESCAPE,          QL_ESCAPE },
    { SDLK_CAPSLOCK,        QL_CAPSLOCK },
    { SDLK_RIGHTBRACKET,    QL_RBRACKET },
    { SDLK_LEFTBRACKET,     QL_LBRACKET },
    { SDLK_PERIOD,          QL_PERIOD },
    { SDLK_BACKQUOTE,       QL_POUND },
    { SDLK_QUOTE,           QL_QUOTE },
    { SDLK_BACKSLASH,       QL_BACKSLASH },
    { SDLK_EQUALS,          QL_EQUAL },
    { SDLK_SEMICOLON,       QL_SEMICOLON },
    { SDLK_MINUS,           QL_MINUS },
    { SDLK_SLASH,           QL_SLASH },
    { SDLK_COMMA,           QL_COMMA },

    { SDLK_0,               QL_0 },
    { SDLK_1,               QL_1 },
    { SDLK_2,               QL_2 },
    { SDLK_3,               QL_3 },
    { SDLK_4,               QL_4 },
    { SDLK_5,               QL_5 },
    { SDLK_6,               QL_6 },
    { SDLK_7,               QL_7 },
    { SDLK_8,               QL_8 },
    { SDLK_9,               QL_9 },

    { SDLK_a,               QL_A },
    { SDLK_b,               QL_B },
    { SDLK_c,               QL_C },
    { SDLK_d,               QL_D },
    { SDLK_e,               QL_E },
    { SDLK_f,               QL_F },
    { SDLK_g,               QL_G },
    { SDLK_h,               QL_H },
    { SDLK_i,               QL_I },
    { SDLK_j,               QL_J },
    { SDLK_k,               QL_K },
    { SDLK_l,               QL_L },
    { SDLK_m,               QL_M },
    { SDLK_n,               QL_N },
    { SDLK_o,               QL_O },
    { SDLK_p,               QL_P },
    { SDLK_q,               QL_Q },
    { SDLK_r,               QL_R },
    { SDLK_s,               QL_S },
    { SDLK_t,               QL_T },
    { SDLK_u,               QL_U },
    { SDLK_v,               QL_V },
    { SDLK_w,               QL_W },
    { SDLK_y,               QL_Y },
    { SDLK_x,               QL_X },
    { SDLK_z,               QL_Z },


    /*
    {SDLK_Next,-0x1,0xDC},
    {SDLK_Prior,-0x1,0xD4},
    {SDLK_Home,-0x1,0xC1},
    {SDLK_End,-0x1,0xC9},
    */
    { 0x0, 0x0}
};

void QLSDProcessKey(SDL_Keysym *keysym, int pressed)
{
	int i = 0;
	// printf("Key %8x Scan %8x P: %i\n", keysym->sym, keysym->scancode, pressed); fflush(stdout);

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
	case SDLK_RALT:
		if (usegrfstate)
		{
			sdl_grfstate = pressed;
			return;
		}
		// else drop through
	case SDLK_LALT:
		sdl_altstate = pressed;
		return;
	case SDLK_F11:
		if (pressed)
			SDLQLFullScreen();
		return;
	}

#ifndef __WIN32__
	// Convert X11 dead keys
	if (keysym->sym == 0x40000000)
	{
		keysym->sym = keysym->scancode;
		// Avoid spanish deadkey clash with keycode for 4
		if ((keyboard == KEY_ES) && (keysym->sym == SDLK_4))
		{
			keysym->sym = SDL_DEADKEY_2;
		}
	}
#endif

	// Action Spanish deadkeys not processed in MGE ROM
	if ((keyboard == KEY_ES) && (!sdl_grfstate) && (!sdl_controlstate))
	{
		if (pressed)
		{
			if (((keysym->sym == SDL_DEADKEY_1) && sdl_shiftstate) ||
			    ((keysym->sym == SDL_DEADKEY_2) && sdl_shiftstate))
			{
				dkey.id = keysym->sym;
				dkey.action = (keysym->sym == SDL_DEADKEY_2) ? KEY_ACTION_DIA : KEY_ACTION_CIR;
				dkey.ignore = true;
				return;
			}
		}
		else
		{
			if ((dkey.id == keysym->sym) && dkey.ignore)
			{
				dkey.id = 0;
				dkey.ignore = false;
				return;
			}
		}
	}

	// Is a dead key active?
	if (dkey.action != KEY_ACTION_NONE)
	{
		if (pressed)
		{
			int replace_mod;

			if ((keysym->sym == SDLK_a) ||
			    (keysym->sym == SDLK_e) ||
			    (keysym->sym == SDLK_i) ||
			    (keysym->sym == SDLK_o) ||
			    (keysym->sym == SDLK_u))
			{
				// determine what key combination to send
				if (dkey.action == KEY_ACTION_CIR)
				{
					switch (keysym->sym)
					{
						case SDLK_a:
							replace_mod = 0x02;
							dkey.replace_code = QL_PERIOD;
						break;
						case SDLK_e:
							replace_mod = 0x06;
							dkey.replace_code = QL_L;
						break;
						case SDLK_i:
							replace_mod = 0x06;
							dkey.replace_code = QL_P;
						break;
						case SDLK_o:
							replace_mod = 0x06;
							dkey.replace_code = QL_SEMICOLON;
						break;
						case SDLK_u:
							replace_mod = 0x02;
							dkey.replace_code = QL_SEMICOLON;
						break;
					}
				}
				else if (dkey.action == KEY_ACTION_DIA)
				{
					replace_mod = 0x06;
					switch (keysym->sym)
					{
						case SDLK_a:
							dkey.replace_code = sdl_shiftstate ? QL_SLASH : QL_R;
						break;
						case SDLK_e:
							dkey.replace_code = QL_S;
						break;
						case SDLK_i:
							dkey.replace_code = QL_M;
						break;
						case SDLK_o:
							dkey.replace_code = sdl_shiftstate ? QL_D : QL_4;
						break;
						case SDLK_u:
							if (!sdl_shiftstate)
								replace_mod = 0x04;
							dkey.replace_code = sdl_shiftstate ? QL_G : QL_BACKSLASH;
						break;
					}
				}

				// Need to detect the release of the translated key
				dkey.id = keysym->sym;
				dkey.ignore = false;
			}
			else
			{
				// Need to send base key
				if (dkey.action == KEY_ACTION_CIR) // ^
				{
					dkey.replace_code = QL_RBRACKET;
					replace_mod = 0x04;
				}
				else if (dkey.action == KEY_ACTION_DIA) // "
				{
					dkey.replace_code = QL_LBRACKET;
					replace_mod = 0x04;
				}
			}

			// Clear the deadkey
			dkey.action = KEY_ACTION_NONE;

			// Press the key
			queueKey(replace_mod, dkey.replace_code, 0);
			SDLQLKeyrowChg(dkey.replace_code, pressed);
			return;
		}
	}

	// Check for releasing a translated dead key
	if (!pressed && (dkey.id == keysym->sym) && !dkey.ignore)
	{
		SDLQLKeyrowChg(dkey.replace_code, pressed);
		dkey.replace_code = 0;
		dkey.id = 0;
		return;
	}
	else
	{
		if (sdlqlmap) {
#ifdef __WIN32__
			// Windows always sets the control key when alt Gr is pressed
			int mod = sdl_altstate | ((sdl_controlstate && (!sdl_grfstate)) ? 2 : 0) |
				sdl_shiftstate << 2 | sdl_grfstate << 4;
#else
			int mod = sdl_altstate | (sdl_controlstate << 1) |
				sdl_shiftstate << 2 | sdl_grfstate << 4;
#endif
			while (sdlqlmap[i].sdl_kc != 0) {
				if ((keysym->sym == sdlqlmap[i].sdl_kc) &&
						((sdlqlmap[i].mod == MOD_WILD) || (mod == sdlqlmap[i].mod))) {

					int code = sdlqlmap[i].code;

					/* Code requires a change in shift state? */
					if (SWAP_SHIFT & code) {
						code &= ~SWAP_SHIFT;
						mod ^= (0x1 << 2);
					}
					/* Code requires a change in control state? */
					if (SWAP_CNTRL & code) {
						code &= ~SWAP_CNTRL;
						mod ^= (0x1 << 1);
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
	}
	// Reset the search
	i = 0;

	// Merge back alt Gr and Alt
	int sdl_altcomstate = (sdl_altstate | sdl_grfstate) ? 1 : 0;

	while (sdlqlmap_default[i].sdl_kc != 0) {
		if (keysym->sym == sdlqlmap_default[i].sdl_kc) {
			int mod = sdl_altcomstate | sdl_controlstate << 1 |
				sdl_shiftstate << 2;

			int code = sdlqlmap_default[i].code;

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
	const char *kbd_string = emulatorOptionString("kbd");
	usegrfstate = 0;
	keyboard = KEY_US;

	if (!strncasecmp("DE", kbd_string, 2)) {
		sdlqlmap = sdlqlmap_DE;
		keyboard = KEY_DE;
		if (V1) printf("Using DE keymap.\n");
	} else if (!strncasecmp("GB", kbd_string, 2)) {
		sdlqlmap = sdlqlmap_GB;
		keyboard = KEY_GB;
		if (V1) printf("Using GB keymap.\n");
	} else if (!strncasecmp("ES", kbd_string, 2)) {
		sdlqlmap = sdlqlmap_ES;
		keyboard = KEY_ES;
		usegrfstate = 1;
		if (V1) printf("Using ES keymap.\n");
	} else {
		if (V1) printf("Using default keymap. (use KBD=<countrycode> in sqlux.ini to change)\n");
		sdlqlmap = NULL;
	}
}


static void QLSDLProcessMouse(int* qlx, int* qly, int x, int y)
{
	*qlx = 0;
	*qly = 0;
	float x_ratio, y_ratio;

	if (SDL_GetWindowFlags(ql_window) & SDL_WINDOW_ALLOW_HIGHDPI) {
		x *= 2;
		y *= 2;
	}

	if (x < dest_rect.x) {
		*qlx = 0;
	} else if (x > (dest_rect.w + dest_rect.x)) {
		*qlx = qlscreen.xres - 1;
	} else {
		x_ratio = (float)dest_rect.w / (float)qlscreen.xres;

		x -= dest_rect.x;

		*qlx = ((float)x / x_ratio);
	}

	if (y < dest_rect.y) {
		*qly = 0;
	} else if (y > (dest_rect.h + dest_rect.y)) {
		*qly = qlscreen.yres - 1;
	} else {
		y_ratio = (float)dest_rect.h / (float)qlscreen.yres;

		y -= dest_rect.y;

		*qly = ((float)y / y_ratio);
	}
}

static void QLProcessMouse(int x, int y)
{
	int qlx = 0, qly = 0;

	if (shaders_selected)
		QLGPUProcessMouse(&qlx, &qly, x, y);
	else
		QLSDLProcessMouse(&qlx, &qly, x, y);

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

#if __EMSCRIPTEN__
		if(!SDL_PollEvent(&event)) {
			return;
		}
#else
	while (1) {
		if (!SDL_PollEvent(&event)) {
			continue;
		}
#endif
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
			QLProcessMouse(event.motion.x, event.motion.y);
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
					if (shaders_selected)
						QLGPUSetSize(event.window.data1,
							event.window.data2);
					QLSDLUpdateScreen();
					break;
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					break;
				case SDL_WINDOWEVENT_EXPOSED:
					QLSDLUpdateScreen();
					break;
				}
			}
			break;
		case SDL_USEREVENT:
			switch (event.user.code) {
			case USER_CODE_SCREENREFRESH:
				QLSDLUpdateScreen();
				break;
			case USER_CODE_EMUEXIT:
				return;
			}
			break;
		default:
			break;
		}
#if !__EMSCRIPTEN__
	}
#endif
}

void QLSDLExit(void)
{
	closeSound();

#ifndef SDL_JOYSTICK_DISABLED
	for (int i=0; i<2; ++i) {
		if (joy[i].sdl_id)
			SDL_JoystickClose(joy[i].sdl_id);
	}
#endif

	SDL_RemoveTimer(fiftyhz_timer);
	if (shaders_selected) {
		QLGPUClean();
	}
}

Uint32 QLSDL50Hz(Uint32 interval, void *param)
{
	SDL_Event event;
	SDL_AtomicSet(&doPoll, 1);

	schedCount = 0;

	if (sem50Hz && !SDL_SemValue(sem50Hz)) {
		SDL_SemPost(sem50Hz);
	}

	if (renderer_idle) {
		event.user.type = SDL_USEREVENT;
		event.user.code = USER_CODE_SCREENREFRESH;
		event.user.data1 = NULL;
		event.user.data2 = NULL;

		event.type = SDL_USEREVENT;

		SDL_PushEvent(&event);
	}

	return interval;
}
