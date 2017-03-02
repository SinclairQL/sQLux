#include <inttypes.h>
#include <SDL2/SDL.h>

#include "QL_hardware.h"
#include "uqlx_cfg.h"
#include "QL68000.h"

static SDL_Window *window = NULL;
static SDL_Surface *screenSurface = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static uint32_t *pixel_buffer = NULL;
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

int QLSDLScreen(void)
{
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

	if (SDL_CreateWindowAndRenderer(qlscreen.xres * qlscreen.zoom,
                qlscreen.yres * qlscreen.zoom, 0, &window,
                &renderer) != 0) {
    	printf("SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
		return 0;
    }

    SDL_SetWindowTitle(window, sdl_win_name);

	if ((window == NULL) || (renderer == NULL)) {
		printf("SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
		return 0;
	}

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STATIC, qlscreen.xres, qlscreen.yres);
    pixel_buffer = malloc(qlscreen.xres * qlscreen.yres * 4);
    if (!pixel_buffer) {
        printf("Pixel Buffer allocation failed\b");
        return 0;
    }
}

static int QLSDLUpdatePixelBuffer(void)
{
    uint8_t *scr_ptr = (void *)theROM + qlscreen.qm_lo;
    uint32_t *pixel_ptr = pixel_buffer;
    int t1, t2, i;

    //printf("qlscreen.qm_length %x\n", qlscreen.qm_len);
    while(scr_ptr < (uint8_t *)((void *)theROM + qlscreen.qm_lo +
                qlscreen.qm_len)) {
        t1 = *scr_ptr++;
        t2 = *scr_ptr++;

        if (display_mode == 8) {

            for(i=0; i<4; i++) {
                uint32_t x;

                x = colors[((t1&2)<<1)+((t2&3))+((t1&1)<<3)];
                *(pixel_ptr + 7-(2*i)) = x;
                *(pixel_ptr + 7-(2*i+1)) = x;

                t1 >>=2;
                t2 >>=2;
            }
        } else {

            for(i=0; i<8; i++)
            {
                uint32_t x;

                x = colors[((t1&1)<<2)+((t2&1)<<1)+((t1&1)&(t2&1))];
                *(pixel_ptr + 7-i) = x;
                t1 >>= 1;
                t2 >>= 1;
            }
        }
        pixel_ptr += 8;
    }
}

int QLSDLRenderScreen(void)
{
    QLSDLUpdatePixelBuffer();
    SDL_UpdateTexture(texture, NULL, pixel_buffer, qlscreen.xres * 4);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect);
	SDL_RenderPresent(renderer);
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
        default:
            break;
        }
    }

    QLSDLRenderScreen();
}

void QLSDLExit(void)
{
    free(pixel_buffer);
    SDL_Quit();
}

