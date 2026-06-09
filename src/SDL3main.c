#include <ftw.h>
#include <SDL3/SDL_init.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "emudisk.h"
#include "emulator_init.h"
#include "emulator_options.h"
#include "QL_sound.h"
#include "SDL3screen.h"
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
						printf("Cleaning: %s\n",
						       qdevs[i].mountPoints[j]);
					}
					rmrf(qdevs[i].mountPoints[j]);
				}
			}
			break;
		}
	}
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	(void)appstate;
	(void)result;
	QLSDLExit();

	CleanRAMDev();

	SDL_Quit();
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	(void)appstate;

	if (QLSDLProcessEvents(event)) {
		return SDL_APP_CONTINUE;
	}
	return SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
	(void)appstate;

	QLRun();

	return SDL_APP_CONTINUE;
}

#ifdef __WIN32__
#include <windows.h>
static void reattach_console(void)
{
	// Will succeed if launched from console,
	// will fail if launched from GUI
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
}
#endif

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
#if __EMSCRIPTEN__
	wasm_init_storage();
#endif
#ifdef __WIN32__
	// Display output if started from console
	if (!getenv("SQLUX_WIN_DISABLE_CONSOLE_OUTPUT")) {
		reattach_console();
	}
#endif

	// set the homedir for the OS first
	SetHome();

	emulatorOptionParse(argc, argv);

	// Set some things that used to be set as side effects
	const char *resString = emulatorOptionString("resolution");
	parse_screen(resString);
	verbose = emulatorOptionInt("verbose");

	// setup the boot_cmd if needed
	const char *boot_cmd = emulatorOptionString("boot_cmd");
	if (strlen(boot_cmd)) {
		ux_boot = 2;
		int len = strlen(boot_cmd);
		ux_bname = (char *)malloc(len + 2);
		strncpy(ux_bname, boot_cmd, len + 2);
		ux_bname[len] = 0x0A;
		ux_bname[len + 1] = 0;
	}

	emulatorInit();
	if (QLSDLScreen()) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			     "failure to create screen");
		exit(1);
	}
	initSound(emulatorOptionInt("sound"));

	uqlxSpeed = (int)(atof(emulatorOptionString("speed")) * 20.0);

	return SDL_APP_CONTINUE;
}
