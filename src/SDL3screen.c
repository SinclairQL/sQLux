/*
 * Copyright (c) 2020-2021 Graeme Gregory
 *
 * SPDX: Zlib
 */

#include <inttypes.h>
#include <math.h>
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "emulator_logging.h"
#include "emulator_options.h"
#include "QL_hardware.h"
#include "QL68000.h"
#include "SDL3screen.h"
#include "qlkeys.h"
#include "qlmouse.h"
#include "QL_screen.h"
#include "unixstuff.h"
#include "QL_sound.h"

#define SWAP_SHIFT 0x100
#define SWAP_CNTRL 0x200
#define SWAP_ALT 0x400
#define BIT(nr) (1UL << (nr))

static SDL_Window *ql_window = NULL;
static uint32_t ql_windowid = 0;
static SDL_Surface *ql_screen = NULL;
static SDL_Renderer *ql_renderer = NULL;
static SDL_Texture *ql_texture = NULL;
static SDL_FRect dest_rect;
static bool renderer_idle = true;
static const char *sdl_video_driver;
static char sdl_win_name[128];
bool ql_fullscreen = false;
double ql_screen_ratio = 1.0;

extern volatile bool is_display_blank; // Boolean to handle bit 1 of port $18063

SDL_AtomicInt doPoll;

SDL_Semaphore *sem50Hz = NULL;

typedef enum {
	KEY_US,
	KEY_GB,
	KEY_DE,
	KEY_GB_CH,
	KEY_ES,
	KEY_IT,
} KeyboardType;

typedef enum {
	KEY_ACTION_NONE,
	KEY_ACTION_DIA,
	KEY_ACTION_CIR,
} KeyboardAction;

typedef struct DeadKey {
	int id; // Dead key id
	bool ignore; // True if dead key release should not be sent to ROM
	int replace_code; // Key code sent to ROM to generate accented char
	KeyboardAction
		action; // Indicates if dead key active, and if so, the type of accent to be added
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
	{ 0xFF, 0xFF, 0x00 }, { 0xFF, 0xFF, 0xFF }, { 0x3f, 0x3f, 0x3f },
	{ 0x00, 0x00, 0x7f }, { 0x7f, 0x00, 0x00 }, { 0x7f, 0x00, 0x7f },
	{ 0x00, 0x7f, 0x00 }, { 0x00, 0x7f, 0x7f }, { 0x7f, 0x7f, 0x00 },
	{ 0x7f, 0x7f, 0x7f },
};

struct QLcolor QLcolors_unsat[16] = {
	{ 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0xB0 }, { 0xB0, 0x00, 0x00 },
	{ 0xB0, 0x00, 0xB0 }, { 0x00, 0xB0, 0x00 }, { 0x00, 0xB0, 0xB0 },
	{ 0xB0, 0xB0, 0x00 }, { 0xB0, 0xB0, 0xB0 }, { 0x3f, 0x3f, 0x3f },
	{ 0x00, 0x00, 0x7f }, { 0x7f, 0x00, 0x00 }, { 0x7f, 0x00, 0x7f },
	{ 0x00, 0x7f, 0x00 }, { 0x00, 0x7f, 0x7f }, { 0x7f, 0x7f, 0x00 },
	{ 0x7f, 0x7f, 0x7f },
};

