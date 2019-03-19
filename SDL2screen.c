#include <inttypes.h>
#include <SDL2/SDL.h>

#include "QL_hardware.h"
#include "uqlx_cfg.h"
#include "QL68000.h"

static SDL_Window *ql_window = NULL;
static SDL_Surface *ql_window_surface = NULL;
static SDL_PixelFormat *ql_window_format = NULL;
static SDL_Surface *ql_screen = NULL;
static SDL_Rect src_rect;
static SDL_Rect dest_rect;
static char sdl_win_name[128];

static int colors[8]={
		0x000000,
		0x0000FF,
		0xFF0000,
		0xFF00FF,
		0x00FF00,
		0x00FFFF,
		0xFFFF00,
		0xFFFFFF
};

struct QLcolor {
	int r;
	int g;
	int b;
};

struct QLcolor QLcolors[8] = {
		{ 0x00, 0x00, 0x00},
		{ 0x00, 0x00, 0xFF},
		{ 0xFF, 0x00, 0x00},
		{ 0xFF, 0x00, 0xFF},
		{ 0x00, 0xFF, 0x00},
		{ 0x00, 0xFF, 0xFF},
		{ 0xFF, 0xFF, 0x00},
		{ 0xFF, 0xFF, 0xFF},
};

uint32_t SDLcolors[8];

int QLSDLScreen(void)
{
	int i;

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.w = qlscreen.xres;
	src_rect.h = qlscreen.yres;

	dest_rect.x = 0;
	dest_rect.y = 0;
	dest_rect.w = qlscreen.xres * qlscreen.zoom;
	dest_rect.h = qlscreen.yres * qlscreen.zoom;

	snprintf(sdl_win_name, 128, "QL - %s, %dK", QMD.sysrom, RTOP/1024);

	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		printf("SDL_Init Error: %s\n", SDL_GetError());
		return 0;
	}

	ql_window = SDL_CreateWindow(sdl_win_name, 0, 0, qlscreen.xres * qlscreen.zoom,
			qlscreen.yres * qlscreen.zoom, 0);
	if (ql_window == NULL) {
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
		return 0;
	}

	ql_window_surface=SDL_GetWindowSurface(ql_window);
	ql_window_format = ql_window_surface->format;

	ql_screen = SDL_CreateRGBSurfaceWithFormat(0, qlscreen.xres, qlscreen.yres,
			ql_window_format->BitsPerPixel, ql_window_format->format);

	
	for (i = 0; i < 8; i++)
		SDLcolors[i] = SDL_MapRGB(ql_window_format, QLcolors[i].r, QLcolors[i].g, QLcolors[i].b);
}

static int QLSDLUpdatePixelBuffer()
{
	uint8_t *scr_ptr = (void *)theROM + qlscreen.qm_lo;
	uint32_t *pixel_ptr32;
	uint16_t *pixel_ptr16;
	int t1, t2, i, color;

	if (SDL_MUSTLOCK(ql_screen)) {
		SDL_LockSurface(ql_screen);
	}


	if(ql_window_format->BitsPerPixel == 32)
		pixel_ptr32 = ql_screen->pixels;
	else
		pixel_ptr16 = ql_screen->pixels;

	pixel_ptr32 = ql_screen->pixels;

	while(scr_ptr < (uint8_t *)((void *)theROM + qlscreen.qm_lo +
			qlscreen.qm_len)) {
		t1 = *scr_ptr++;
		t2 = *scr_ptr++;

		if (display_mode == 8) {

			for(i = 0; i < 8; i+=2) {
				uint32_t x;

				color = ((t1&2)<<1)+((t2&3))+((t1&1)<<3);

				x = SDLcolors[color];

				if (ql_window_format->BitsPerPixel == 32) {
					*(pixel_ptr32 + 7-(i)) = x;
					*(pixel_ptr32 + 7-(i+1)) = x;
				} else {
					*(pixel_ptr16 + 7-(i)) = x;
					*(pixel_ptr16 + 7-(i+1)) = x;
				}

				t1 >>=2;
				t2 >>=2;
			}
		} else {

			for(i=0; i<8; i++)
			{
				uint32_t x;

				color = ((t1&1)<<2)+((t2&1)<<1)+((t1&1)&(t2&1));

				x = SDLcolors[color];

				if (ql_window_format->BitsPerPixel == 32) {
					*(pixel_ptr32 + 7-i) = x;
				} else {
					*(pixel_ptr16 + 7-i) = x;
				}

				t1 >>= 1;
				t2 >>= 1;
			}
		}
		if(ql_window_format->BitsPerPixel == 32)
			pixel_ptr32 += 8;
		else
			pixel_ptr16 += 8;
	}
	SDL_UnlockSurface(ql_screen);
}

int QLSDLRenderScreen(void)
{
	void *texture_buffer;
	int pitch;
	int w, h;
	SDL_PixelFormat pixelformat;

	QLSDLUpdatePixelBuffer();

	if(SDL_MUSTLOCK(ql_window_surface)) {
		SDL_LockSurface(ql_window_surface);
	}
	SDL_BlitScaled(ql_screen, &src_rect, ql_window_surface, NULL);
	((uint32_t *)(ql_window_surface->pixels))[0]=0xFFFFFF;
	SDL_UnlockSurface(ql_window_surface);

	SDL_UpdateWindowSurface(ql_window);
}

