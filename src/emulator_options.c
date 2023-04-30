#include <stdio.h>

#include "args.h"
#include "sds.h"
#include "version.h"


static const char * const helpTextInit = "\n\
Usage: sqlux [OPTIONS] [args...]\n\
\n\
Positionals:\n\
  args                        Arguments passed to QDOS\n\
\n\
Options:\n\
  -h,--help                   Print this help message and exit\n\
  -f,--CONFIG [sqlux.ini]     Read an ini file\n";

static const char * const helpTextTail = "\
  --version                   version number\n";


enum emuOptsType {
	EMU_OPT_INT,
	EMU_OPT_CHAR,
	EMU_OPT_DEV
};

struct emuOpts {
	char *option;
	char *alias;
	char *help;
	int type;
	int intVal;
	char *charVal;
};

struct emuOpts emuOptions[] = {
{"bdi1", "", "file exposed by the BDI interface", EMU_OPT_CHAR, 0 , NULL},
{"boot_cmd", "b", "command to run on boot (executed in basic)", EMU_OPT_CHAR, 0, NULL},
{"boot_device", "d", "device to load BOOT file from", EMU_OPT_CHAR, 0, "mdv1"},
{"cpu_hog", "", "1 = use all cpu, 0 = sleep when idle", EMU_OPT_INT, 1, NULL},
{"device", "", "QDOS_name,path,flags (may be used multiple times", EMU_OPT_DEV, 0, NULL},
{"fast_startup", "", "1 = skip ram test (does not affect Minerva)", EMU_OPT_INT, 0, NULL},
{"filter", "", "enable bilinear filter when zooming", EMU_OPT_INT, 0, NULL},
{"fixaspect", "", "0 = 1:1 pixel mapping, 1 = 2:3 non square pixels, 2 = BBQL aspect non square pixels", EMU_OPT_INT, 0, NULL},
{"iorom1", "", "rom in 1st IO area (Minerva only 0x10000 address)", EMU_OPT_CHAR, 0, NULL},
{"iorom2", "", "rom in 2nd IO area (Minerva only 0x14000 address)", EMU_OPT_CHAR, 0, NULL},
{"joy1", "", "1-8 SDL2 joystick index", EMU_OPT_INT, 0, NULL},
{"joy2", "", "1-8 SDL2 joystick index", EMU_OPT_INT, 0, NULL},
{"kbd", "", "keyboard language DE, GB, US", EMU_OPT_CHAR, 0, "US"},
{"no_patch", "n", "disable patching the rom", EMU_OPT_INT, 0, NULL},
{"palette", "", "0 = Full colour, 1 = Unsaturated colours (slightly more CRT like), 2 =  Enable grayscale display", EMU_OPT_INT, 0, NULL},
{"print", "", "command to use for print jobs", EMU_OPT_CHAR, 0, "lpr"},
{"ramtop", "r", "The memory space top (128K + QL ram, not valid if ramsize set)", EMU_OPT_INT, 0, NULL},
{"ramsize", "", "The size of ram", EMU_OPT_INT, 128, NULL},
{"resolution", "g", "resolution of screen in mode 4", EMU_OPT_CHAR, 0, "512x256"},
{"romdir", "", "path to the roms", EMU_OPT_CHAR, 0, "roms"},
{"romport", "", "rom in QL rom port (0xC000 address)", EMU_OPT_CHAR, 0, NULL},
{"romim", "", "rom in QL rom port (0xC000 address, legacy alias for romport)", EMU_OPT_CHAR, 0, NULL},
{"ser1", "", "device for ser1", EMU_OPT_CHAR, 0, NULL},
{"ser2", "", "device for ser2", EMU_OPT_CHAR, 0, NULL},
{"ser3", "", "device for ser3", EMU_OPT_CHAR, 0, NULL},
{"ser4", "", "device for ser4", EMU_OPT_CHAR, 0, NULL},
{"shader", "", "0 = Disabled, 1 = Use flat shader, 2 = Use curved shader", EMU_OPT_INT, 0, NULL},
{"shader_file", "", "Path to shader file to use if SHADER is 1 or 2", EMU_OPT_CHAR, 0, "shader.glsl"},
{"skip_boot", "", "1 = skip f1/f2 screen, 0 = show f1/f2 screen", EMU_OPT_INT, 1, NULL},
{"sound", "", "volume in range 1-8, 0 to disable", EMU_OPT_INT, 0, NULL},
{"speed", "", "speed in factor of BBQL speed, 0.0 for full speed", EMU_OPT_CHAR, 0, "0.0"},
{"strict_lock", "", "enable strict file locking", EMU_OPT_INT, 0, NULL},
{"sysrom", "", "system rom", EMU_OPT_CHAR, 0, "MIN198.rom"},
{"win_size", "w", "window size 1x, 2x, 3x, max, full", EMU_OPT_CHAR, 0, "1x"},
{"verbose", "v", "verbosity level 0-3", EMU_OPT_INT, 1, NULL},
{NULL},
};

static ArgParser* parser;

static bool match(const char *name, const char *option)
{
	return (strcasecmp(name, option) == 0);
}

int emulatorOptionParse(int argc, char **argv)
{
	int i;
	sds helptext = sdsnew(helpTextInit);

	parser = ap_new_parser();
	if (!parser) {
		exit(1);
	}

	i = 0;
	while (emuOptions[i].option != NULL) {
		int j;
		sds helpItem = sdsempty();

		if(strlen(emuOptions[i].alias)) {
			helpItem = sdscatprintf(helpItem, "  -%s,", emuOptions[i].alias);
		} else {
			helpItem = sdscatprintf(helpItem, "  ");
		}
		helpItem = sdscatprintf(helpItem, "--%s", emuOptions[i].option);

		if (emuOptions[i].type == EMU_OPT_INT) {
			helpItem = sdscatprintf(helpItem, " [%d]", emuOptions[i].intVal);
		} else if (emuOptions[i].type == EMU_OPT_CHAR) {
			if (emuOptions[i].charVal != NULL) {
				helpItem = sdscatprintf(helpItem, " [%s]", emuOptions[i].charVal);
			}
		}
		for (j = sdslen(helpItem); j < 30; j++){
			helpItem = sdscat(helpItem, " ");
		}

		helpItem = sdscatprintf(helpItem, "%s\n", emuOptions[i].help);

		helptext = sdscatsds(helptext, helpItem);

		sdsfree(helpItem);

		i++;
	}
	helptext = sdscat(helptext, helpTextTail);

	ap_set_helptext(parser, helptext);
	ap_set_version(parser, release);

	sdsfree(helptext);

	i = 0;
	while (emuOptions[i].option != NULL) {
		sds optItem = sdsnew("");

		if (strlen(emuOptions[i].alias)) {
			optItem = sdscatprintf(optItem, "%s %s",
				emuOptions[i].option,
				emuOptions[i].alias);
		} else {
			optItem = sdscat(optItem, emuOptions[i].option);
		}

		if (emuOptions[i].type == EMU_OPT_INT) {
			ap_add_int_opt(parser, optItem, 0);
		} else {
			ap_add_str_opt(parser, optItem, "");
		}

		sdsfree(optItem);

		i++;
	}

	if (!ap_parse(parser, argc, argv)) {
		exit(1);
	}

	printf("SER2 Count %d\n", ap_count(parser, "ser2"));

	return 0;
}
