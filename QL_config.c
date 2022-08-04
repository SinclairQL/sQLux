/*
 * (c) UQLX - see COPYRIGHT
 */

#include <stdbool.h>
#include <stdint.h>

#include "QL68000.h"
#include "QL.h"
#include <stdio.h>
#include "debug.h"
#include "iexl_general.h"
#include "xcodes.h"
#include "QDOS.h"
#include "QL_basext.h"
#include "QL_config.h"
#include "QL_driver.h"
#include "QL_poll.h"
#include "QL_screen.h"
#include "unix.h"
#include "unixstuff.h"

/*#include "QTimer.h"
#include "QColor.h"
#include "QSound.h"
#include "QDisk.h"
#include "QSerial.h" */
#include <string.h>
/*#include "ConfigDialog.h"*/
#include "QFilesPriv.h"
#include "SqluxOptions.hpp"
#include "uqlx_cfg.h"
#include "xqlmouse.h"
#include "Xscreen.h"

static short ramItem = -1;
extern int do_update;

static Cond qemlPatch = false;

int isMinerva = 0;
uw32 orig_kbenc;

int testMinervaVersion(char *ver);

bool LookFor(uint32_t *a, uint32_t w, int nMax)
{
	while (nMax-- > 0 && RL((Ptr)memBase + (*a)) != w)
		(*a) += 2;
	return nMax > 0;
}
static Cond LookFor2(uw32 *a, uw32 w, uw16 w1, long nMax)
{
rty:
	while (nMax-- > 0 && RL((Ptr)memBase + (*a)) != w)
		(*a) += 2;
	if (RW((Ptr)memBase + (*a) + 4) == w1)
		return nMax > 0;
	(*a) += 2;
	if (nMax > 0)
		goto rty;
	return 0;
}

static void PatchKeyTrans(void)
{
	KEYTRANS_CMD_ADDR = RL((Ptr)memBase + 0xbff2);
	KEYTRANS_OCODE = RW((uw16 *)((Ptr)memBase + KEYTRANS_CMD_ADDR));
	WW(((uw16 *)((Ptr)memBase + KEYTRANS_CMD_ADDR)), KEYTRANS_CMD_CODE);
}

static Cond PatchFind(void)
{
	Cond ok;

	IPC_CMD_ADDR = 0x2b80;
	IPCR_CMD_ADDR = 0x2e98;
	IPCW_CMD_ADDR = 0x2e78;
	ok = LookFor(&IPC_CMD_ADDR, 0x40e7007c, 2000);
	if (ok)
		ok = LookFor(&IPCR_CMD_ADDR, 0x12bc000e, 2000);
	if (ok)
		ok = LookFor(&IPCW_CMD_ADDR, 0xe9080000, 2000);
	return ok;
}

static Cond InstallQemlRom(void)
{
	if (!qemlPatch) {
		if (isMinerva) {
			ROMINIT_CMD_ADDR = 0x0;
			qemlPatch = LookFor2(&ROMINIT_CMD_ADDR, 0x0c934afbl, 1,
					     48 * 1024);
			if (testMinervaVersion("1.89")) {
				uintptr_t mpatch_addr = 0x4e6;
				uint16_t tmp = RW((Ptr)memBase + mpatch_addr);
				printf("Minerva 1.89, checking for >1MB patch\n");
				if (tmp == 0xd640) {
					WW((Ptr)memBase + mpatch_addr, 0xd680);
					printf("....applied patch\n");
				}
			}
		}

		else /* not Minerva */
		{
			ROMINIT_CMD_ADDR = 0x4a00;
			qemlPatch =
				LookFor(&ROMINIT_CMD_ADDR, 0x0c934afbl, 1000);
		}
		if (!qemlPatch)
			printf("Warning: could not patch ROM scan sequence\n");
		if (qemlPatch) {
			WW(((uw16 *)((Ptr)memBase + ROMINIT_CMD_ADDR)),
			   ROMINIT_CMD_CODE);
			WW(((uw16 *)((Ptr)memBase + MDVIO_CMD_ADDR)),
			   MDVIO_CMD_CODE); /* device driver routines */
			WW(((uw16 *)((Ptr)memBase + MDVO_CMD_ADDR)),
			   MDVO_CMD_CODE);
			WW(((uw16 *)((Ptr)memBase + MDVC_CMD_ADDR)),
			   MDVC_CMD_CODE);
			WW(((uw16 *)((Ptr)memBase + MDVSL_CMD_ADDR)),
			   MDVSL_CMD_CODE);
			WW(((uw16 *)((Ptr)memBase + MDVFO_CMD_ADDR)),
			   MDVFO_CMD_CODE);
		}
	}
	return qemlPatch;
}

