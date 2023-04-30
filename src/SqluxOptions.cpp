#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unistd.h>

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"
#include "iequals.hpp"


extern "C" {
    #include <sys/stat.h>

    #include "emudisk.h"
    #include "unixstuff.h"
}

namespace emulator
{
CLI::App sqluxOpt("sQLux Options Parser");

using namespace std;

static string argv0;

std::vector<std::string> argvRemaining;

std::vector<std::string> stringSplit(const std::string str, const std::string regex_str)
{
    std::regex regexz(regex_str);
    std::vector<std::string> list(std::sregex_token_iterator(str.begin(), str.end(), regexz, -1),
                                  std::sregex_token_iterator());
    return list;}

void deviceInstall(std::vector<string> device)
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

	if (isdigit(device[0].back())) {
		ndev = device[0].back() - '0';
        device[0].pop_back();
	} else {
		ndev = -1;
	}

	for (i = 0; i < MAXDEV; i++) {
		if (qdevs[i].qname &&
            (iequals(qdevs[i].qname, device[0]))) {
			idev = i;
			break;
		} else if (qdevs[i].qname == NULL && lfree == -1) {
			lfree = i;
		}
	}

	if (idev == -1 && lfree == -1) {
		printf("sorry, no more free entries in Directory Device Driver table\n");
		printf("check your sqlux.ini if you really need all this devices\n");

		return;
	}

	if (idev != -1 && ndev == 0) {
		memset((qdevs + idev), 0, sizeof(EMUDEV_t));
	} else {
		if (lfree != -1) {
			idev = lfree;
            string upper = device[0].substr(0, 3);
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            //boost::to_upper(upper);
			qdevs[idev].qname = strdup(upper.c_str());
		}
		if (ndev && ndev < 9) {
			if (device.size() > 1) {
                string fileString = device[1];

				if (fileString.front() == '~') {
                    fileString.erase(0, 1);
                    fileString.insert(0, "/");
                    fileString.insert(0, homedir);
				}

                auto hex = fileString.find("%x");
                if (hex != string::npos) {
                    char tbuf[17];
                    snprintf(tbuf, 17, "%x", getpid());
                    fileString.erase(hex, 2);
                    fileString.insert(hex, tbuf);
                }

                std::filesystem::path p{fileString};

                // check file/dir exists unless its a ramdisk
                if (!iequals("ram", qdevs[idev].qname)) {
                    if (!std::filesystem::exists(p)) {
                        cerr << "Mountpoint " << fileString << " for device " << device[0] << ndev << "_ may not be accessible\n";
                    }
                }

                // if its a directory make sure it ends in a slash
                if (std::filesystem::is_directory(p) && (fileString.back() != '/')) {
                    fileString.append("/");
                }

                // ram devices need to end in /
                if (iequals("ram", qdevs[idev].qname) && (fileString.back() != '/')) {
                    fileString.append("/");
                }

                qdevs[idev].mountPoints[ndev - 1] = strdup(fileString.c_str());
                qdevs[idev].Present[ndev - 1] = 1;
			} else {
				qdevs[idev].Present[ndev - 1] = 0;
            }

			if (device.size() > 2) {
				int flag_set = 0;

                for (int i = 2; i < device.size(); i++) {
                    if ((device[i].find("native") != string::npos) ||
                            (device[i].find("qdos-fs") != string::npos)) {
					    flag_set |= qdevs[idev].Where[ndev - 1] = 1;
                    } else if (device[i].find("qdos-like") != string::npos) {
					    flag_set |= qdevs[idev].Where[ndev - 1] = 2;
                    }

                    if (device[i].find("clean") != string::npos) {
				        flag_set |= qdevs[idev].clean[ndev - 1] = 1;
                    }
                }

				if (!flag_set) {
					cout << "WARNING: flag " << device[i] << " in definition of " << device[0]
                        << ndev << "_ not recognised\n";
                }
			}
		}
	}
}

