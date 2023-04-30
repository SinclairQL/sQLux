/*
 * (c) UQLX - see COPYRIGHT
 */

/* fancy code to implement bigger screen */

#ifdef XSCREEN
#include "QL68000.h"
#include "QL.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xcodes.h"

#include "driver.h"
#include "QDOS.h"

#include "instructions.h"
#include "QInstAddr.h"
#include "qmtrap.h"
#include "unix.h"
#include "uqlx_cfg.h"
#include "QL_screen.h"

#define min(_a_, _b_) (_a_ < _b_ ? _a_ : _b_)
#define max(_a_, _b_) (_a_ > _b_ ? _a_ : _b_)

struct SCREENDEF {
	uw32 scrbase;
	uw32 scrlen;
	uw16 linel;
	uw16 xres;
	uw16 yres;
};

Cond XLookFor(uintptr_t *a, uw32 w, long nMax)
{
	while (nMax-- > 0 && RL((*a)) != w)
		(*a) += 2;
	return nMax > 0;
}

static int PtrPatchOk = 0;
void XPatchPTRENV()
{
	struct SCREENDEF *scrdef;
	int flag;

	scrdef = (Ptr)pc - 8000;
	while (XLookFor((uintptr_t *)&scrdef, 0x20000, 24000)) {
		if (RL(&(scrdef->scrlen)) == 0x8000 &&
		    RW(&(scrdef->linel)) == 0x80 &&
		    RW(&(scrdef->xres)) == 0x200 &&
		    RW(&(scrdef->yres)) == 0x100) {
			PtrPatchOk = 1;
			WL(&(scrdef->scrbase), qlscreen.qm_lo);
			WL(&(scrdef->scrlen), qlscreen.qm_len);
			WW(&(scrdef->linel), qlscreen.linel);
			WW(&(scrdef->xres), qlscreen.xres);
			WW(&(scrdef->yres), qlscreen.yres);

			return;
		} else
			scrdef = (struct SCREENDEF *)((char *)scrdef + 2);
	}

	if (!PtrPatchOk)
		printf("WARNING: could not patch Pointer Environment\n");
}

/* oadr is QL format, nadr c-pointer ! */
void scan_patch_chans(w32 oadr)
{
	w32 ca;
	w32 ce, cc;

	ce = ReadLong(0x2807c); /* table top */
	ca = ReadLong(0x28078);

	for (; ca <= ce; ca += 4) {
		cc = ReadLong(ca);
		if (ReadLong(cc + 4) == oadr) {
			WriteWord(cc + 0x64, qlscreen.linel);
			WriteLong(cc + 0x32, qlscreen.qm_lo);
		}
	}
}

/* fool ptr_gen */
static int xic = 0;
static uw32 cdrv = 0;
static uw32 fpdr;
static uw32 orig_open, orig_io, orig_close, orig_cdrv;

void devpefio_cmd()
{
	int xw, yw, xo, yo;
	int xm, ym;
	uw16 *pbl;
	uw32 saved_regs[16];
	int op;

	if ((uw8)reg[0] == 0xd) {
		pbl = (Ptr)memBase + aReg[1];
		xw = RW(pbl++);
		yw = RW(pbl++);
		xo = RW(pbl++);
		yo = RW(pbl);

		xm = xw + xo;
		ym = yw + yo;

		if (xm > 512 ||
		    ym > 256 && xm <= qlscreen.xres && ym <= qlscreen.yres) {
			uint16_t bwidth;
			/* get the border width */
			bwidth = ReadWord(aReg[0] + 0x20);

			WriteWord(aReg[0] + 0x18,
				  xo + (bwidth * 2) /*scr_par[2].i*/);
			WriteWord(aReg[0] + 0x1a, yo + bwidth /*scr_par[3].i*/);
			WriteWord(aReg[0] + 0x1c,
				  xw - (bwidth * 4) /*scr_par[0].i*/);
			WriteWord(aReg[0] + 0x1e,
				  yw - (bwidth * 2) /*scr_par[1].i*/);

			reg[0] = 0;
			rts();
			return;
		}
	}

	code = DEVPEFIO_OCODE;
	qlux_table[code]();
}
open_arg scr_par[6];
extern struct NAME_PARS con_name, scr_name;

