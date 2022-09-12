/*
 * Copyright (c) 2020-2021 Graeme Gregory
 *
 */

#ifndef _GPUSHADERS_H
#define _GPUSHADERS_H
#include <stdint.h>
#include <stdbool.h>

bool QLGPUCreateDisplay(int w , int h, int ly, uint32_t* id,
			const char* name, uint32_t sdl_window_mode);
void QLGPUUpdateDisplay(void);
void QLGPUSetFullscreen(void);
void QLGPUSetSize(int w, int h);

#endif

