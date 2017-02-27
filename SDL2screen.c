#include <inttypes.h>
#include <SDL2/SDL.h>

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

int QLSDLRenderScreen(void)
{
    printf("GGG: QLSDLRenderScreen\n");
	memcpy(pixel_buffer, xi_buf, sdl_width * sdl_height * 4);
    SDL_UpdateTexture(texture, NULL, pixel_buffer, sdl_width * 4);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect);
	SDL_RenderPresent(renderer);
}