#define GXS ((Ptr)memBase + UQLX_STR_SCRATCH)
void mangle_args(char *dev)
{
	int xw, yw;

	xw = scr_par[0].i + scr_par[2].i;
	yw = scr_par[1].i + scr_par[3].i;

	if (xw <= 512 && yw <= 256)
		return; /* no action needed */
	if (xw > qlscreen.xres || yw > qlscreen.yres)
		return; /* same, for other reasons */

	if (scr_par[4].i >= 0)
		snprintf(GXS + 2, 0x400, "%s__%ld", dev, scr_par[4].i);
	else
		snprintf(GXS + 2, 0x400, "%s", dev);

	WW(GXS, strlen(GXS + 2));
	aReg[0] = UQLX_STR_SCRATCH;
}

void devpefo_cmd()
{
	int res;
	uw32 sA0;
	uint16_t bwidth;

	sA0 = aReg[0];

	res = decode_name((Ptr)memBase + ((aReg[0]) & ADDR_MASK_E), &scr_name,
			  (open_arg *)&scr_par);
	if (res == 1)
		mangle_args("SCR_");
	else {
		res = decode_name((Ptr)memBase + ((aReg[0]) & ADDR_MASK_E),
				  &con_name, (open_arg *)&scr_par);
		if (res == 1)
			mangle_args("CON_");
	}

	/* we must patch ROM, but that is mprotect'ed !*/
	WW((Ptr)memBase + orig_open, DEVPEFO_OCODE);
	QLsubr(orig_open /*XS_GETOPEN((Ptr)memBase+(aReg[3]+0x18))*/, 200000000);
	WW((Ptr)memBase + orig_open, DEVPEFO_CMD_CODE);

	if ((w16)reg[0] < 0) {
		aReg[0] = sA0;

		rts();
		return;
	}

	/* get the border width */
	bwidth = ReadWord(aReg[0] + 0x20);

	/* set real bounds */
	WriteWord(aReg[0] + 0x18, scr_par[2].i + (bwidth * 2));
	WriteWord(aReg[0] + 0x1a, scr_par[3].i + bwidth);
	WriteWord(aReg[0] + 0x1c, scr_par[0].i - (bwidth * 4));
	WriteWord(aReg[0] + 0x1e, scr_par[1].i - (bwidth * 2));
	/* set physical screen: */

	WriteWord(aReg[0] + 0x64, qlscreen.linel);
	WriteLong(aReg[0] + 0x32, qlscreen.qm_lo);
	rts();
}

/* problems that could not be handled by device init */
void init_xscreen()
{
	uw32 ca;
	int j;

	/* get ch#0 */
	ca = ReadLong(0x28078);
	ca = ReadLong(ca);
	/* driver addr */
	ca = ReadLong(ca + 4);
	cdrv = ca;

	orig_io = ReadLong(cdrv + 4);
	orig_open = ReadLong(cdrv + 8);
	orig_close = ReadLong(cdrv + 12);
	orig_cdrv = cdrv;

	DEVPEFIO_OCODE = ReadWord(orig_io);
	WW((Ptr)memBase + orig_io, DEVPEF_CMD_CODE);

	qlux_table[DEVPEF_CMD_CODE] = devpefio_cmd;

	scan_patch_chans(orig_cdrv);

	DEVPEFO_OCODE = ReadWord(orig_open);
	WW((Ptr)memBase + orig_open, DEVPEFO_CMD_CODE);
	qlux_table[DEVPEFO_CMD_CODE] = devpefo_cmd;
}

/***************************************************************/
/* various related stuff */

static void pps_usg(char *m)
{
	printf("Bad geometry: %s. Please use 'nXm' where n=x size, m=y size\n",
	       m);

	qlscreen.xres = 512;
	qlscreen.yres = 256;
}

void parse_screen(char *geometry)
{
	char *p, *pp;
	long i;

	qlscreen.xres = 512;
	qlscreen.yres = 256;

	i = strtol(geometry, &p, 10);
	if (p == geometry) {
		pps_usg(geometry);
		return;
	}
	qlscreen.xres = max(i, 512);
	if (!(*p == 'x' || *p == 'X')) {
		pps_usg(geometry);
		return;
	} else
		p++;
	i = strtol(p, &pp, 10);
	if (p == pp) {
		pps_usg(geometry);
		return;
	}
	qlscreen.yres = max(i, 256);
}

#endif /* XSCREEN */
