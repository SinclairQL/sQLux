#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "args.h"
#include "emudisk.h"
#include "ini.h"
#include "sds.h"
#include "unixstuff.h"
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
{"ramtop", "r", "The memory space top (128K + QL ram, not valid if ramsize set)", EMU_OPT_INT, 256, NULL},
{"ramsize", "", "The size of ram", EMU_OPT_INT, 0, NULL},
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

static void toupperStr(sds lower)
{
	int i;

	for (i = 0; i < sdslen(lower); i++) {
		lower[i] = toupper(lower[i]);
	}
}

static sds replaceX(sds fileString) {
	if (strstr(fileString, "%x")) {
		sds *split;
		int count;
		sds pidStr;

		split = sdssplitlen(fileString, sdslen(fileString), "%x", 2, &count);

		if (count > 2) {
			fprintf(stderr, "WARNING: Only one %%x allowed\n");
		}

		sdsfree(fileString);
		fileString = sdsnew("");

		fileString = sdscatprintf(fileString, "%s%x%s", split[0], getpid(), split[1]);

		sdsfreesplitres(split, count);
	}

	return fileString;
}

static bool fileExists(const sds fileString)
{
	struct stat statBuf;
	int ret;

	ret = stat(fileString, &statBuf);
	if (ret < 0) {
		return false;
	}
	return true;
}

static bool isDirectory(const sds fileString)
{
	struct stat statBuf;
	int ret;

	ret = stat(fileString, &statBuf);
	if (ret < 0) {
		return false;
	}
	return S_ISDIR(statBuf.st_mode);
}

void deviceInstall(sds *device, int count)
{
	/*DIR *dirp;*/
	short ndev = 0;
	short len = 0;
	short idev = -1;
	short lfree = -1;
	short i;
	char tmp[401];
	int err;

	struct stat sbuf;

	if (isdigit(device[0][sdslen(device[0]) - 1])) {
		ndev = device[0][sdslen(device[0]) - 1] - '0';
		sdsrange(device[0], 0, -2);
	} else {
		ndev = -1;
	}

	for (i = 0; i < MAXDEV; i++) {
		if (qdevs[i].qname && (match(qdevs[i].qname, device[0]))) {
			idev = i;
			break;
		} else if (qdevs[i].qname == NULL && lfree == -1) {
			lfree = i;
		}
	}

	if (idev == -1 && lfree == -1) {
		fprintf(stderr,
			"sorry, no more free entries in Directory Device Driver table\n");
		fprintf(stderr,
			"check your sqlux.ini if you really need all this devices\n");

		return;
	}

	if (idev != -1 && ndev == 0) {
		memset((qdevs + idev), 0, sizeof(EMUDEV_t));
	} else {
		if (lfree != -1) {
			idev = lfree;
			toupperStr(device[0]);
			qdevs[idev].qname = strdup(device[0]);
		}
		if (ndev && ndev < 9) {
			if (count > 1) {
				sds fileString;

				if (device[1][0] == '~') {
					fileString =
						sdscatprintf(sdsnew(""),
							     "%s/%s", homedir,
							     device[1] + 1);
				} else {
					fileString = sdsnew(device[1]);
				}

				fileString = replaceX(fileString);

				// check file/dir exists unless its a ramdisk
				if (!match("ram", qdevs[idev].qname)) {
					if (!fileExists(fileString)) {
						fprintf(stderr,
							"Mountpoint %s for device %s%d_ may not be accessible\n",
							fileString, device[0],
							ndev);
					}
				}

				if (isDirectory(fileString) &&
				    (fileString[sdslen(fileString) - 1] !=
				     '/')) {
					fileString = sdscat(fileString, "/");
				}

				// ram devices need to end in /
				if (match("ram", qdevs[idev].qname) &&
				    (fileString[sdslen(fileString) - 1] !=
				     '/')) {
					fileString = sdscat(fileString, "/");
				}

				qdevs[idev].mountPoints[ndev - 1] =
					strdup(fileString);
				qdevs[idev].Present[ndev - 1] = 1;
			} else {
				qdevs[idev].Present[ndev - 1] = 0;
			}

			if (count > 2) {
				int flag_set = 0;

				for (int i = 2; i < count; i++) {
					if (strstr(device[i], "native") ||
					    strstr(device[i], "qdos-fs")) {
						flag_set |=
							qdevs[idev].Where[ndev -
									  1] =
								1;
					} else if (strstr(device[i],
							  "qdos-like")) {
						flag_set |=
							qdevs[idev].Where[ndev -
									  1] =
								2;
					}

					if (strstr(device[i], "clean")) {
						flag_set |=
							qdevs[idev].clean[ndev -
									  1] =
								1;
					}
				}

				if (!flag_set) {
					fprintf(stderr,
						"WARNING: flag %s in definition of %s%d_ not recognised",
						device[i], device[0], ndev);
				}
			}
		}
	}
}

static int iniHandler(void* user, const char* section, const char* name,
                   const char* value)
{
	int i;

	if (match(name, "device")) {
		int count;
		sds *splitDevice = sdssplitlen(value, strlen(value), ",", 1, &count);
		deviceInstall(splitDevice, count);
		sdsfreesplitres(splitDevice, count);
		return 0;
	}

	i = 0;
	while (emuOptions[i].option != NULL) {
		if (match(name, emuOptions[i].option)) {
			if (emuOptions[i].type == EMU_OPT_CHAR) {
				emuOptions[i].charVal = strdup(value);
				return 0;
			} else if (emuOptions[i].type == EMU_OPT_INT) {
				emuOptions[i].intVal = atoi(value);
				return 0;
			}

			return 1;
		}

		i++;
	}

	return 1;
}

int emulatorOptionParse(int argc, char **argv)
{
	int i;
	sds helptext = sdsnew(helpTextInit);
	const char *configFile;

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

	ap_add_str_opt(parser, "config f", "sqlux.ini");

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

	for (i = 0; i < ap_count(parser, "device"); i++) {
		int count;
		const char *device = ap_get_str_value_at_index(parser, "device", i);
		sds *splitDevice = sdssplitlen(device, strlen(device), ",", 1, &count);
		deviceInstall(splitDevice, count);
		sdsfreesplitres(splitDevice, count);
	}
	configFile = ap_get_str_value(parser, "config");

	if (ini_parse(configFile, iniHandler, NULL) < 0) {
		fprintf(stderr, "Can't load '%s'\n", configFile);
		return 1;
	}

	return 0;
}

void emulatorOptionsRemove()
{
	ap_free(parser);
}

char *emulatorOptionString(const char *name)
{
	int i;

	if (ap_count(parser, name)) {
		return ap_get_str_value(parser, name);
	}

	i = 0;
	while (emuOptions[i].option != NULL) {
		if ((strcmp(emuOptions[i].option, name) == 0)
			&& (emuOptions[i].type == EMU_OPT_CHAR)
			&& (emuOptions[i].charVal != NULL)) {
			return emuOptions[i].charVal;
		}
		i++;
	}

	return "";
}

int emulatorOptionInt(const char *name)
{
	int i;

	if (ap_count(parser, name)) {
		return ap_get_int_value(parser, name);
	}

	i = 0;
	while (emuOptions[i].option != NULL) {
		if (strcmp(emuOptions[i].option, name) == 0) {
			return emuOptions[i].intVal;
		}
		i++;
	}

	return 0;
}

int emulatorOptionArgc()
{
	return ap_count_args(parser);
}

char *emulatorOptionArgv(int idx)
{
	return ap_get_arg_at_index(parser, idx);
}
