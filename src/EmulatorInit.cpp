#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>

extern "C" {
    #include "debug.h"
    #include "QInstAddr.h"
    #include "QLtraps.h"
    #include "QL_cconv.h"
    #include "QL_config.h"
    #include "QL_hardware.h"
    #include "QL_screen.h"
    #include "QL_sound.h"
    #include "SDL2screen.h"
    #include "unixstuff.h"
    #include "uqlx_cfg.h"
    #include "version.h"
    #include "xcodes.h"
    #include "xqlmouse.h"

    // Temp here until full migration
    extern uint32_t *memBase;
    extern int32_t RTOP;
    extern int isMinerva;
    extern void (**qlux_table)(void);
    extern void InitialSetup(void);
    extern uint32_t reg[16];
    extern void SetPC(uint32_t);
}

namespace emulator
{
using namespace std;

void loadRom(string name, uint32_t addr, int size)
{
    std::filesystem::path p{name};
    if (std::filesystem::file_size(p) != size) {
        throw std::length_error("Rom Size Error");
    }

    ifstream romFile(name.c_str(), std::ios::binary);
    if (romFile.bad()) {
        throw std::runtime_error("File NOT Found");
    }
    romFile.read((char *)memBase + addr, size);
    romFile.close();
}

void init()
{
	char *rf;
	int rl = 0;
	void *tbuff;

	if (V1)
		cout << "*** sQLux release" << release << "\n\n";

	tzset();

	memBase = (uint32_t *)malloc(RTOP);
	if (memBase == NULL) {
        cout << "sorry, not enough memory for a " << RTOP/1024 << "K QL\n";
		exit(1);
	}

	if (EmulatorTable()) {
		cout << "Failed to allocate instruction table\n";
		free(memBase);
		exit(1);
	}

    try {
        loadRom(std::string(QMD.romdir) + "/" + QMD.sysrom, 0, 49152);
    }
    catch (const exception &e)
    {
        cout << "Error Loading ROM " << QMD.sysrom << " reason: " << e.what() << "\n";
        exit(1);
    }
	if (strlen(QMD.romim)) {
        try {
            loadRom(std::string(QMD.romdir) + "/" + QMD.romim, 49152, 16384);
        }
        catch(const exception &e)
        {
            cout << "Error Loading ROM " << QMD.romim << "reason: " << e.what() << "\n";
            exit(1);
        }
	}

	init_uqlx_tz();

	init_iso();

	LoadMainRom(); /* patch QDOS ROM*/

	/* Minerva cannot handle more than 16M of memory... */
	if (isMinerva && RTOP > 16384 * 1024)
		RTOP = 16384 * 1024;
	/* ...everything else not more than 4M */
	if (!isMinerva && RTOP > 4096 * 1024)
		RTOP = 4096 * 1024;

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

	if (V1 && (QMD.speed > 0))
		printf("Speed %.1f\n", QMD.speed);

	sound_enabled = (QMD.sound > 0);
	if (V1 && (QMD.sound > 0))
		printf("sound enabled, volume %i.\n", QMD.sound);

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

	if (QMD.skip_boot)
		qlux_table[0x4e43] = btrap3;

	InitialSetup();

	if (isMinerva) {
		reg[1] = (RTOP & ~16383) | 1;
		SetPC(0x186);
	}

	QLdone = 0;
}

} // namespace emulator