void deviceParse()
{
    try {
        if (sqluxOpt.get_option("--DEVICE")->count()) {
            std::vector<std::string> devices = sqluxOpt.get_option("--DEVICE")->as<std::vector<std::string>>();
            for (auto devString : devices) {
                auto device = stringSplit(devString, ",");
                deviceInstall(device);
            }
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

int optionParse(int argc, char *argv[])
{
    argv0 = string(argv[0]);

    string home(homedir);
    vector<string> config_files= {"sqlux.ini", home + "/.sqluxrc", home + "/.uqlxrc"};
    std::string configFile;

    for (auto &file: config_files) {
        if (filesystem::exists(file)) {
            cout << "Loading Config from: " << file <<"\n";
            configFile = file;
        }
    }

    try {
        auto config_base=sqluxOpt.get_config_formatter_base();
        config_base->arrayDelimiter(':');

        sqluxOpt.option_defaults()->ignore_case()->configurable();
        sqluxOpt.set_config("--CONFIG,-f", "sqlux.ini configuration file")->default_str(configFile);
        sqluxOpt.add_option("--BDI1", "file exposed by the BDI interface");
        sqluxOpt.add_option("--BOOT_CMD,-b", "command to run on boot (executed in basic)");
        sqluxOpt.add_option("--BOOT_DEVICE,-d", "device to load BOOT file from")->default_str("mdv1");
        sqluxOpt.add_option("--CPU_HOG", "1 = use all cpu, 0 = sleep when idle")->default_str("1");
        sqluxOpt.add_option("--DEVICE", "QDOS_name,path,flags (may be used multiple times")->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);
        sqluxOpt.add_option("--FAST_STARTUP", "1 = skip ram test (does not affect Minerva)")->default_str("0");
        sqluxOpt.add_option("--FILTER", "enable bilinear filter when zooming")->default_str("0");
        sqluxOpt.add_option("--FIXASPECT", "0 = 1:1 pixel mapping, 1 = 2:3 non square pixels, 2 = BBQL aspect non square pixels")->default_str("0");
        sqluxOpt.add_option("--IOROM1", "rom in 1st IO area (Minerva only 0x10000 address)");
        sqluxOpt.add_option("--IOROM2", "rom in 2nd IO area (Minerva only 0x14000 address)");
        sqluxOpt.add_option("--JOY1", "1-8 SDL2 joystick index");
        sqluxOpt.add_option("--JOY2", "1-8 SDL2 joystick index");
        sqluxOpt.add_option("--KBD", "keyboard language DE, GB, US")->default_str("US");
        sqluxOpt.add_option("--NO_PATCH,-n", "disable patching the rom")->default_str("0");
        sqluxOpt.add_option("--PALETTE", "0 = Full colour, 1 = Unsaturated colours (slightly more CRT like), 2 =  Enable grayscale display")->default_str("0");
        sqluxOpt.add_option("--PRINT", "command to use for print jobs")->default_str("lpr");
        sqluxOpt.add_option("--RAMTOP,-r", "The memory space top (128K + QL ram, not valid if ramsize set)");
        sqluxOpt.add_option("--RAMSIZE", "The size of ram");
        sqluxOpt.add_option("--RESOLUTION,-g", "resolution of screen in mode 4")->default_str("512X256");
        sqluxOpt.add_option("--ROMDIR", "path to the roms")->default_str("roms");
        sqluxOpt.add_option("--ROMPORT", "rom in QL rom port (0xC000 address)");
        sqluxOpt.add_option("--ROMIM", "rom in QL rom port (0xC000 address, legacy alias for romport)");
        sqluxOpt.add_option("--SER1", "device for ser1");
        sqluxOpt.add_option("--SER2", "device for ser2");
        sqluxOpt.add_option("--SER3", "device for ser3");
        sqluxOpt.add_option("--SER4", "device for ser4");
        sqluxOpt.add_option("--SHADER", "0 = Disabled, 1 = Use flat shader, 2 = Use curved shader")->default_str("0");
        sqluxOpt.add_option("--SHADER_FILE", "Path to shader file to use if SHADER is 1 or 2")->default_str("shader.glsl");
        sqluxOpt.add_option("--SKIP_BOOT", "1 = skip f1/f2 screen, 0 = show f1/f2 screen")->default_str("1");
        sqluxOpt.add_option("--SOUND", "volume in range 1-8, 0 to disable")->default_str("0");
        sqluxOpt.add_option("--SPEED", "speed in factor of BBQL speed, 0.0 for full speed")->default_str("0.0");
        sqluxOpt.add_option("--STRICT_LOCK", "enable strict file locking")->default_str("0");
        sqluxOpt.add_option("--SYSROM", "system rom")->default_str("MIN198.rom");
        sqluxOpt.add_option("--WIN_SIZE,-w", "window size 1x, 2x, 3x, max, full")->default_str("1x");
        sqluxOpt.add_option("--VERBOSE,-v", "verbosity level 0-3")->default_str("1");
        sqluxOpt.add_option("args", argvRemaining, "Arguments passed to QDOS");
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }
    CLI11_PARSE(sqluxOpt, argc, argv);

    return 1;
}

} // namespace emulator

extern "C" {

float optionFloat(char *optionName)
{
    try {
        return emulator::sqluxOpt.get_option(std::string("--") + std::string(optionName))->as<float>();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0.0;
}

int optionArgc()
{
    try {
        return emulator::argvRemaining.size();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}

const char *optionArgv(int index)
{
    if (index == 0) {
        return emulator::argv0.c_str();
    }

    index--;
    if(index < emulator::argvRemaining.size()) {
        return emulator::argvRemaining[index].c_str();
    }

    return "";
}

}