struct QLcolor QLcolors_gray[16] = {
	{ 0x00, 0x00, 0x00 }, { 0x12, 0x12, 0x12 }, { 0x36, 0x36, 0x36 },
	{ 0x48, 0x48, 0x48 }, { 0xB6, 0xB6, 0xB6 }, { 0xC8, 0xC8, 0xC8 },
	{ 0xEC, 0xEC, 0x00 }, { 0xFF, 0xFF, 0xFF }, { 0x3f, 0x3f, 0x3f },
	{ 0x09, 0x09, 0x09 }, { 0x1B, 0x1B, 0x1B }, { 0x24, 0x00, 0x24 },
	{ 0x5A, 0x5A, 0x5A }, { 0x63, 0x63, 0x63 }, { 0x75, 0x75, 0x75 },
	{ 0x7f, 0x7f, 0x7f },
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

#define MOD_NONE (0x0)
#define MOD_ALT (1 << 0)
#define MOD_CTRL (1 << 1)
#define MOD_SHIFT (1 << 2)
#define MOD_WILD (1 << 3)
#define MOD_GRF (1 << 4)
#define MOD_CSFT (MOD_CTRL | MOD_SHIFT)

static struct SDLQLMap_f *sdlqlmap = NULL;
static void setKeyboardLayout(void);

/* GIMP RGBA C-Source image dump (sQLuxLogo2.c) */

static const struct {
	unsigned width;
	unsigned height;
	unsigned bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */
	uint8_t pixel_data[32 * 32 * 4 + 1];
} sqluxlogo = {
	32,
	32,
	4,
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

struct aspect {
	int x;
	int y;
};

const struct aspect aspects[] = {
	{ 1024, 512 }, // mode 4 square pixels
	{ 1024, 768 }, // approx aspect for QL with integer scaling
	{ 1024, 694 }, // approx aspect for QL with non integer scaling
	{ 1024, 1024 }, // mode 8 square pixels
};

#ifndef SDL_JOYSTICK_DISABLED
typedef struct {
	SDL_Joystick *sdl_id; // Assigned SDL handle
	int which; // Index into SDL joysticks list
	int left_axis; // SDL axis id for left / right
	int up_axis; // SDL axis id for up / down
} joy_data;

int joy_char[2][5] = { { 49, 52, 50, 55, 54 }, // left, right, up, down, fire
		       { 57, 60, 56, 59, 61 } };

static joy_data joy[2] = { { NULL, -1, 0, 1 }, { NULL, -1, 0, 1 } };
#endif

static bool QLSDLCreateDisplay(int w, int h, int ly, uint32_t *id,
			       const char *name,
			       SDL_WindowFlags sdl_window_mode);
static void QLSDLUpdateScreen();
static void QLSDLUpdatePixelBuffer();

static void QLSDLInitJoystick(void);
static void QLSDLOpenJoystick(int index, int which);
static void QLProcessJoystickAxis(Sint32 which, Uint8 axis, Sint16 value);
static void QLProcessJoystickButton(Sint32 which, Sint16 button,
				    Sint16 pressed);
static int QLConvertWhichToIndex(Sint32 which);

/* =============================================================== */
/*           NEW HIGH PRECISION (50.000 Hz) PULSE                  */
/* 				(Replaces legacy QLSDL50Hz)"                       */
/* =============================================================== */
static int active_metronome = 1;
extern int schedCount;

int Pulse50Thread(void *ptr)
{
	Uint64 frequency = SDL_GetPerformanceFrequency();
	Uint64 ticks_por_frame = frequency / 50; // Exactamente 20ms
	Uint64 next_trigger = SDL_GetPerformanceCounter();

	while (active_metronome) {
		Uint64 Now = SDL_GetPerformanceCounter();

		// FRAME TRIGGER?
		if (Now >= next_trigger) {
			SDL_SetAtomicInt(&doPoll, 1);
			schedCount = 0;

			if (sem50Hz) {
				if (!SDL_GetSemaphoreValue(sem50Hz)) {
					SDL_SignalSemaphore(sem50Hz);
				}
			}

			// Screen Refresh
			if (renderer_idle) {
				SDL_Event event;
				event.user.type = SDL_EVENT_USER;
				event.user.code = USER_CODE_SCREENREFRESH;
				event.user.data1 = NULL;
				event.user.data2 = NULL;
				event.type = SDL_EVENT_USER;
				SDL_PushEvent(&event);
			}

			// PREPARE NEXT FRAME (DRIFT CORRECTION)
			// We add 20ms to the PREVIOUS target time.
			// This automatically corrects 49Hz to a rock-solid 50Hz.
			next_trigger += ticks_por_frame;

			// Watchdog protection (reset if lag exceeds 1 sec)
			if (Now > next_trigger + frequency) {
				next_trigger = Now + ticks_por_frame;
			}
		}

		Now = SDL_GetPerformanceCounter();
		if (Now < next_trigger) {
			Uint64 remaining_ticks = next_trigger - Now;
			double ms_remaining =
				((double)remaining_ticks * 1000.0) / frequency;

			// If more than 1.5ms remain, sleep to save CPU cycles
			if (ms_remaining > 1.5) {
				SDL_Delay((Uint32)(ms_remaining - 1.0));
			} else {
				// If remaining time is very short, use busy-wait for maximum precision
				SDL_Delay(0);
			}
		}
	}
	return 0;
}

int QLSDLScreen(void)
{
	const char *sysrom = emulatorOptionString("sysrom");
	SDL_Rect emulatorDestRect;
	int xRes, yRes;

	SDL_snprintf(sdl_win_name, 128, "sQLux - %s, %dK", sysrom, RTOP / 1024);

	int aspect = emulatorOptionInt("fixaspect");
	if (aspect > 3) {
		SDL_LogError(EMU_LOG_SCREEN, "Invalid FIXASPECT %d", aspect);
		return 1;
	}

	if (qlscreen.xres > 512 || qlscreen.yres > 256) {
		if (aspect != 0) {
			SDL_LogWarn(
				EMU_LOG_SCREEN,
				"extended screen active fixaspect disabled");
		}

		aspect = 0;

		xRes = qlscreen.xres;
		yRes = qlscreen.yres;
	} else {
		xRes = aspects[aspect].x;
		yRes = aspects[aspect].y;
	}

	dest_rect.x = dest_rect.y = 0;
	dest_rect.w = xRes;
	dest_rect.h = yRes;

	ql_window = SDL_CreateWindow(sdl_win_name, xRes, yRes,
				     SDL_WINDOW_RESIZABLE);

	if (ql_window == NULL) {
		SDL_LogError(EMU_LOG_SCREEN, "Failed to Create Window %s\n",
			     SDL_GetError());
		return 1;
	}

	ql_renderer = SDL_CreateRenderer(ql_window, NULL);

	if (ql_renderer == NULL) {
		SDL_LogError(EMU_LOG_SCREEN, "Failed to Create Renderer %s\n",
			     SDL_GetError());
		return 1;
	}

	SDL_SetRenderLogicalPresentation(ql_renderer, xRes, yRes,
					 SDL_LOGICAL_PRESENTATION_LETTERBOX);

	SDL_RenderClear(ql_renderer);
	SDL_RenderPresent(ql_renderer);

	ql_screen = SDL_CreateSurface(qlscreen.xres, qlscreen.yres,
				      SDL_PIXELFORMAT_RGBA32);

	if (ql_screen == NULL) {
		SDL_LogError(EMU_LOG_SCREEN, "Failed to Create Screen %s\n",
			     SDL_GetError());
		return 1;
	}

	ql_texture = SDL_CreateTexture(ql_renderer, SDL_PIXELFORMAT_RGBA32,
				       SDL_TEXTUREACCESS_STREAMING,
				       qlscreen.xres, qlscreen.yres);

	if (ql_texture == NULL) {
		SDL_LogError(EMU_LOG_SCREEN, "Failed to Create Texture %s\n",
			     SDL_GetError());
		return 1;
	}

	QLSDLCreatePalette(ql_screen->format);
	QLSDLCreateIcon(ql_window);
	SDL_CreateThread(Pulse50Thread, "MetronomoQL", NULL);

	return 0;
}

void QLSDLCreateIcon(SDL_Window *window)
{
	SDL_Surface *icon;

	/* The logo is stored as 32x32 RGBA bytes in memory order */
	icon = SDL_CreateSurfaceFrom(
		sqluxlogo.width, sqluxlogo.height, SDL_PIXELFORMAT_RGBA32,
		(void *)sqluxlogo.pixel_data,
		sqluxlogo.bytes_per_pixel * sqluxlogo.width);

	SDL_SetWindowIcon(window, icon);
	SDL_DestroySurface(icon);
}

void QLSDLCreatePalette(SDL_PixelFormat format)
{
	const SDL_PixelFormatDetails *details =
		SDL_GetPixelFormatDetails(format);
	int option = emulatorOptionInt("palette");
	for (int i = 0; i < 16; i++) {
		if (option == 2) {
			SDLcolors[i] =
				SDL_MapRGB(details, NULL, QLcolors_gray[i].r,
					   QLcolors_gray[i].g,
					   QLcolors_gray[i].b);
		} else if (option == 1) {
			SDLcolors[i] =
				SDL_MapRGB(details, NULL, QLcolors_unsat[i].r,
					   QLcolors_unsat[i].g,
					   QLcolors_unsat[i].b);
		} else {
			SDLcolors[i] = SDL_MapRGB(details, NULL, QLcolors[i].r,
						  QLcolors[i].g, QLcolors[i].b);
		}
	}
}

// frame counter for flash
static int curframe = 0;

static void emulatorUpdatePixelBufferQL(uint32_t *pixelPtr32,
					uint8_t *emulatorScreenPtr,
					uint8_t *emulatorScreenPtrEnd)
{
	int curpix = 0;
	uint32_t flashbg = 0;
	int flashon = 0;

	while (emulatorScreenPtr < emulatorScreenPtrEnd) {
		uint8_t t1 = *emulatorScreenPtr++;
		uint8_t t2 = *emulatorScreenPtr++;

		switch (display_mode) {
		case 8:
			for (int i = 6; i > -2; i -= 2) {
				uint8_t p1 = (t1 >> i) & 0x03;
				uint8_t p2 = (t2 >> i) & 0x03;

				int color = ((p1 & 2) << 1) + ((p2 & 3));
				int flashbit = (p1 & 1);

				uint32_t x = SDLcolors[color];

				if ((curframe & BIT(5)) && flashon) {
					x = flashbg;
				}

				*pixelPtr32++ = x;
				*pixelPtr32++ = x;

				// flash happens after the pixel
				if (flashbit) {
					if (flashon == 0) {
						flashbg = x;
						flashon = 1;
					} else {
						flashon = 0;
					}
				}

				// Handle flash end of line
				// stride is fixed at 256 because mode 8
				// is fixed at that size on QL and Q68
				curpix++;
				curpix %= 256;
				if (curpix == 0) {
					flashbg = 0;
					flashon = 0;
				}
			}
			break;
		case 1:
		case 4:
			for (int i = 7; i > -1; i--) {
				uint8_t p1 = (t1 >> i) & 0x01;
				uint8_t p2 = (t2 >> i) & 0x01;

				int color = ((p1 & 1) << 2) + ((p2 & 1) << 1) +
					    ((p1 & 1) & (p2 & 1));

				uint32_t x = SDLcolors[color];

				*pixelPtr32++ = x;
			}
			break;
		}
	}

	// frame counter for flash
	curframe++;
	curframe %= 64;
}

static void QLSDLUpdatePixelBuffer()
{
	if (SDL_MUSTLOCK(ql_screen)) {
		SDL_LockSurface(ql_screen);
	}

	uint8_t *emulatorScreenPtr = (uint8_t *)memBase + qlscreen.qm_lo;
	uint8_t *emulatorScreenPtrEnd = emulatorScreenPtr + qlscreen.qm_len;

	emulatorUpdatePixelBufferQL(ql_screen->pixels, emulatorScreenPtr,
				    emulatorScreenPtrEnd);

	if (SDL_MUSTLOCK(ql_screen)) {
		SDL_UnlockSurface(ql_screen);
	}
}

void QLSDLRenderScreen(void)
{
	SDL_UpdateTexture(ql_texture, NULL, ql_screen->pixels,
			  ql_screen->pitch);
	SDL_RenderClear(ql_renderer);
	if (!is_display_blank) {
		SDL_RenderTexture(ql_renderer, ql_texture, NULL, &dest_rect);
	}

	SDL_RenderPresent(ql_renderer);
}

void SDLQLFullScreen(void)
{
	ql_fullscreen = !ql_fullscreen;

	SDL_SetWindowFullscreen(ql_window,
				ql_fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
}

static void QLSDLUpdateScreen()
{
	renderer_idle = false;
	QLSDLUpdatePixelBuffer();
	QLSDLRenderScreen();
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
		int joysticks = 0;
		SDL_JoystickID *ids = SDL_GetJoysticks(&joysticks);

		if (joysticks && ids) {
			if (joysticks > which0) {
				SDL_JoystickID id = ids[which0];

				// Check for duplicate ids
				if (QLConvertWhichToIndex(id) == -1) {
					joy[index].sdl_id =
						SDL_OpenJoystick(id);
					if (joy[index].sdl_id == NULL) {
						if (V1)
							printf("Joystick %i initialisation failed\n",
							       index + 1);
					} else {
						joy[index].which = id;
						if (V1)
							printf("Joystick %i initialised\n",
							       index + 1);
					}
				} else {
					if (V1)
						printf("Joystick %i initialisation failed. Duplicate index %i\n",
						       index + 1, which);
				}
			} else {
				if (V1)
					printf("Joystick %i initialisation failed. Index %i too high\n",
					       index + 1, which);
			}
		} else {
			if (V1)
				printf("No joysticks detected by SDL\n");
		}
		SDL_free(ids);
	} else {
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
	code &= 0xff; // Make sure that array bounds are not exceeded
	int row = 7 - code / 8;
	int col = 0x1 << (code % 8);

	if (press)
		sdl_keyrow[row] |= col;
	else
		sdl_keyrow[row] &= ~col;
}

// Adjust for Windows and X11 generating different scan codes for dead keys
#ifdef __WIN32__
#define SDL_DEADKEY_1 SDLK_GRAVE
#else
#define SDL_DEADKEY_1 SDLK_SLASH
#endif
#define SDL_DEADKEY_2 180

// Note this is the Windows keymap. Modified from MacOS with no test
static struct SDLQLMap_f sdlqlmap_DE[] = {
	// These should be valid for all platforms
	{ MOD_WILD, SDLK_Z, QL_Y }, // Y	OK
	{ MOD_WILD, SDLK_Y, QL_Z }, // Z	OK
	{ MOD_WILD, SDLK_MINUS, QL_SS }, // ß?	OK
	{ MOD_WILD, 0xE4, QL_QUOTE }, // Ää 	OK
	{ MOD_WILD, 0xF6, QL_SEMICOLON }, // Öö	OK
	{ MOD_WILD, 0xFC, QL_LBRACKET }, // Üü 	OK
	{ MOD_WILD, 0x3c, QL_BACKSLASH }, // <>	OK
	{ MOD_WILD, 0x23, QL_EQUAL }, // #' 	OK
	{ MOD_SHIFT, 0xb4, (SWAP_CNTRL | QL_SLASH) }, // ` 	OK
	{ MOD_NONE, 0xb4, (SWAP_SHIFT | QL_EQUAL) }, // ' on acc key OK
	{ MOD_NONE, 0x5e, (SWAP_SHIFT | QL_BACKSLASH) }, // ^ 	NOK
	{ MOD_SHIFT, 0x5e, (SWAP_CNTRL | QL_Y) }, // °	OK
	{ MOD_WILD, 0x2B, QL_RBRACKET }, // + OK */
	{ MOD_WILD, 0xDF, QL_MINUS }, // ß OK */
	{ MOD_NONE, SDLK_KP_DIVIDE, (SWAP_SHIFT | QL_7) },
	{ MOD_NONE, SDLK_KP_MINUS, QL_SLASH },
	{ MOD_NONE, SDLK_KP_PLUS, QL_RBRACKET },
	{ MOD_NONE, SDLK_KP_MULTIPLY, (SWAP_SHIFT | QL_RBRACKET) },

	// The following are Windows-specific (Alt-GR-based)
	{ MOD_ALT, SDLK_LESS, (SWAP_ALT | SWAP_CNTRL | QL_8) }, // |	OK
	{ MOD_ALT, SDLK_8, (SWAP_ALT | SWAP_CNTRL | QL_9) }, // [	OK
	{ MOD_ALT, SDLK_9, (SWAP_ALT | SWAP_CNTRL | QL_0) }, // ]	OK
	{ MOD_ALT, SDLK_7, (SWAP_ALT | SWAP_CNTRL | QL_MINUS) }, // {	OK
	{ MOD_ALT, SDLK_0, (SWAP_ALT | SWAP_CNTRL | QL_EQUAL) }, // }	OK
	{ MOD_ALT, SDLK_PLUS, (SWAP_CNTRL | QL_COMMA) }, // ~	OK
	{ MOD_ALT, SDLK_2, (SWAP_CNTRL | QL_BACKSLASH) }, // @	OK

	{ 0x0, 0x0, 0x0 }
};

// This is the MacOS keymap. Mostly identical with the Windows one
static struct SDLQLMap_f sdlqlmap_DE_MacOS[] = {
	// These should be valid for all platforms
	{ MOD_WILD, SDLK_Z, QL_Y }, // Y	OK
	{ MOD_WILD, SDLK_Y, QL_Z }, // Z	OK
	{ MOD_WILD, SDLK_MINUS, QL_SS }, // ß?	OK
	{ MOD_WILD, 0xE4, QL_QUOTE }, // Ää 	OK
	{ MOD_WILD, 0xF6, QL_SEMICOLON }, // Öö	OK
	{ MOD_WILD, 0xFC, QL_LBRACKET }, // Üü 	OK
	{ MOD_WILD, 0x3c, QL_BACKSLASH }, // <>	OK
	{ MOD_WILD, 0x23, QL_EQUAL }, // #' 	OK
	{ MOD_SHIFT, 0xb4, (SWAP_CNTRL | QL_SLASH) }, // ` 	OK
	{ MOD_NONE, 0xb4, (SWAP_SHIFT | QL_EQUAL) }, // ' on acc key OK
	{ MOD_NONE, 0x5e, (SWAP_SHIFT | QL_BACKSLASH) }, // ^ 	NOK
	{ MOD_SHIFT, 0x5e, (SWAP_CNTRL | QL_Y) }, // °	OK
	{ MOD_WILD, 0x2B, QL_RBRACKET }, // + OK */
	{ MOD_WILD, 0xDF, QL_MINUS }, // ß OK */
	{ MOD_NONE, SDLK_KP_DIVIDE, (SWAP_SHIFT | QL_7) },
	{ MOD_NONE, SDLK_KP_MINUS, QL_SLASH },
	{ MOD_NONE, SDLK_KP_PLUS, QL_RBRACKET },
	{ MOD_NONE, SDLK_KP_MULTIPLY, (SWAP_SHIFT | QL_RBRACKET) },

	// The following are MacOS-specific ("Option"-based)
	{ MOD_ALT, SDLK_7, (SWAP_ALT | SWAP_CNTRL | QL_8) }, // |	OK
	{ MOD_ALT, SDLK_5, (SWAP_ALT | SWAP_CNTRL | QL_9) }, // [	OK
	{ MOD_ALT, SDLK_6, (SWAP_ALT | SWAP_CNTRL | QL_0) }, // ]	OK
	{ MOD_ALT, SDLK_8, (SWAP_ALT | SWAP_CNTRL | QL_MINUS) }, // {	OK
	{ MOD_ALT, SDLK_9, (SWAP_ALT | SWAP_CNTRL | QL_EQUAL) }, // }	OK
	{ MOD_ALT, SDLK_N,
	  (SWAP_ALT | SWAP_CNTRL | QL_BACKSLASH) }, // ~	OK
	{ MOD_ALT, SDLK_L, (SWAP_ALT | SWAP_CNTRL | QL_COMMA) }, // @	NOK

	{ 0x0, 0x0, 0x0 }
};

static struct SDLQLMap_f sdlqlmap_GB_ch[] = {
	{ MOD_NONE, 167, (SWAP_SHIFT | SWAP_CNTRL | QL_V) }, // §
	{ MOD_SHIFT, 167, (SWAP_CNTRL | QL_Z) }, // °
	{ MOD_SHIFT, SDLK_1, QL_EQUAL }, // +
	{ MOD_CTRL, SDLK_1, (SWAP_SHIFT | SWAP_CNTRL | QL_BACKSLASH) }, // |
	{ MOD_SHIFT, SDLK_2, QL_QUOTE }, // "
	{ MOD_CTRL, SDLK_2, (SWAP_SHIFT | SWAP_CNTRL | QL_2) }, // @
	{ MOD_SHIFT, SDLK_3, QLSH_8 }, // *
	{ MOD_CTRL, SDLK_3, (SWAP_SHIFT | SWAP_CNTRL | QL_3) }, // #
	{ MOD_SHIFT, SDLK_4, (SWAP_CNTRL | QLSH_9) }, // ç
	{ MOD_SHIFT, SDLK_6, QL_7 }, // &
	{ MOD_SHIFT, SDLK_7, (SWAP_SHIFT | QL_SLASH) }, // slash
	{ MOD_CTRL, SDLK_7, (SWAP_SHIFT | SWAP_CNTRL | QL_BACKSLASH) }, // |
	{ MOD_SHIFT, SDLK_8, QL_9 }, // (
	{ MOD_SHIFT, SDLK_9, QL_0 }, // )
	{ MOD_SHIFT, SDLK_0, (SWAP_SHIFT | QL_EQUAL) }, // =
	{ MOD_NONE, 39, QL_QUOTE }, // '
	{ MOD_SHIFT, 39, QLSH_SLASH }, // ?
	{ MOD_NONE, 94, (SWAP_SHIFT | QL_6) }, // ^
	{ MOD_SHIFT, 94, (SWAP_CNTRL | QLSH_SLASH) }, // `
	{ MOD_WILD, 94, (SWAP_SHIFT | SWAP_CNTRL | QL_SHIFT | QL_POUND) }, // ~
	{ MOD_NONE, 252, (SWAP_CNTRL | QL_QUOTE) }, // ü
	{ MOD_SHIFT, 252, (SWAP_CNTRL | QL_SHIFT | QL_G) }, // Ü
	{ MOD_CTRL, 252, (QL_CTRL | QL_LBRACKET) }, // [
	{ MOD_NONE, 168, (SWAP_SHIFT | QL_1) }, // !
	{ MOD_SHIFT, 168, QLSH_QUOTE }, // ¨
	{ MOD_CTRL, 168, (QL_CTRL | QL_RBRACKET) }, // ]
	{ MOD_NONE, 246, (SWAP_CNTRL | SWAP_SHIFT | QL_4) }, // ö
	{ MOD_SHIFT, 246, (SWAP_CNTRL | QL_SHIFT | QL_D) }, // Ö
	{ MOD_NONE, 228, (SWAP_CNTRL | QL_ESCAPE) }, // ä
	{ MOD_SHIFT, 228, (SWAP_CNTRL | QL_SHIFT) | QL_2 }, // Ä
	{ MOD_CTRL, 228, (SWAP_SHIFT | QL_CTRL | QL_LBRACKET) }, // {
	{ MOD_NONE, 36, (SWAP_SHIFT | QL_4) }, // $
	{ MOD_SHIFT, 36, (SWAP_SHIFT | QL_POUND) }, // £
	{ MOD_CTRL, 36, (SWAP_SHIFT | QL_CTRL | QL_RBRACKET) }, // }
	{ MOD_NONE, 60, (SWAP_SHIFT | QL_PERIOD) }, // <
	{ MOD_SHIFT, 60, QL_COMMA }, // >
	{ MOD_CTRL, 60, (QL_CTRL | QL_BACKSLASH) }, // bash
	{ MOD_SHIFT, SDLK_PERIOD, QL_SEMICOLON }, // ;
	{ MOD_SHIFT, SDLK_COMMA, (SWAP_SHIFT | QL_SEMICOLON) }, // :
	{ MOD_NONE, SDLK_MINUS, QL_MINUS }, // -
	{ 0x0, 0x0, 0x0 }
};

static struct SDLQLMap_f sdlqlmap_GB[] = {
	{ MOD_NONE, SDLK_GRAVE, (SWAP_SHIFT | QL_3) }, // For UK Mac
	{ MOD_SHIFT, SDLK_3, (SWAP_SHIFT | QL_POUND) },
	{ MOD_SHIFT, SDLK_APOSTROPHE, QL_2 },
	{ MOD_SHIFT, SDLK_2, QL_QUOTE },
	{ MOD_NONE, SDLK_HASH, (SWAP_SHIFT | QL_3) },
	{ MOD_SHIFT, SDLK_HASH, QL_POUND },
	{ MOD_NONE, SDLK_KP_PLUS, (SWAP_SHIFT | QL_EQUAL) },
	{ MOD_NONE, SDLK_KP_MULTIPLY, (SWAP_SHIFT | QL_8) },
	{ 0x0, 0x0, 0x0 }
};

static struct SDLQLMap_f sdlqlmap_ES[] = {
	{ MOD_SHIFT, SDLK_1, QLSH_PERIOD }, // !
	{ MOD_SHIFT, SDLK_2, (QL_LBRACKET) }, // "
	{ MOD_SHIFT, SDLK_3, (SWAP_SHIFT | QL_PERIOD) }, // .
	{ MOD_SHIFT, SDLK_6, QL_7 }, // &
	{ MOD_SHIFT, SDLK_7, QL_6 }, // /
	{ MOD_SHIFT, SDLK_8, QL_9 }, // (
	{ MOD_SHIFT, SDLK_9, QL_0 }, // )
	{ MOD_SHIFT, SDLK_0, (SWAP_SHIFT | QL_EQUAL) }, // =
	{ MOD_SHIFT, SDLK_PERIOD, QL_QUOTE }, // :
	{ MOD_SHIFT, SDLK_COMMA, (SWAP_SHIFT | QL_QUOTE) }, // ;
	{ MOD_NONE, 161, (SWAP_SHIFT | QL_1) }, // ¡
	{ MOD_SHIFT, 161, QLSH_2 }, // ¿
	{ MOD_NONE, 186, (SWAP_SHIFT | SWAP_CNTRL | QL_Z) }, // º
	{ MOD_SHIFT, 186, (SWAP_CNTRL | QL_Z) }, // º as no ª
	{ MOD_NONE, SDLK_APOSTROPHE, (SWAP_CNTRL | QL_LBRACKET) }, // '
	{ MOD_SHIFT, SDLK_APOSTROPHE, QLSH_COMMA }, // ?
	{ MOD_NONE, SDLK_PLUS, (SWAP_SHIFT | QL_EQUAL) }, // +
	{ MOD_SHIFT, SDLK_PLUS, QL_8 }, // *
	{ MOD_NONE, SDL_DEADKEY_2, QL_LBRACKET }, // ´
	{ MOD_NONE, SDL_DEADKEY_1, QL_RBRACKET }, // `
	{ MOD_NONE, 231, (SWAP_SHIFT | QL_POUND) }, // ç
	{ MOD_SHIFT, 231, (SWAP_CNTRL | QL_H) }, // Ç
	{ MOD_WILD, 241, QL_SEMICOLON }, // ñ Ñ û
	{ MOD_WILD, SDLK_LESS, QL_SLASH }, // < >
	{ MOD_CTRL, SDLK_PLUS, (SWAP_SHIFT | QL_RBRACKET) }, // down arrow
	{ MOD_CSFT, SDLK_PLUS, QL_LBRACKET }, // Up arrow
	{ MOD_CTRL, 231, QL_QUOTE }, // right arrow
	{ MOD_CSFT, 231, QL_QUOTE }, // left arrow
	{ MOD_GRF, SDLK_1, (SWAP_CNTRL | QL_0) }, // |
	{ MOD_GRF, SDLK_2, (SWAP_CNTRL | QL_8) }, // @
	{ MOD_GRF, SDLK_3, (SWAP_SHIFT | QL_3) }, // #
	{ MOD_GRF, SDL_DEADKEY_1, QL_POUND }, // [
	{ MOD_GRF, SDLK_PLUS, QL_BACKSLASH }, // ]
	{ MOD_GRF, 231, (SWAP_CNTRL | QL_POUND) }, // }
	{ MOD_GRF, SDL_DEADKEY_2, (SWAP_CNTRL | QL_EQUAL) }, // {
	{ MOD_GRF, 186, (SWAP_CNTRL | QL_9) }, // backslash
	{ MOD_GRF, SDLK_Z, (SWAP_CNTRL | SWAP_SHIFT | QL_X) }, // «
	{ MOD_GRF, SDLK_X, (SWAP_CNTRL | SWAP_SHIFT | QL_Y) }, // »
	{ MOD_NONE, SDLK_KP_DIVIDE, (SWAP_SHIFT | QL_6) },
	{ MOD_NONE, SDLK_KP_PLUS, (SWAP_SHIFT | QL_EQUAL) },
	{ MOD_NONE, SDLK_KP_MULTIPLY, (SWAP_SHIFT | QL_8) },
	{ 0x0, 0x0, 0x0 }
};

static struct SDLQLMap_f sdlqlmap_IT[] = {
	{ MOD_SHIFT, SDLK_0, (SWAP_SHIFT | QL_EQUAL) },
	{ MOD_SHIFT, SDLK_9, (QLSH_0) },
	{ MOD_SHIFT, SDLK_8, (QLSH_9) },
	{ MOD_SHIFT, SDLK_7, (SWAP_SHIFT | QL_SLASH) },
	{ MOD_SHIFT, SDLK_6, (QLSH_7) },
	{ MOD_SHIFT, SDLK_3, (SWAP_SHIFT | QL_POUND) },
	{ MOD_SHIFT, SDLK_2, QL_QUOTE },
	{ MOD_SHIFT, SDLK_APOSTROPHE, QL_SLASH },
	{ MOD_SHIFT, 0xec, (QLSH_6) },
	{ MOD_GRF, 0x2b, QL_RBRACKET }, // ]
	{ MOD_GRF, 0xe8, QL_LBRACKET }, // [
	{ MOD_SHIFT | MOD_GRF, 0x2b, (QLSH_RBRACKET) }, // }
	{ MOD_NONE, 0x2b, (SWAP_SHIFT | QL_EQUAL) }, // +
	{ MOD_SHIFT, 0x2b, (QL_8) }, // *
	{ MOD_SHIFT | MOD_GRF, 0xe8, (QLSH_LBRACKET) }, // {
	{ MOD_NONE, 0xec, (SWAP_CNTRL | QL_CTRL | QL_4) }, // ì
	{ MOD_NONE, 0x00e0, (SWAP_CNTRL | QL_MINUS) }, // à
	{ MOD_NONE, 0x00f2, (SWAP_CNTRL | QL_7) }, // ò
	{ MOD_GRF, 0x00f2, (SWAP_SHIFT | QL_2) }, // @
	{ MOD_SHIFT, 0x00e0, (SWAP_CNTRL | QL_Z) }, // °
	{ MOD_GRF, 0x00e0, (SWAP_SHIFT | QL_3) }, // #
	{ MOD_NONE, 0xe8, (SWAP_CNTRL | QL_0) }, // è
	{ MOD_SHIFT, 0xe8, (SWAP_CNTRL | QL_3) }, // é
	{ MOD_SHIFT, SDLK_PERIOD, QL_SEMICOLON }, // :
	{ MOD_SHIFT, SDLK_COMMA, SWAP_SHIFT | QL_SEMICOLON }, // ;
	{ MOD_NONE, 0x3c, SWAP_SHIFT | QL_COMMA }, // <
	{ MOD_SHIFT, 0x3c, QL_PERIOD }, // >
	{ MOD_SHIFT, 249, (SWAP_CNTRL | QL_V) }, // §
	{ MOD_NONE, 249,
	  (SWAP_CNTRL |
	   QL_9) }, // wrong accented 'u' - couldn't find how to reproduce it from the international QL keymap
	{ 0x0, 0x0, 0x0 }
};

static struct SDLQLMap sdlqlmap_default[] = { { SDLK_LEFT, QL_LEFT },
					      { SDLK_UP, QL_UP },
					      { SDLK_RIGHT, QL_RIGHT },
					      { SDLK_DOWN, QL_DOWN },

					      { SDLK_F1, QL_F1 },
					      { SDLK_F2, QL_F2 },
					      { SDLK_F3, QL_F3 },
					      { SDLK_F4, QL_F4 },
					      { SDLK_F5, QL_F5 },

					      { SDLK_RETURN, QL_ENTER },
					      { SDLK_SPACE, QL_SPACE },
					      { SDLK_TAB, QL_TAB },
					      { SDLK_ESCAPE, QL_ESCAPE },
					      { SDLK_CAPSLOCK, QL_CAPSLOCK },
					      { SDLK_RIGHTBRACKET,
						QL_RBRACKET },
					      { SDLK_LEFTBRACKET, QL_LBRACKET },
					      { SDLK_PERIOD, QL_PERIOD },
					      { SDLK_GRAVE, QL_POUND },
					      { SDLK_APOSTROPHE, QL_QUOTE },
					      { SDLK_BACKSLASH, QL_BACKSLASH },
					      { SDLK_EQUALS, QL_EQUAL },
					      { SDLK_SEMICOLON, QL_SEMICOLON },
					      { SDLK_MINUS, QL_MINUS },
					      { SDLK_SLASH, QL_SLASH },
					      { SDLK_COMMA, QL_COMMA },

					      { SDLK_0, QL_0 },
					      { SDLK_1, QL_1 },
					      { SDLK_2, QL_2 },
					      { SDLK_3, QL_3 },
					      { SDLK_4, QL_4 },
					      { SDLK_5, QL_5 },
					      { SDLK_6, QL_6 },
					      { SDLK_7, QL_7 },
					      { SDLK_8, QL_8 },
					      { SDLK_9, QL_9 },

					      { SDLK_A, QL_A },
					      { SDLK_B, QL_B },
					      { SDLK_C, QL_C },
					      { SDLK_D, QL_D },
					      { SDLK_E, QL_E },
					      { SDLK_F, QL_F },
					      { SDLK_G, QL_G },
					      { SDLK_H, QL_H },
					      { SDLK_I, QL_I },
					      { SDLK_J, QL_J },
					      { SDLK_K, QL_K },
					      { SDLK_L, QL_L },
					      { SDLK_M, QL_M },
					      { SDLK_N, QL_N },
					      { SDLK_O, QL_O },
					      { SDLK_P, QL_P },
					      { SDLK_Q, QL_Q },
					      { SDLK_R, QL_R },
					      { SDLK_S, QL_S },
					      { SDLK_T, QL_T },
					      { SDLK_U, QL_U },
					      { SDLK_V, QL_V },
					      { SDLK_W, QL_W },
					      { SDLK_Y, QL_Y },
					      { SDLK_X, QL_X },
					      { SDLK_Z, QL_Z },
					      /* Map keypad */
					      { SDLK_KP_DIVIDE, QL_SLASH },
					      { SDLK_KP_MINUS, QL_MINUS },
					      { SDLK_KP_ENTER, QL_ENTER },
					      { SDLK_KP_1, QL_1 },
					      { SDLK_KP_2, QL_2 },
					      { SDLK_KP_3, QL_3 },
					      { SDLK_KP_4, QL_4 },
					      { SDLK_KP_5, QL_5 },
					      { SDLK_KP_6, QL_6 },
					      { SDLK_KP_7, QL_7 },
					      { SDLK_KP_8, QL_8 },
					      { SDLK_KP_9, QL_9 },
					      { SDLK_KP_0, QL_0 },
					      { SDLK_KP_PERIOD, QL_PERIOD },
					      { 0x0, 0x0 } };

void QLSDProcessKey(SDL_Keycode sym, SDL_Scancode scancode, int pressed)
{
	int i = 0;
	//printf("Key %8x Scan %8x P: %i SH: %d ALT: %d CTRL: %d GRF: %d\n",
	//	sym, scancode, pressed,
	// 	sdl_shiftstate, sdl_altstate, sdl_controlstate,
	//	sdl_grfstate); fflush(stdout);

	/* Handle key pad entries that require shift - with the US keyboard */
	if ((sym == SDLK_KP_MULTIPLY) && pressed && !sdlqlmap) {
		queueKey(1 << 2, QL_8, 0);
		return;
	}
	if ((sym == SDLK_KP_PLUS) && pressed && !sdlqlmap) {
		queueKey(1 << 2, QL_EQUAL, 0);
		return;
	}

	/* Convert keypad entries that depend on num lock not being set */
	if (((SDL_GetModState() & SDL_KMOD_NUM) != (SDL_KMOD_NUM)) && pressed) {
		switch (sym) {
		case SDLK_KP_1:
			sym = SDLK_END;
			break;
		case SDLK_KP_2:
			sym = SDLK_DOWN;
			break;
		case SDLK_KP_3:
			sym = SDLK_PAGEDOWN;
			break;
		case SDLK_KP_4:
			sym = SDLK_LEFT;
			break;
		case SDLK_KP_5:
			return;
		case SDLK_KP_6:
			sym = SDLK_RIGHT;
			break;
		case SDLK_KP_7:
			sym = SDLK_HOME;
			break;
		case SDLK_KP_8:
			sym = SDLK_UP;
			break;
		case SDLK_KP_9:
			sym = SDLK_PAGEUP;
			break;
		case SDLK_KP_0:
			sym = SDLK_INSERT;
			break;
		case SDLK_KP_PERIOD:
			sym = SDLK_DELETE;
			break;
		}
	}

	/* Handle extended cursor keys */
	/* backspace maps to control left */
	if ((sym == SDLK_BACKSPACE) && pressed) {
		queueKey(1 << 1, 49, 0);
		return;
	}
	/* Delete maps to control right */
	if ((sym == SDLK_DELETE) && pressed) {
		queueKey(1 << 1, 52, 0);
		return;
	}
	/* Home maps to alt left */
	if ((sym == SDLK_HOME) && pressed) {
		queueKey(1 << 0, 49, 0);
		return;
	}
	/* End maps to alt right */
	if ((sym == SDLK_END) && pressed) {
		queueKey(1 << 0, 52, 0);
		return;
	}
	/* Insert maps to shift F4 */
	if ((sym == SDLK_INSERT) && pressed) {
		queueKey(1 << 2, 56, 0);
		return;
	}
	/* Page Up maps to shift down */
	if ((sym == SDLK_PAGEUP) && pressed) {
		queueKey(1 << 2, 50, 0);
		return;
	}
	/* Page Down maps to shift down */
	if ((sym == SDLK_PAGEDOWN) && pressed) {
		queueKey(1 << 2, 55, 0);
		return;
	}

	switch (sym) {
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		sdl_shiftstate = pressed;
		return;
	case SDLK_LCTRL:
	case SDLK_RCTRL:
		sdl_controlstate = pressed;
		return;
	case SDLK_RALT:
		if (usegrfstate) {
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
	if (sym == 0x40000000) {
		sym = scancode;
		// Avoid spanish deadkey clash with keycode for 4
		if ((keyboard == KEY_ES) && (sym == SDLK_4)) {
			sym = SDL_DEADKEY_2;
		}
	}
#endif

	// Action Spanish deadkeys not processed in MGE ROM
	if ((keyboard == KEY_ES) && (!sdl_grfstate) && (!sdl_controlstate)) {
		if (pressed) {
			if (((sym == SDL_DEADKEY_1) && sdl_shiftstate) ||
			    ((sym == SDL_DEADKEY_2) && sdl_shiftstate)) {
				dkey.id = sym;
				dkey.action = (sym == SDL_DEADKEY_2) ?
						      KEY_ACTION_DIA :
						      KEY_ACTION_CIR;
				dkey.ignore = true;
				return;
			}
		} else {
			if ((dkey.id == sym) && dkey.ignore) {
				dkey.id = 0;
				dkey.ignore = false;
				return;
			}
		}
	}

	// Is a dead key active?
	if (dkey.action != KEY_ACTION_NONE) {
		if (pressed) {
			int replace_mod;

			if ((sym == SDLK_A) || (sym == SDLK_E) ||
			    (sym == SDLK_I) || (sym == SDLK_O) ||
			    (sym == SDLK_U)) {
				// determine what key combination to send
				if (dkey.action == KEY_ACTION_CIR) {
					switch (sym) {
					case SDLK_A:
						replace_mod = 0x02;
						dkey.replace_code = QL_PERIOD;
						break;
					case SDLK_E:
						replace_mod = 0x06;
						dkey.replace_code = QL_L;
						break;
					case SDLK_I:
						replace_mod = 0x06;
						dkey.replace_code = QL_P;
						break;
					case SDLK_O:
						replace_mod = 0x06;
						dkey.replace_code =
							QL_SEMICOLON;
						break;
					case SDLK_U:
						replace_mod = 0x02;
						dkey.replace_code =
							QL_SEMICOLON;
						break;
					}
				} else if (dkey.action == KEY_ACTION_DIA) {
					replace_mod = 0x06;
					switch (sym) {
					case SDLK_A:
						dkey.replace_code =
							sdl_shiftstate ?
								QL_SLASH :
								QL_R;
						break;
					case SDLK_E:
						dkey.replace_code = QL_S;
						break;
					case SDLK_I:
						dkey.replace_code = QL_M;
						break;
					case SDLK_O:
						dkey.replace_code =
							sdl_shiftstate ? QL_D :
									 QL_4;
						break;
					case SDLK_U:
						if (!sdl_shiftstate)
							replace_mod = 0x04;
						dkey.replace_code =
							sdl_shiftstate ?
								QL_G :
								QL_BACKSLASH;
						break;
					}
				}

				// Need to detect the release of the translated key
				dkey.id = sym;
				dkey.ignore = false;
			} else {
				// Need to send base key
				if (dkey.action == KEY_ACTION_CIR) // ^
				{
					dkey.replace_code = QL_RBRACKET;
					replace_mod = 0x04;
				} else if (dkey.action == KEY_ACTION_DIA) // "
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
	if (!pressed && (dkey.id == sym) && !dkey.ignore) {
		SDLQLKeyrowChg(dkey.replace_code, pressed);
		dkey.replace_code = 0;
		dkey.id = 0;
		return;
	} else {
		if (sdlqlmap) {
#ifdef __WIN32__
			// Windows always sets the control key when alt Gr is pressed
			int mod = sdl_altstate |
				  ((sdl_controlstate && (!sdl_grfstate)) ? 2 :
									   0) |
				  sdl_shiftstate << 2 | sdl_grfstate << 4;
#else
			int mod = sdl_altstate | (sdl_controlstate << 1) |
				  sdl_shiftstate << 2 | sdl_grfstate << 4;
#endif
			while (sdlqlmap[i].sdl_kc != 0) {
				if ((sym == sdlqlmap[i].sdl_kc) &&
				    ((sdlqlmap[i].mod == MOD_WILD) ||
				     (mod == sdlqlmap[i].mod))) {
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
		if (sym == sdlqlmap_default[i].sdl_kc) {
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

static void setKeyboardLayout(void)
{
	const char *kbd_string = emulatorOptionString("kbd");
	usegrfstate = 0;
	keyboard = KEY_US;

	if (!strncasecmp("GB_ch", kbd_string, 5)) {
		sdlqlmap = sdlqlmap_GB_ch;
		keyboard = KEY_GB_CH;
		if (V1)
			printf("Using GB_ch keymap.\n");
	} else if (!strncasecmp("DE", kbd_string, 2)) {
// MacOS receives a specific keymap...
#if defined(__APPLE__) && defined(__MACH__)
		sdlqlmap = sdlqlmap_DE_MacOS;
#else
		sdlqlmap = sdlqlmap_DE;
#endif
		keyboard = KEY_DE;
		if (V1)
			printf("Using DE keymap.\n");
	} else if (!strncasecmp("GB", kbd_string, 2)) {
		sdlqlmap = sdlqlmap_GB;
		keyboard = KEY_GB;
		if (V1)
			printf("Using GB keymap.\n");
	} else if (!strncasecmp("ES", kbd_string, 2)) {
		sdlqlmap = sdlqlmap_ES;
		keyboard = KEY_ES;
		usegrfstate = 1;
		if (V1)
			printf("Using ES keymap.\n");
	} else if (!strncasecmp("IT", kbd_string, 2)) {
		sdlqlmap = sdlqlmap_IT;
		keyboard = KEY_IT;
		usegrfstate = 1;
		if (V1)
			printf("Using IT keymap.\n");
	} else if (!strncasecmp("US", kbd_string, 2)) {
		if (V1)
			printf("Using US keymap.\n");
		sdlqlmap = NULL;
	} else {
		if (V1)
			printf("Using default keymap. (use KBD=<countrycode> in sqlux.ini to change)\n");
		sdlqlmap = NULL;
	}
}

static void QLSDLProcessMouse(int *qlx, int *qly, int x, int y)
{
	*qlx = 0;
	*qly = 0;
	float x_ratio, y_ratio;

	if (SDL_GetWindowFlags(ql_window) & SDL_WINDOW_HIGH_PIXEL_DENSITY) {
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

	QLSDLProcessMouse(&qlx, &qly, x, y);

	QLMovePointer(qlx, qly);
}

#ifndef SDL_JOYSTICK_DISABLED
static int QLConvertWhichToIndex(Sint32 which)
{
	for (int i = 0; i < 2; ++i) {
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
			} else if (value > 10000) {
				queueKey(0, joy_char[index][offset + 1], 0);
				SDLQLKeyrowChg(joy_char[index][offset + 1], 1);
				SDLQLKeyrowChg(joy_char[index][offset], 0);
			} else {
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
#endif

void QLSDLProcessEvents(void)
{
	SDL_Event event;
	int keypressed;
	int w, h;

#if __EMSCRIPTEN__
	if (!SDL_PollEvent(&event)) {
		return;
	}
#else
	while (1) {
		if (!SDL_PollEvent(&event)) {
			continue;
		}
#endif
	switch (event.type) {
	case SDL_EVENT_KEY_DOWN:
		QLSDProcessKey(event.key.key, event.key.scancode, 1);
		break;
	case SDL_EVENT_KEY_UP:
		QLSDProcessKey(event.key.key, event.key.scancode, 0);
		break;
#ifndef SDL_JOYSTICK_DISABLED
	case SDL_EVENT_JOYSTICK_AXIS_MOTION:
		QLProcessJoystickAxis(event.jaxis.which, event.jaxis.axis,
				      event.jaxis.value);

		break;
	case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
		QLProcessJoystickButton(event.jbutton.which,
					event.jbutton.button, 1);
		break;
	case SDL_EVENT_JOYSTICK_BUTTON_UP:
		QLProcessJoystickButton(event.jbutton.which,
					event.jbutton.button, 0);
		break;
#endif
	case SDL_EVENT_QUIT:
		return;
		break;
	case SDL_EVENT_MOUSE_MOTION:
		QLProcessMouse((int)event.motion.x, (int)event.motion.y);
		//inside=1;
		break;
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		QLButton(event.button.button, 1);
		break;
	case SDL_EVENT_MOUSE_BUTTON_UP:
		QLButton(event.button.button, 0);
		break;
	case SDL_EVENT_WINDOW_MOUSE_ENTER:
		if (event.window.windowID == ql_windowid)
			SDL_HideCursor();
		break;
	case SDL_EVENT_WINDOW_MOUSE_LEAVE:
		if (event.window.windowID == ql_windowid)
			SDL_ShowCursor();
		break;
	case SDL_EVENT_WINDOW_RESIZED:
	case SDL_EVENT_WINDOW_EXPOSED:
		if (event.window.windowID == ql_windowid)
			QLSDLUpdateScreen();
		break;
	case SDL_EVENT_USER:
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
	for (int i = 0; i < 2; ++i) {
		if (joy[i].sdl_id)
			SDL_CloseJoystick(joy[i].sdl_id);
	}
#endif

	active_metronome = 0; // <-- Detiene el bucle del hilo suavemente
}
