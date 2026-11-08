#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"
#include "emulator_options.h"
#include "memaccess.h"
#include "QInstAddr.h"
#include "QL68000.h"
#include "qsound.h"
#include "QLtraps.h"
#include "QL_config.h"
#include "QL_cconv.h"
#include "QL_hardware.h"
#include "QL_screen.h"
#include "sds.h"
#include "unixstuff.h"
#include "version.h"
#include "xcodes.h"
#include "xqlmouse.h"
#include "mdv.h"
#include "ipc.h"
#include "cycles68k.h"
#include "zx8301.h"
#include "qimsi.h"
#include "qimi.h"
#include "ice.h"
#include "thor.h"

// TODO: fixup iexl_general.h to not break stuff
void InitialSetup(void);
extern int qsound_mapped; /* memaccess.c */
extern int qliont_ay_mapped; /* memaccess.c */
extern uint32_t qsound_base; /* memaccess.c */
extern uint32_t mem_limit; /* memaccess.c */
extern uint32_t mem_limit_for(uint32_t rtop); /* memaccess.c */

int emulatorLoadRom(const char *romDir, const char *romName, uint32_t addr,
		    size_t size)
{
	struct stat romStat;
	int ret, romFile;
	sds romPath;

	if (romDir[0] == '~') {
		romPath = sdscatprintf(sdsnew(""), "%s/%s/%s", homedir,
				       romDir + 1, romName);
	} else {
		romPath = sdscatprintf(sdsnew(""), "%s/%s", romDir, romName);
	}

	ret = stat(romPath, &romStat);
	if (ret < 0) {
		fprintf(stderr, "FUNC: %s ERR: %s VAL: %s\n", __func__,
			strerror(errno), romPath);
		return ret;
	}

	if (romStat.st_size > size) {
		fprintf(stderr,
			"FUNC: %s ERR: Rom Size Error VAL: %zd != %jd\n",
			__func__, size, (intmax_t)romStat.st_size);
		return -1;
	} else if (romStat.st_size < size)
		printf("SQLUX WARNING: ROM %s is not %dk", romName,
		       size / 1024);

	romFile = open(romPath, O_RDONLY);
	if (romFile < 0) {
		fprintf(stderr, "FUNC: %s ERR: %s VAL: %s\n", __func__,
			strerror(errno), romPath);
		return -1;
	}
	ret = read(romFile, (char *)memBase + addr, size);
	if (ret < 0) {
		fprintf(stderr, "FUNC: %s ERR: %s VAL: %s\n", __func__,
			strerror(errno), romPath);
	}
	close(romFile);

	sdsfree(romPath);

	return ret;
}

