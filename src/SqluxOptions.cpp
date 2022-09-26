#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/process/environment.hpp>

extern "C" {
    #include <sys/stat.h>

    #include "emudisk.h"
    #include "unixstuff.h"
}

namespace emulator
{
using namespace std;
namespace po = boost::program_options;
namespace po_style = boost::program_options::command_line_style;
namespace pid = boost::this_process;

static po::variables_map vm;

static string argv0;

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
            (boost::iequals(qdevs[i].qname, device[0]))) {
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
            boost::to_upper(upper);
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
                    snprintf(tbuf, 17, "%x", pid::get_id());
                    fileString.erase(hex, 2);
                    fileString.insert(hex, tbuf);
                }

                std::filesystem::path p{fileString};

                // check file/dir exists unless its a ramdisk
                if (!boost::iequals("ram", qdevs[idev].qname)) {
                    if (!std::filesystem::exists(p)) {
                        cerr << "Mountpoint " << fileString << " for device " << device[0] << ndev << "_ may not be accessible\n";
                    }
                }

                // if its a directory make sure it ends in a slash
                if (std::filesystem::is_directory(p) && (fileString.back() != '/')) {
                    fileString.append("/");
                }

                // ram devices need to end in /
                if (boost::iequals("ram", qdevs[idev].qname) && (fileString.back() != '/')) {
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
    if (!emulator::vm.count("DEVICE")) {
        return;
    }

    for (auto &devString : vm["DEVICE"].as< vector<string> >()) {
        auto device = stringSplit(devString, ",");
        deviceInstall(device);
    }
}

