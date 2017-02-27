#include <inttypes.h>
#include <SDL2/SDL.h>

#include "QL_hardware.h"

static SDL_Window *window = NULL;
static SDL_Surface *screenSurface = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static uint32_t *pixel_buffer = NULL;
static int sdl_width = 0, sdl_height = 0;
static SDL_Rect src_rect;
static SDL_Rect dest_rect;

extern void *theROM;
extern char *xi_buf;

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

int QLSDLScreen(int width, int height, int zoom)
{
    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.w = width;
    src_rect.h = height;

    dest_rect.x = 0;
    dest_rect.y = 0;
    dest_rect.w = width * zoom;
    dest_rect.h = height * zoom;

    sdl_width = width;
    sdl_height = height;

    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
    	return 0;
	}

	if (SDL_CreateWindowAndRenderer(sdl_width * zoom, sdl_height * zoom, 0, &window,
                &renderer) != 0) {
    	printf("SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
		return 0;
    }

	if ((window == NULL) || (renderer == NULL)) {
		printf("SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
		return 0;
	}

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STATIC, sdl_width, sdl_height);
    pixel_buffer = malloc(sdl_width * sdl_height * 4);
    if (!pixel_buffer) {
        printf("Pixel Buffer allocation failed\b");
        return 0;
    }


}

static int QLSDLUpdatePixelBuffer(void)
{
    uint8_t *scr_ptr = theROM + 0x20000;
    uint32_t *pixel_ptr = pixel_buffer;
    int t1, t2, i;

    while(scr_ptr < (uint8_t *)(theROM + 0x28000)) {
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
    SDL_UpdateTexture(texture, NULL, pixel_buffer, sdl_width * 4);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect);
	SDL_RenderPresent(renderer);
}