/* Store the keys pressed */
unsigned int sdl_keyrow[]={0,0,0,0,0,0,0,0};
int sdl_shiftstate,sdl_controlstate, sdl_altstate;

static void SDLQLKeyrowChg(int code,int press)
{
	int j;

	if (code > -1) {
		j= 1 << (code % 8);

		if (press)
			sdl_keyrow[7 - code / 8] |= j;
		else
			sdl_keyrow[7 - code / 8] &= ~j;
	}
}

struct SDLQLMap
{
	SDL_Keycode sdl_kc;
	int code;
	int qchar;
};

static struct SDLQLMap sdlqlmap[] = {
		{SDLK_LEFT,         49, 0},
		{SDLK_UP,           50, 0},
		{SDLK_RIGHT,        52, 0},
		{SDLK_DOWN,         55, 0},

		{SDLK_F1,           57, 0},
		{SDLK_F2,           59, 0},
		{SDLK_F3,           60, 0},
		{SDLK_F4,           56, 0},
		{SDLK_F5,           61, 0},

		{SDLK_RETURN,       48, 0},
		{SDLK_SPACE,        54, 0},
		{SDLK_TAB,          19, 0},
		{SDLK_ESCAPE,       51, 0},
		//{SDLK_CAPSLOCK,     33, 0},
		{SDLK_RIGHTBRACKET, 40, 0},
		{SDLK_5,            58, 0},
		{SDLK_4,            62, 0},
		{SDLK_7,            63, 0},
		{SDLK_LEFTBRACKET,  32, 0},
		{SDLK_z,            41, 0},

		{SDLK_PERIOD,       42, 0},
		{SDLK_c,            43, 0},
		{SDLK_b,            44, 0},
		{SDLK_BACKQUOTE,    45, 0},

		{SDLK_m,            46, 0},
		{SDLK_QUOTE,        47, 0},
		{SDLK_BACKSLASH,    53, 0},
		{SDLK_k,            34, 0},
		{SDLK_s,            35, 0},
		{SDLK_f,            36, 0},
		{SDLK_EQUALS,       37, 0},
		{SDLK_g,            38, 0},
		{SDLK_SEMICOLON,    39, 0},
		{SDLK_l,            24, 0},
		{SDLK_3,            25, 0},
		{SDLK_h,            26, 0},
		{SDLK_1,            27, 0},
		{SDLK_a,            28, 0},
		{SDLK_p,            29, 0},
		{SDLK_d,            30, 0},
		{SDLK_j,            31, 0},
		{SDLK_9,            16, 0},
		{SDLK_w,            17, 0},
		{SDLK_i,            18, 0},
		{SDLK_r,            20, 0},
		{SDLK_MINUS,        21, 0},
		{SDLK_y,            22, 0},
		{SDLK_o,            23, 0},
		{SDLK_8,            8, 0},
		{SDLK_2,            9, 0},
		{SDLK_6,            10, 0},
		{SDLK_q,            11, 0},
		{SDLK_e,            12, 0},
		{SDLK_0,            13, 0},
		{SDLK_t,            14, 0},
		{SDLK_u,            15, 0},
		{SDLK_x,            3, 0},
		{SDLK_v,            4, 0},
		{SDLK_SLASH,        5, 0},
		{SDLK_n,            6, 0},
		{SDLK_COMMA,        7, 0},
		/*
  {SDLK_Next,-1,220},
  {SDLK_Prior,-1,212},
  {SDLK_Home,-1,193},
  {SDLK_End,-1,201},
		 */
		{0,0,0}
};


void QLSDProcessKey(SDL_Keysym *keysym, int pressed)
{
	int i = 0;
	int mod = 0;

	/* Special case backspace */
	if ((keysym->sym == SDLK_BACKSPACE) && pressed) {
		queueKey(1 << 1, 49, 0);
		return;
	}

	switch (keysym->sym) {
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		sdl_shiftstate = pressed;
		break;
	case SDLK_LCTRL:
	case SDLK_RCTRL:
		sdl_controlstate = pressed;
		break;
	case SDLK_LALT:
	case SDLK_RALT:
		sdl_altstate = pressed;
		break;
	}

	mod = sdl_altstate | sdl_controlstate << 1 | sdl_shiftstate << 2;

	while (sdlqlmap[i].sdl_kc != 0) {
		if (keysym->sym == sdlqlmap[i].sdl_kc) {
			if (pressed)
				queueKey(mod, sdlqlmap[i].code, 0);
			SDLQLKeyrowChg(sdlqlmap[i].code, pressed);
		}
		i++;
	}
}

int QLSDLProcessEvents(void)
{
	SDL_Event event;
	int keypressed;

	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_KEYDOWN:
			QLSDProcessKey(&event.key.keysym, 1);
			break;
		case SDL_KEYUP:
			QLSDProcessKey(&event.key.keysym, 0);
			break;
		case SDL_QUIT:
			cleanup(0);
			break;
		case SDL_MOUSEMOTION:
			QLMovePointer(event.motion.x / qlscreen.zoom,
					event.motion.y / qlscreen.zoom);
			//inside=1;
			break;
		case SDL_MOUSEBUTTONDOWN:
			QLButton(event.button.button, 1);
			break;
		case SDL_MOUSEBUTTONUP:
			QLButton(event.button.button, 0);
			break;
		default:
			break;
		}
	}

	QLSDLRenderScreen();
}

void QLSDLExit(void)
{
	SDL_Quit();
}

