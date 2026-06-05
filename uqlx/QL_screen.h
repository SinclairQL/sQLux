/*
 * (c) UQLX - see COPYRIGHT
 */
#pragma once

#include <stddef.h>

typedef struct _SCREEN_SPECS {
	uint32_t qm_lo;
	uint32_t qm_hi;
	uint32_t qm_len;

	uint32_t linel;
	int yres;
	int xres;
} screen_specs;

extern screen_specs qlscreen;

void QLPatchPTRENV(void);

