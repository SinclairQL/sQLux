/*
 * (c) UQLX - see COPYRIGHT
 */

/*#include "QLtypes.h"*/
#include "QL68000.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <SDL_timer.h>

#include "instructions.h"
#include "xcodes.h"
#include "QL.h"
#include "QDOS.h"

#include "QInstAddr.h"
#include "QL_config.h"
#include "unix.h"
#include "uqlx_cfg.h"

#include "unixstuff.h"

#include "util.h"

/*extern int schedCount;*/
extern int inside;
extern int screen_drawable;
extern int extInt;
extern int sct_size;
//extern char *scrModTable;

extern struct paste_buf *paste_bl;
int mouse_emu_reg;
int ochd = 0;

int min_idle = 5;
int HasPTR = 0;
int quickMouseUpdate = 1;

#define max(x, y)                                                              \
	({                                                                     \
		typeof(x) _x = (x);                                            \
		typeof(y) _y = (y);                                            \
		_x > _y ? _x : _y;                                             \
	})
#define sgn(x)                                                                 \
	({                                                                     \
		typeof(x) _x = (x);                                            \
		typeof(x) res = 0;                                             \
		res = (_x > 0) - (_x < 0);                                     \
		res;                                                           \
	})

#ifdef MOUSE
static w32 iscrdrv = 0;
static w32 ptrscrdrv = 0;
static int lastx = 0;
static int lasty = 0;
static int llastx = 0;
static int llasty = 0;
static int lastactive = 0;
static int ign_x = -1;
static int ign_y = -1;
int invisible = 0;

/*#define SCHEDULER_CMD_ADDR 0x14030*/

int getMouseX()
{
	return ReadWord(ptrscrdrv + 0x20);
}
int getMouseY()
{
	return ReadWord(ptrscrdrv + 0x22);
}
int ptractive()
{
	return !ReadByte(ptrscrdrv + 0x38);
}

void setMouseXY(int x, int y)
{
	if (x < 1)
		x = 1;
	if (y < 1)
		y = 1;
	WriteByte(ptrscrdrv + 0x16,
		  ReadByte(ptrscrdrv + 0x16) +
			  max(abs(x - getMouseX()), abs(y - getMouseY())));

	WriteWord(ptrscrdrv + 0x20, x);
	WriteWord(ptrscrdrv + 0x22, y);
	WriteByte(ptrscrdrv + 0x52 - 0x18, 0);
	WriteByte(ptrscrdrv + 0x8f - 0x18, 0);
}

void print_mousepos()
{
	printf("mouse x,y = %4d,%4d\n", getMouseX(), getMouseY());
}

#endif /* MOUSE */

w32 getSCRdriver()
{
	return ReadLong(ReadLong(ReadLong(0x28078)) +
			4); /* :HIER: abs. sysref raus */
}

/* link in scheduler task */
void SchedInit()
{
	reg[1] = 0x10;
	reg[2] = 0;
	QLtrap(1, 0x18, 2000000l);

	if (*reg == 0) {
		Ptr p = (Ptr)memBase + aReg[0];
		p = p + 4;

		WL(p, SCHEDULER_CMD_ADDR);
		WW((Ptr)memBase + SCHEDULER_CMD_ADDR, SCHEDULER_CMD_CODE);

		QLtrap(1, 0x1e, 200000l);
	}
#ifdef MOUSE
	iscrdrv = getSCRdriver();
	if (V3)
		printf("Initial Screen driver is %x\n", iscrdrv);
#endif
}

int MButtonUndetected = 0;
int MButtonPendRelease = 0;
int relbnr = 0;

#ifdef MOUSE
void QLButton(int bnr, int press)
{
#ifdef DEBUG_M /* .hpr, 13.9.99 probehalber */ /**/
	printf("Button %d %s\n", bnr, press ? "pressed" : "released");
	gKeyDown = press;
#endif

	if (press && bnr != 2)
		MButtonUndetected = 5;

	if (press == 0 && MButtonUndetected) {
		MButtonPendRelease = 5;
		relbnr = bnr;
		return;
	}

	if (HasPTR) {
		if (press == 0)
			WriteByte(ptrscrdrv + 0xaf - 0x18, 0);

		switch (bnr) {
		case 1:
			if (press)
				WriteByte(ptrscrdrv + 0xaf - 0x18, 1);
			WriteByte(ptrscrdrv + 0x8c - 0x18, 0);
#if 0
	  if (press) queueCode(0,0x36);
	  KeyrowChg(0x36,press);
#endif
#if 0
	  KeyrowChg(0x36,press);
	  if (!isMinerva)
	    {
	      if (press) queueCode(0,0);
	      MButtonDown=gKeyDown=press;
	    }
#endif

			break;
			/*  case 2: hot */
		case 2:
			break;
		case 3:
			if (press)
				WriteByte(ptrscrdrv + 0xaf - 0x18, 2);
			WriteByte(ptrscrdrv + 0x8c - 0x18, 0);
			break;
		}
		WriteByte(ptrscrdrv + 0x16,
			  ReadByte(ptrscrdrv + 0x16) + 1); /* int count ?*?*/
	}
}
#endif

#ifdef MOUSE
//extern int QLdone;