/* presently simply search for "QView" */
int testMinerva(void)
{
	char *p = (char *)memBase;
	char *limit = (Ptr)memBase + 48 * 1024;
	/* char *limit=(Ptr)memBase+4*1024; */
	/* "JSL1" should be found in 1st 4K */ /* .hpr 8.8.99 */

	do {
		/*       p=memchr(p,'Q',limit-p); */
		/*       if (!p) return 0; */
		/*       if (!memcmp(++p,"View",4)) return 1; */

		p = memchr(p, 'J', limit - p);
		if (!p)
			return 0;
		if (!memcmp(++p, "SL1", 3))
			return 1;
	} while (p && p < limit);

	return 0;
}

int testMinervaVersion(char *ver)
{
	char *p = (char *)memBase;
	char *limit = (Ptr)memBase + 48 * 1024;

	do {
		p = memchr(p, ver[0], limit - p);
		if (!p)
			return 0;
		if (!memcmp(++p, &ver[1], 3))
			return 1;
	} while (p && p < limit);
	return 0;
}

static uint32_t bootpatchaddr[] = {
                    0x842A, /* Minerva 1.89 */
                    0x83CA, /* Minerva 1.98 */
                    0x83CC, /* Minerva 1.98a1 */
                    0x4BE6, /* JS */
                    0 };

static void PatchBootDev()
{
	int i = 0;

	/* patch the boot device in ROM */
	while (bootpatchaddr[i]) {
		if (!strncasecmp((void *)memBase + bootpatchaddr[i], "mdv1",
				 4)) {
			if (V2)
				printf("Patching Boot Device %s at 0x%x\n",
					optionString("boot_dev"), bootpatchaddr[i]);

			strncpy((void *)memBase + bootpatchaddr[i], optionString("boot_dev"),
				4);
		}
		i++;
	}
}

int LoadMainRom(void) /* load and modify QL ROM */
{
	short e;
	long l;
	uw32 a;
	Cond p;

	qemlPatch = false;

	p = 1; /*ReasonableROM(memBase);*/

	isMinerva = testMinerva();
	if (isMinerva && V1)
		printf("using Minerva ROM\n");

	//if(V1)printf("no_patch: %d\n",QMD.no_patch);

	if (p && !optionInt("no_patch")) {
		if (!isMinerva) {
			if (p)
				p = PatchFind();
			if (p) {
				WW((((Ptr)memBase + IPC_CMD_ADDR)),
				   IPC_CMD_CODE);
				WW((((Ptr)memBase + IPCR_CMD_ADDR)),
				   IPCR_CMD_CODE);
				WW((((Ptr)memBase + IPCW_CMD_ADDR)),
				   IPCW_CMD_CODE);
			}
		}

		WW((((Ptr)memBase + 0x4000 + RW((uw16 *)((Ptr)memBase + 0x124)))),
		   MDVR_CMD_CODE); /* read mdv sector */
		WW((((Ptr)memBase + 0x4000 + RW((uw16 *)((Ptr)memBase + 0x126)))),
		   MDVW_CMD_CODE); /* write mdv sector */
		WW(((uw16 *)((Ptr)memBase + 0x4000 +
			     RW((uw16 *)((Ptr)memBase + 0x128)))),
		   MDVV_CMD_CODE); /* verify mdv sector */
		WW(((uw16 *)((Ptr)memBase + 0x4000 +
			     RW((uw16 *)((Ptr)memBase + 0x12a)))),
		   MDVH_CMD_CODE); /* read mdv sector header */

		if (!isMinerva && optionInt("fast_startup"))
			WW(((uw16 *)((Ptr)memBase + RL(&memBase[1]))),
			   FSTART_CMD_CODE); /* fast startup patch */
		/* FastStartup() -- 0xadc7 */

		if (!isMinerva) {
			/* correct bugs which crashes the QL with 4M ram */
			a = 0x250;
			p = LookFor(&a, 0xd6c028cb, 6);
			if (p) {
				WW(((uw16 *)((Ptr)memBase + a)), 0xd7c0);
				a = 0x3120;
				p = LookFor(&a, 0xd2c02001, 250);
			} else if (V3)
				printf("looks like ROM doesn't have 16MB bugs\n");
			if (p) {
				WW(((uw16 *)((Ptr)memBase + a)), 0xd3c0);
				a = 0x4330;
				p = LookFor(&a, 0x90023dbc, 120);
				if (p)
					WW(((uw16 *)((Ptr)memBase + a)), 0x9802);
			}

			PatchKeyTrans();
		}

		p = InstallQemlRom();

		PatchBootDev();
	}
	if (!p && !optionInt("no_patch"))
		printf("warning : could not complete ROM patch\n");

	/* last not least intrument the ROM code HW register access */

	if (e == 0 && !p)
		e = ERR_ROM_UNKNOWN;
	return e;
}

void save_regs(void *p)
{
	memcpy(p, reg, 14 * 4);
}
void restore_regs(void *p)
{
	memcpy(reg, p, 14 * 4);
}

extern int tracetrap;

