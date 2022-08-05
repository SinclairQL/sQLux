#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

namespace emulator
{
using namespace std;
namespace po = boost::program_options;
namespace po_style = boost::program_options::command_line_style;

static po::variables_map vm;

int optionParse(int argc, char *argv[])
{
    try {
        po::options_description generic("Generic options");
        generic.add_options()
        ("config,f", po::value<string>(),
                "sqlux.ini configuration file.")
        ("help,h",
            "produce help message")
        ("verbose,v", po::value<int>()->default_value(1),
            "verbosity level 0-3")
            ;

        po::options_description config("Configuration");
        config.add_options()
            ("bdi1", po::value<string>(),
                "file exposed by the BDI interface")
            ("boot_device,d", po::value<string>()->default_value("MDV1"),
                "device to load BOOT file from")
            ("cpu_hog", po::value<int>()->default_value(1),
                "1 = use all cpu, 0 = sleep when idle")
            ("device", po::value< vector<string> >(),
                "QDOS_name,path,flags (may be used multiple times")
            ("fast_startup", po::value<int>()->default_value(0),
                "1 = skip ram test (does not affect Minerva)")
            ("filter", po::value<int>()->default_value(0),
                "enable bilinear filter when zooming")
            ("fixaspect", po::value<int>()->default_value(0),
                "0 = 1:1 pixel mapping, 1 = BBQL aspect non square pixels")
            ("grey", po::value<int>()->default_value(0),
                "1 enable greyscale display")
            ("gray", po::value<int>()->default_value(0),
                "1 enable grayscale display")
            ("iorom1", po::value<string>(),
                "rom in 1st IO area (Minerva only 0x10000 address)")
            ("iorom2", po::value<string>(),
                "rom in 2nd IO area (Minerva only 0x14000 address)")
            ("joy1", po::value<int>(),
                "1-8 SDL2 joystick index")
            ("joy2", po::value<int>(),
                "1-8 SDL2 joystick index")
            ("kbd", po::value<string>()->default_value("US"),
                "keyboard language DE, GB, US")
            ("no_patch,n", po::value<int>()->default_value(0),
                "disable patching the rom")
            ("print", po::value<string>()->default_value("lpr"),
                "command to use for print jobs")
            ("ramtop,r", po::value<int>(),
                "The memory space top (128K + QL ram, not valid if ramsize set)")
            ("ramsize", po::value<int>(),
                "The size of ram")
            ("resolution,g", po::value<string>()->default_value("512x256"),
                "resolution of screen in mode 4")
            ("romdir", po::value<string>()->default_value("roms/"),
                "path to the roms")
            ("romport", po::value<string>(),
                "rom in QL rom port (0xC000 address)")
            ("romim", po::value<string>(),
                "rom in QL rom port (0xC000 address, legacy alias for romport)")
            ("ser1", po::value<string>(),
                "device for ser1")
            ("ser2", po::value<string>(),
                "device for ser2")
            ("ser3", po::value<string>(),
                "device for ser3")
            ("ser4", po::value<string>(),
                "device for ser4")
            ("skip_boot", po::value<int>()->default_value(1),
                "1 = skip f1/f2 screen, 0 = show f1/f2 screen")
            ("sound", po::value<int>()->default_value(0),
                "volume in range 1-8, 0 to disable")
            ("speed", po::value<float>()->default_value(0.0),
                "speed in factor of BBQL speed, 0.0 for full speed")
            ("strict_lock", po::value<int>()->default_value(0),
                "enable strict file locking")
            ("sysrom", po::value<string>()->default_value("MIN198.rom"),
                "system rom")
            ("win_size,w", po::value<string>(),
                "window size 1x, 2x, 3x, max, full")
            ;

        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config); //.add(hidden);

        po::options_description config_file_options;
        config_file_options.add(config); //.add(hidden);

        po::options_description visible("Allowed options");
        visible.add(generic).add(config);

        store(po::command_line_parser(argc, argv).
              options(cmdline_options).style(po_style::unix_style|po_style::case_insensitive).run(), vm);
        notify(vm);

        if (vm.count("help")) {
            cout << visible << "\n";
            return 0;
        }

        if (vm.count("config")) {
            ifstream ifs(vm["config"].as<string>().c_str());
            if (!ifs) {
                cout << "Cannot open config file " << vm["config"].as<string>() << "\n";
                return 1;
            } else {
                stringstream config;
                string line;

                while (getline(ifs, line)) {
                    boost::algorithm::to_lower(line);
                    config << line << "\n";
                }
                store(parse_config_file(config, config_file_options), vm);
                notify(vm);
            };
        } else {
            string home = getenv("HOME");
            vector<string> config_files= {"sqlux.ini", home + "/.sqluxrc", home + "/.uqlxrc"};

            for (auto &file: config_files) {
                if (filesystem::exists(file)) {
                    cout << "Loading Config from: " << file <<"\n";
                    ifstream ifs(file);
                    if (!ifs) {
                        cout << "Cannot open config file " << file << "\n";
                    } else {
                        stringstream config;
                        string line;

                        while (getline(ifs, line)) {
                            boost::algorithm::to_lower(line);
                            config << line << "\n";
                        }
                        store(parse_config_file(config, config_file_options), vm);
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

}