void MouseTask()
{
	int x, y;
	static int pendreset = 0;
	w32 saveA3;

	saveA3 = aReg[3];
	aReg[0] = 0;

	if (MButtonUndetected > 0) {
		/*printf("Button Undetected %d\n",MButtonUndetected);*/
		MButtonUndetected--;
	}

	if (MButtonPendRelease > 0) {
		/*printf("Button pending release %d\n",MButtonPendRelease);*/
		MButtonPendRelease--;
		if (MButtonPendRelease == 0) {
			/*printf("...release nr %d !\n",relbnr) ;*/
			QLButton(relbnr, 0);
		}
	}

	/* lets hope PTR_GEN is the first screen driver extension that gets loaded */
	if (iscrdrv && !ptrscrdrv && iscrdrv != getSCRdriver()) {
		reg[3] = 0;
		QLtrap(3, 0x70, 200000l);
		if (((w16)reg[0]) != 0) {
			iscrdrv = getSCRdriver();
			printf("registered new screen driver at %x\niop.pinf returns %x\n",
			       iscrdrv, (w16)reg[0]);
		} else {
			ptrscrdrv = getSCRdriver();
			if (V3)
				printf("PTRgen screen driver is %x\n",
				       ptrscrdrv);
			HasPTR = 1;
		}
	}

	if (HasPTR && !QLdone) {
#if 0
      if (ochd != ReadLong(ptrscrdrv+0xa8-0x18))
	{
	  ochd=ReadLong(ptrscrdrv+0xa8-0x18);
	  printf("PT_CHAD %x\n",ochd);
	}
#endif
#if 0
      printf("Pointer activ: %s\n",ReadWord(ptrscrdrv+0x38)?"yes":"no");
      printf("Mouse Hotkey: %s\n",ReadByte(ptrscrdrv+0x15)?"yes":"no");
      printf("mouse counter: %d\n",ReadByte(ptrscrdrv+0x19));
      printf("PT_WAKE: %d\n",ReadByte(ptrscrdrv+0x34));
#endif
#if 0
      printf("PT_BSTAT: %d\t\t",ReadByte(ptrscrdrv+0x48-0x18));
      printf("PT_BSUPP: %d\t\t",ReadByte(ptrscrdrv+0x49-0x18));
      printf("PT_BPRESS: %d\t\t",ReadByte(ptrscrdrv+0x4a-0x18));
      printf("PT_BCURR: %d\n",ReadByte(ptrscrdrv+0x4b-0x18));
#endif
		x = getMouseX();
		y = getMouseY();

		{
			/* printf("mouse pointer %sactivated\n",ptractive()?"":"de"); */
			if (!(ptractive())) {
				if (!lastactive) {
					pendreset = 50;
					lastactive = 1;
				}
			} else {
				pendreset = 0;
				if (!invisible) {
					if (!script) {
						invisible = 1;
						//InvisibleMousePtr();
					}
				}
				lastactive = 0;
			}
		}
		/*if (pendreset) printf("pendreset: %d\n",pendreset);*/
#ifndef QLMOUSE
		if (pendreset)
			if (--pendreset <= 0) {
				if (!script) {
					invisible = 0;
					//ResetMousePtr();
				}
			}
#endif
		/* } */
		if ((x != lastx || y != lasty) && lastx * lasty) {
#if 0
	  printf("internal mouse change - ");
	  print_mousepos();
#endif
			if (/*inside &&*/ !script) {
#ifdef XAW
				XWarpPointer(XtDisplay(bitMap), None,
					     XtWindow(bitMap), 0, 0, 0, 0, x,
					     y);
#else
				//XWarpPointer(display,None,imagewin,0,0,0,0, x,y);
				lastx = x;
				lasty = y;
				ign_x = x;
				ign_y = y;
#endif
			}
		}
	}
	aReg[3] = saveA3;
}
#endif

#define DISPLAY_CHANGED() display_changed()
static inline int display_changed()
{
	int i = 0;

	while (i < sct_size) {
		if (scrModTable[i])
			return 1;
		i++;
	}

	return 0;
}

#ifdef MOUSE
void QLMovePointer(int pos_x, int pos_y)
{
	if (pos_x == ign_x && pos_y == ign_y) {
		ign_x = -1;
		ign_y = -1;
		return;
	}
	if (HasPTR)
		if (pos_x != lastx || pos_y != lasty) {
			getMouseX();
			getMouseY();

			setMouseXY(pos_x, pos_y);

			llastx = lastx;
			lastx = pos_x;
			llasty = lasty;
			lasty = pos_y;
		}
}
#endif

void SchedulerCmd()
{
	w32 saved_regs[16];

	if ((long)((Ptr)gPC - (Ptr)memBase) - 2 != SCHEDULER_CMD_ADDR) {
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
		return;
	}

	save_regs(saved_regs);

#ifdef MOUSE
	MouseTask();
#endif

	if (QMD.cpu_hog == 0 && schedCount++ > min_idle) /* QDOS running idle */
	{
		/*printf(".");fflush(stdout);*/
		/*printf("schedCount %d\n",schedCount);*/
		SDL_Delay(20);
		if (nInst > 5) {
			nInst2 = nInst;
			nInst = 5;
		}
	}
	restore_regs(saved_regs);

	rts();
}