void InitROM(void)
{
	w32 saved_regs[16];
	char qvers[6];
	char *initstr = "UQLX v%s, release\012      %s\012QDOS Version %s\012";
	long sysvars, sxvars;

	if ((uintptr_t)((Ptr)gPC - (Ptr)memBase) - 2 != ROMINIT_CMD_ADDR) {
		printf("PC %8x is not patched with ROMINIT\n",
		       (unsigned)((uintptr_t)gPC - (uintptr_t)memBase));
		exception = 4;
		extraFlag = true;
		return;
	}
#if 0
	printf("a6=%x, basic at %x\n",aReg[6],ReadLong(0x28010));
#endif

	save_regs(saved_regs);

	do_update = 1; /* flip in screen RAM */

#ifdef OLD_PATCH
	/* lea $0C000,a3 */
	aReg[3] = 0x0c000;
	gPC += 2;
#else
	WW((Ptr)gPC - 2, 0x0c93); /* restore original instruction */
#endif
#if 0
	KillSound();
	CloseSerial();
	InitSerial();
	ZeroKeyboardBuffer();
#endif

	/* delete old MDV drivers (for optical reasons) */
	WriteLong(0x28048, 0);

	InitFileDrivers();
	InitDrivers();

#ifdef XSCREEN
	/*printf("call init_xscreen\n");*/
	init_xscreen();
#endif

	SchedInit();

	init_bas_exts();

	QLtrap(1, 0, 20000l);
#if 0
	printf("QDOS vars at %x, trap res=%d, RTOP=%d\n",aReg[0],reg[0],RTOP);
#endif

	sysvars = aReg[0];
	sxvars = RL((Ptr)memBase + sysvars + 0x7c);
#if 0
	if (isMinerva)
	  printf("Minerva extended vars at %x\n",sxvars);
#endif
	if (V3)
		printf("sysvars at %x, ux RAMTOP %d, sys.ramt %d, qlscreen at %d\n",
		       (unsigned)sysvars, RTOP, sysvar_l(20), qlscreen.qm_lo);

	// QDOS version
	WL((Ptr)qvers, reg[2]);
	qvers[4] = 0;

#if 0
	p=(Ptr)memBase+RTOP-0x07FFEl;
	sprintf(p,initstr,uqlx_version,release,qvers);
	WriteWord(aReg[1]=RTOP-0x08000l,strlen(p));

#if 1
	QLvector(0xd0,200000);
#else
	WriteLong((*sp)-=4,(w32)gPC-(w32)memBase);
	gPC=(uw16*)((Ptr)memBase+RW((uw16*)((Ptr)memBase+0xd0)));	/* write string */
#endif
#endif

	/* now install TKII defaults */

	reg[1] = 0x6c;
	reg[2] = 0;
	QLtrap(1, 0x18, 200000);
	if (reg[0] == 0) {
		if (V3)
			printf("Initialising TK2 device defaults\n");
		WriteLong(0x28070 + 0x3c, aReg[0]);
		WriteLong(0x28070 + 0x40, 32 + aReg[0]);
		WriteLong(0x28070 + 0x44, 64 + aReg[0]);
		WriteWord(aReg[0], strlen(DATAD));
		strcpy((char *)((Ptr)memBase + aReg[0] + 2), DATAD);
		WriteWord(aReg[0] + 32, strlen(PROGD));
		strcpy((char *)((Ptr)memBase + aReg[0] + 34), PROGD);
		WriteWord(aReg[0] + 64, strlen(SPOOLD));
		strcpy((char *)((Ptr)memBase + aReg[0] + 66), SPOOLD);
	}

	/* link in Minerva keyboard handling */
#if 1
	if (isMinerva) {
		reg[1] = 8;
		reg[2] = 0;
		QLtrap(1, 0x18, 200000);
		if (reg[0] == 0) {
			WW((Ptr)memBase + MIPC_CMD_ADDR, MIPC_CMD_CODE);
			WL((Ptr)memBase + aReg[0],
			   RL((Ptr)memBase + sxvars + 0x14));
			WL((Ptr)memBase + aReg[0] + 4, MIPC_CMD_ADDR);
			WL((Ptr)memBase + sxvars + 0x14, aReg[0]);
		}
		WW((Ptr)memBase + KBENC_CMD_ADDR, KBENC_CMD_CODE);
		orig_kbenc = RL((Ptr)memBase + sxvars + 0x10);
		WL((Ptr)memBase + sxvars + 0x10, KBENC_CMD_ADDR);
#if 0
	    printf("orig_kbenc=%x\nreplacement kbenc=%x\n",orig_kbenc,KBENC_CMD_ADDR);
	    printf("sx_kbenc addr=%x\n",sxvars+0x10);
#endif
	}
#endif

	init_poll();

	/* make sure it wasn't changed somewhere */
	restore_regs(saved_regs);
#ifdef OLD_PATCH
	aReg[3] = 0x0c000;
#endif

#ifndef OLD_PATCH
	qlux_table[code = 0x0c93](); /* run the original routine */
#endif
}