int optionParse(int argc, char *argv[])
{
    argv0 = string(argv[0]);

    try {
        po::options_description generic("Generic options");
        generic.add_options()
        ("CONFIG,f", po::value<string>(),
                "sqlux.ini configuration file.")
        ("HELP,h",
            "produce help message")
        ("VERBOSE,v", po::value<int>()->default_value(1),
            "verbosity level 0-3")
            ;

        po::options_description config("Configuration");
        config.add_options()
            ("BDI1", po::value<string>(),
                "file exposed by the BDI interface")
            ("BOOT_DEVICE,d", po::value<string>()->default_value("MDV1"),
                "device to load BOOT file from")
            ("CPU_HOG", po::value<int>()->default_value(1),
                "1 = use all cpu, 0 = sleep when idle")
            ("DEVICE", po::value< vector<string> >(),
                "QDOS_name,path,flags (may be used multiple times")
            ("FAST_STARTUP", po::value<int>()->default_value(0),
                "1 = skip ram test (does not affect Minerva)")
            ("FILTER", po::value<int>()->default_value(0),
                "enable bilinear filter when zooming")
            ("FIXASPECT", po::value<int>()->default_value(0),
                "0 = 1:1 pixel mapping, 1 = 2:3 non square pixels, 2 = BBQL aspect non square pixels")
            ("IOROM1", po::value<string>(),
                "rom in 1st IO area (Minerva only 0x10000 address)")
            ("IOROM2", po::value<string>(),
                "rom in 2nd IO area (Minerva only 0x14000 address)")
            ("JOY1", po::value<int>(),
                "1-8 SDL2 joystick index")
            ("JOY2", po::value<int>(),
                "1-8 SDL2 joystick index")
            ("KBD", po::value<string>()->default_value("US"),
                "keyboard language DE, GB, US")
            ("NO_PATCH,n", po::value<int>()->default_value(0),
                "disable patching the rom")
            ("PALETTE", po::value<int>()->default_value(0),
                "0 = Full colour, 1 = Unsaturated colours (slightly more CRT like), 2 =  Enable grayscale display")
            ("PRINT", po::value<string>()->default_value("lpr"),
                "command to use for print jobs")
            ("RAMTOP,r", po::value<int>(),
                "The memory space top (128K + QL ram, not valid if ramsize set)")
            ("RAMSIZE", po::value<int>(),
                "The size of ram")
            ("RESOLUTION,g", po::value<string>()->default_value("512x256"),
                "resolution of screen in mode 4")
            ("ROMDIR", po::value<string>()->default_value("roms/"),
                "path to the roms")
            ("ROMPORT", po::value<string>(),
                "rom in QL rom port (0xC000 address)")
            ("ROMIM", po::value<string>(),
                "rom in QL rom port (0xC000 address, legacy alias for romport)")
            ("SER1", po::value<string>(),
                "device for ser1")
            ("SER2", po::value<string>(),
                "device for ser2")
            ("SER3", po::value<string>(),
                "device for ser3")
            ("SER4", po::value<string>(),
                "device for ser4")
            ("SHADER", po::value<int>()->default_value(0),
                "0 = Disabled, 1 = Use flat shader, 2 = Use curved shader")
            ("SHADER_FILE", po::value<string>()->default_value("shader.glsl"),
                "Path to shader file to use if SHADER is 1 or 2")
            ("SKIP_BOOT", po::value<int>()->default_value(1),
                "1 = skip f1/f2 screen, 0 = show f1/f2 screen")
            ("SOUND", po::value<int>()->default_value(0),
                "volume in range 1-8, 0 to disable")
            ("SPEED", po::value<float>()->default_value(0.0),
                "speed in factor of BBQL speed, 0.0 for full speed")
            ("STRICT_LOCK", po::value<int>()->default_value(0),
                "enable strict file locking")
            ("SYSROM", po::value<string>()->default_value("MIN198.rom"),
                "system rom")
            ("WIN_SIZE,w", po::value<string>()->default_value("1x"),
                "window size 1x, 2x, 3x, max, full")
            ;

        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("SQLUX-ARGS", po::value< vector<string> >(), "Arguments for QDOS")
            ;

        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config).add(hidden);

        po::options_description config_file_options;
        config_file_options.add(config); //.add(hidden);

        po::options_description visible("Allowed options");
        visible.add(generic).add(config);

        po::positional_options_description p;
        p.add("SQLUX-ARGS", -1);

        store(po::command_line_parser(argc, argv).
              options(cmdline_options).positional(p).style(po_style::unix_style|po_style::case_insensitive).run(), vm);
        notify(vm);

        if (vm.count("HELP")) {
            cout << visible << "\n";
            return 1;
        }

        if (vm.count("CONFIG")) {
            ifstream ifs(vm["CONFIG"].as<string>().c_str());
            if (!ifs) {
                cout << "Cannot open config file " << vm["config"].as<string>() << "\n";
                return 1;
            } else {
                store(parse_config_file(ifs, config_file_options), vm);
                notify(vm);
            };
        } else {
            string home(homedir);
            vector<string> config_files= {"sqlux.ini", home + "/.sqluxrc", home + "/.uqlxrc"};

            for (auto &file: config_files) {
                if (filesystem::exists(file)) {
                    cout << "Loading Config from: " << file <<"\n";
                    ifstream ifs(file);
                    if (!ifs) {
                        cout << "Cannot open config file " << file << "\n";
                    } else {
                        store(parse_config_file(ifs, config_file_options), vm);
                        notify(vm);
                    }
                    break;
                }
            }
        }
    }
    catch(exception& e)
    {
        cout << e.what() << "\n";
        return 1;
    }
    return 0;
}
}

extern "C" {

int optionInt(char *optionName)
{
    if (emulator::vm.count(optionName)) {
        return emulator::vm[optionName].as<int>();
    }

    return 0;
}

const char *optionString(char *optionName)
{
    if (emulator::vm.count(optionName)) {
        return emulator::vm[optionName].as<std::string>().c_str();
    }

    return "";
}

float optionFloat(char *optionName)
{
    if (emulator::vm.count(optionName)) {
        return emulator::vm[optionName].as<float>();
    }

    return 0.0;
}

int optionArgc()
{
    if (emulator::vm.count("SQLUX-ARGS")) {
        return emulator::vm["SQLUX-ARGS"].as< std::vector<std::string> >().size();
    }

    return 0;
}

const char *optionArgv(int index)
{
    if (index == 0) {
        return emulator::argv0.c_str();
    }

    index--;

    if (emulator::vm.count("SQLUX-ARGS")) {
        return emulator::vm["SQLUX-ARGS"].as< std::vector<std::string> >()[index].c_str();
    }

    return "";
}

}