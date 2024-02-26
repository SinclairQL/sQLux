#include <ftw.h>
#include <SDL.h>
#include <stdio.h>
#include <unistd.h>

#include "debug.h"
#include "emudisk.h"
#include "emulator_init.h"
#include "emulator_options.h"
#include "QL_sound.h"
#include "SDL2screen.h"
#include "unixstuff.h"
#include "Xscreen.h"

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include "wasm_support.h"
#endif

static SDL_Thread *emuThread = NULL;

static int unlink_cb(const char *fpath, const struct stat *sb, int typeflag,
	      struct FTW *ftwbuf)
{
	int rv = remove(fpath);

	if (rv)
		perror(fpath);

	return rv;
}

static int rmrf(const char *path)
{
	return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

void CleanRAMDev()
{
    int i, j;

    for (i = 0; i < MAXDEV; i++) {
        if (qdevs[i].qname && strcmp(qdevs[i].qname, "RAM") == 0) {
            for (j = 0; j < 8; j++) {
                if ((qdevs[i].mountPoints[j] != NULL) &&
                    qdevs[i].clean[j]) {
                    if (V2) {
                        printf("Cleaning: %s\n", qdevs[i].mountPoints[j]);
                    }
                    rmrf(qdevs[i].mountPoints[j]);
                }
            }
            break;
        }
    }
}

void emu_shutdown()
{
    QLdone = 1;

    SDL_WaitThread(emuThread, NULL);

    QLSDLExit();

    CleanRAMDev();
}

void emu_loop() {
    static int init_done = 0;
    int boot_file_ready = 1;

#if __EMSCRIPTEN__
    if(!init_done) {
        boot_file_ready = wasm_does_boot_file_exist();
    }
#endif
    if(boot_file_ready && !init_done) {
        emulatorInit();
        QLSDLScreen();
        initSound(emulatorOptionInt("sound"));
        emuThread = SDL_CreateThread(QLRun, "sQLux Emulator", NULL);
        init_done = 1;
    }
    if(init_done) {
        QLSDLProcessEvents();
    }
}

#ifdef __WIN32__
#include <windows.h>
static void reattach_console(void)
{
   // Will succeed if launched from console,
   // will fail if launched from GUI
   if (AttachConsole(ATTACH_PARENT_PROCESS))
   {
       freopen("CONIN$", "r", stdin);
       freopen("CONOUT$", "w", stdout);
       freopen("CONOUT$", "w", stderr);
   }
}
#endif

int main(int argc, char *argv[])
{
#if __EMSCRIPTEN__
    wasm_init_storage();
#endif
#ifdef __WIN32__
    // Display output if started from console
    reattach_console();
#endif

    // set the homedir for the OS first
    SetHome();

    emulatorOptionParse(argc, argv);

    // Set some things that used to be set as side effects
    const char *resString = emulatorOptionString("resolution");
    parse_screen(resString);
    verbose = emulatorOptionInt("verbose");

    // setup the boot_cmd if needed
    const char *boot_cmd=emulatorOptionString("boot_cmd");
    if (strlen(boot_cmd)) {
        ux_boot = 2;
        int len = strlen(boot_cmd);
        ux_bname = (char *)malloc(len + 2);
        strncpy(ux_bname, boot_cmd, len + 2);
        ux_bname[len] = 0x0A;
        ux_bname[len + 1] = 0;
    }


#if __EMSCRIPTEN__
    emscripten_set_main_loop(emu_loop, -1, 1);
#else
    emu_loop();
#endif

    emu_shutdown();

    return 0;
}
