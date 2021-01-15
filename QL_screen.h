#ifndef QLSCREEN_H
#define QLSCREEN_H
/*
 * (c) UQLX - see COPYRIGHT
 */

typedef struct _SCREEN_SPECS {
	uw32 qm_lo;
	uw32 qm_hi;
	uw32 qm_len;

	uw32 linel;
	int yres;
	int xres;
} screen_specs;

extern screen_specs qlscreen;

void QLPatchPTRENV(void);

#endif