void emulatorInit()
{
	char *rf;
	int rl = 0;
	void *tbuff;
	int ret;

	if (V1)
		printf("*** sQLux release %s\n\n", release);

	tzset();

	if (emulatorOptionInt("ramsize")) {
		RTOP = (128 + emulatorOptionInt("ramsize")) * 1024;
	} else {
		RTOP = emulatorOptionInt("ramtop") * 1024;
	}

	if (RTOP < (256 * 1024)) {
		fprintf(stderr, "Sorry not enough ram defined for QDOS %dK\n",
			(RTOP / 1024) - 128);
		exit(1);
	}

	// At least 1 MB is allocated to memBase to ensure the $0C0000 zone fits within the PC's memory
	// Memory reserved above RTOP for the expansions, see mem_limit_for()
	mem_limit = mem_limit_for(RTOP);
	size_t alloc_size = mem_limit;
	memBase = (int32_t *)malloc(alloc_size);
	if (memBase == NULL) {
		fprintf(stderr, "sorry, not enough memory for a %dK QL\n",
			RTOP / 1024);
		exit(1);
	}
	memset(memBase, 0, alloc_size); // Clean memory

	if (EmulatorTable()) {
		fprintf(stderr, "Failed to allocate instruction table\n");
		free(memBase);
		exit(1);
	}

	const char *romdir = emulatorOptionString("romdir");
	const char *sysrom = emulatorOptionString("sysrom");
	const char *romport = emulatorOptionString("romport");
	const char *romim = emulatorOptionString("romim");
	const char *mouse = emulatorOptionString("mouse");
	const char *iorom1 = emulatorOptionString("iorom1");
	const char *iorom2 = emulatorOptionString("iorom2");
	const char *qsrom = emulatorOptionString("qsrom");

	ret = emulatorLoadRom(romdir, sysrom, QL_ROM_BASE, QL_ROM_SIZE);
	if (ret < 0) {
		fprintf(stderr, "Error Loading sysrom %s\n", sysrom);
		exit(ret);
	}

	// The QIMSI registers live in the ROM port: MOUSE = QIMSI takes
	// precedence over ROMPORT/ROMIM
	if (!strcasecmp(mouse, "QIMSI")) {
		if (strlen(romport) || strlen(romim))
			fprintf(stderr,
				"SQLUX WARNING: MOUSE = QIMSI and ROMPORT are mutually exclusive, ROMPORT disabled (QIMSI has preference)\n");
		qimsi_init();
		printf(">>> [QIMSI] Mouse registers at $FED0-$FEDF\n");
	} else if (strlen(romport)) {
		ret = emulatorLoadRom(romdir, romport, QL_ROM_PORT_BASE,
				      QL_ROM_PORT_SIZE);
		if (ret < 0) {
			fprintf(stderr, "Error Loading romport %s\n", romport);
			exit(ret);
		}
	} else if (strlen(romim)) {
		ret = emulatorLoadRom(romdir, romim, QL_ROM_PORT_BASE,
				      QL_ROM_PORT_SIZE);
		if (ret < 0) {
			fprintf(stderr, "Error Loading romim %s\n", romim);
			exit(ret);
		}
	}

	if (!strcasecmp(mouse, "ICE")) {
		ice_init();
		ice_check_rom((strlen(romport) || strlen(romim)) ?
				      (uint8_t *)memBase + QL_ROM_PORT_BASE : NULL,
			      QL_ROM_PORT_SIZE);
		printf(">>> [ICE] Mouse at $FFFE/$FFFF\n");
	} else if (!strcasecmp(mouse, "THOR")) {
		// The I/O block is the last 8K of the first megabyte: the RAM
		// must not reach it
		if (RTOP >= THOR_IO_BASE) {
			fprintf(stderr,
				"SQLUX WARNING: MOUSE = THOR needs RAMTOP below %d (it is %d), mouse disabled\n",
				THOR_IO_BASE / 1024, RTOP / 1024);
		} else {
			thor_init();
			printf(">>> [THOR] Mouse PIA at $FE040-$FE04F (and every $100 up to $FFF40)\n");
		}
	} else if (!strcasecmp(mouse, "QIMI")) {
		qimi_init();
		printf(">>> [QIMI] Mouse registers at $1BF9C, $1BFBC and $1BFBE\n");
	} else if (strlen(mouse) && strcasecmp(mouse, "QIMSI")) {
		fprintf(stderr, "SQLUX WARNING: unknown MOUSE type %s\n", mouse);
	}

	if (strlen(iorom1)) {
		ret = emulatorLoadRom(romdir, iorom1, QL_ROM2_BASE,
				      QL_ROM2_SIZE);
		if (ret < 0) {
			fprintf(stderr, "Error Loading iorom1 %s\n", iorom1);
			exit(ret);
		}
	}

	if (strlen(iorom2)) {
		ret = emulatorLoadRom(romdir, iorom2, QL_ROM3_BASE,
				      QL_ROM3_SIZE);
		if (ret < 0) {
			fprintf(stderr, "Error Loading iorom2 %s\n", iorom2);
			exit(ret);
		}
	}

	init_uqlx_tz();

	init_iso();

	/* CPU timing model: 68008 (original QL) or 68000 */
	if (emulatorOptionInt("cpu_timing") == 68000) {
		cycles_init(CYC_CPU_68000);
		printf("CPU timing: 68000 (16 bit bus)\n");
	} else {
		cycles_init(CYC_CPU_68008);
	}
	/* ZX8301 RAM contention: only with the original 68008 */
	zx8301_init(emulatorOptionInt("zx8301_contention") &&
		    emulatorOptionInt("cpu_timing") != 68000,
		    emulatorOptionInt("ntsc"),
		    emulatorOptionInt("zx8301_vsync_lines"));
	mdv_init();

	/* Low level IPC: the 8049 with its original firmware */
	ipc_lle_init(emulatorOptionString("ipc_rom"));

	const char *mdv1_path = emulatorOptionString("mdv1");
	if (strlen(mdv1_path)) {
		mdv_load(0, mdv1_path);
		if (emulatorOptionInt("mdv_reverse")) {
			mdv_set_reverse(0, true);
		}
	}

	const char *mdv2_path = emulatorOptionString("mdv2");
	if (strlen(mdv2_path)) {
		mdv_load(1, mdv2_path);
		if (emulatorOptionInt("mdv_reverse")) {
			mdv_set_reverse(1, true);
		}
	}

	LoadMainRom(); /* patch QDOS ROM*/

	/* Minerva cannot handle more than 16M of memory... */
	if (isMinerva && RTOP > 16384 * 1024)
		RTOP = 16384 * 1024;
	/* ...everything else not more than 4M */
	if (!isMinerva && RTOP > 4096 * 1024)
		RTOP = 4096 * 1024;
	mem_limit = mem_limit_for(RTOP);

	// If QSROM is defined in sqlux.ini, the QSound card is fitted: a 16K
	// block with the 8K ROM followed by the registers ($2000 and $3000
	// from its base). With RAM up to $0C0000 it sits at $0C0000. With more
	// RAM it goes in the first free 16K block between RTOP and mem_limit
	// (the reserved area above the RAM), and the card is disabled if there
	// is none.
	if (emulatorOptionInt("qliont_ay")) {
		// Same chip as QSound, without ROM, at $80002A/$800028
		if (RTOP > QLIONT_AY_BASE) {
			fprintf(stderr,
				"SQLUX WARNING: QLIONT_AY disabled, its address $%06X overlaps the RAM (RAMTOP %dK)\n",
				QLIONT_AY_BASE, RTOP / 1024);
		} else {
			qliont_ay_mapped = 1;
			printf(">>> [QLIONT_AY] AY-3-8910 at $80002A (control) and $800028 (data)\n");
			if (strlen(qsrom))
				fprintf(stderr,
					"SQLUX WARNING: QLIONT_AY and QSROM are mutually exclusive, QSROM disabled (QLIONT_AY has preference)\n");
		}
	}
	if (strlen(qsrom) && !qliont_ay_mapped) {
		uint32_t base = 0x000C0000;
		int ok = 1;

		if (RTOP > 0x000C0000) {
			base = (RTOP + 0x3FFF) & ~0x3FFFu;
			if (base + 0x4000 > mem_limit) {
				fprintf(stderr,
					"SQLUX WARNING: QSound disabled, no free 16K block between RAMTOP %dK and the end of the reserved memory (%dK)\n",
					RTOP / 1024, mem_limit / 1024);
				ok = 0;
			} else {
				fprintf(stderr,
					"SQLUX WARNING: QSound moved to address $%06X, as the RAM (RAMTOP %dK) overlaps $0C0000\n",
					base, RTOP / 1024);
			}
		}
		if (ok) {
			ret = emulatorLoadRom(romdir, qsrom, base, 8192);
			if (ret < 0) {
				fprintf(stderr, "Error Loading qsrom %s\n",
					qsrom);
				exit(ret);
			}
			qsound_base = base;
			qsound_mapped = 1;
			printf(">>> [QSOUND] Loading QSound ROM '%s' allocate at $%06X\n",
			       qsrom, base);
		}
	}

	if (isMinerva) {
		qlscreen.xres = qlscreen.xres & (~(7));
		qlscreen.linel = qlscreen.xres / 4;
		qlscreen.qm_len = qlscreen.linel * qlscreen.yres;

		qlscreen.qm_lo = 128 * 1024;
		qlscreen.qm_hi = 128 * 1024 + qlscreen.qm_len;
		if (qlscreen.qm_len > 0x8000) {
			if (((long)RTOP - qlscreen.qm_len) <
			    256 * 1024 + 8192) {
				/*RTOP+=qlscreen.qm_len;*/
				printf("sorry, not enough RAM for such a big screen\n");
				goto bsfb;
			}
			qlscreen.qm_lo = ((RTOP - qlscreen.qm_len) >> 15)
					 << 15; /* RTOP MUST BE 32K aligned.. */
			qlscreen.qm_hi = qlscreen.qm_lo + qlscreen.qm_len;
			RTOP = qlscreen.qm_lo;
		}
	} else /* JS doesn't handle big screen */
	{
	bsfb:
		qlscreen.linel = 128;
		qlscreen.yres = 256;
		qlscreen.xres = 512;

		qlscreen.qm_lo = 128 * 1024;
		qlscreen.qm_hi = 128 * 1024 + 32 * 1024;
		qlscreen.qm_len = 0x8000;
	}

	if (V1 && (atof(emulatorOptionString("speed")) > 0.0))
		printf("Emulation Speed: %s\n", emulatorOptionString("speed"));
	else if (V1)
		printf("Emulation Speed: FULL\n");

	if (V1 && (emulatorOptionInt("sound") > 0))
		printf("sound enabled, volume %i.\n",
		       emulatorOptionInt("sound"));

	if (!isMinerva) {
		qlux_table[IPC_CMD_CODE] = UseIPC; /* install pseudoops */
		qlux_table[IPCR_CMD_CODE] = ReadIPC;
		qlux_table[IPCW_CMD_CODE] = WriteIPC;
		qlux_table[KEYTRANS_CMD_CODE] = QL_KeyTrans;

		qlux_table[FSTART_CMD_CODE] = FastStartup;
	}
	qlux_table[ROMINIT_CMD_CODE] = InitROM;
	qlux_table[MDVIO_CMD_CODE] = MdvIO;
	qlux_table[MDVO_CMD_CODE] = MdvOpen;
	qlux_table[MDVC_CMD_CODE] = MdvClose;
	qlux_table[MDVSL_CMD_CODE] = MdvSlaving;
	qlux_table[MDVFO_CMD_CODE] = MdvFormat;
	qlux_table[POLL_CMD_CODE] = PollCmd;

#ifdef SERIAL
#ifndef NEWSERIAL
	qlux_table[OSERIO_CMD_CODE] = SerIO;
	qlux_table[OSERO_CMD_CODE] = SerOpen;
	qlux_table[OSERC_CMD_CODE] = SerClose;
#endif
#endif

	qlux_table[SCHEDULER_CMD_CODE] = SchedulerCmd;
	if (isMinerva) {
		qlux_table[MIPC_CMD_CODE] = KbdCmd;
		qlux_table[KBENC_CMD_CODE] = KBencCmd;
	}
	qlux_table[BASEXT_CMD_CODE] = BASEXTCmd;

	if (emulatorOptionInt("skip_boot"))
		qlux_table[0x4e43] = btrap3;

	InitialSetup();

	if (isMinerva) {
		reg[1] = (RTOP & ~16383) | 1;
		SetPC(0x186);
	}

	QLdone = 0;
}
