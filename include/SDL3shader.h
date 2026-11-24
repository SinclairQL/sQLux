#ifndef _SDL3SHADER_H
#define _SDL3SHADER_H

#include <SDL3/SDL.h>

/* A GPU renderer with the CRT shader for the SHADER option, or NULL */
SDL_Renderer *QLShaderCreateRenderer(SDL_Window *window, int option);
/* Around the drawing of the QL screen texture */
void QLShaderBegin(SDL_Renderer *renderer, float tex_w, float tex_h);
void QLShaderEnd(SDL_Renderer *renderer);
void QLShaderQuit(void);
/* Mouse position (0 to 1) through the curvature of the screen */
void QLShaderDistort(float *x, float *y);

#endif
