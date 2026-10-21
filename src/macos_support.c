/*
 * macOS app bundle support
 *
 * A bundle launched from the Finder starts with "/" as its working
 * directory, while sQLux looks for sqlux.ini, the roms and the device
 * directories relative to the working directory. When running from a
 * bundle without a sqlux.ini in the working directory, and without an
 * explicit --config, switch to ~/Library/Application Support/sQLux and
 * seed it with the defaults shipped in the bundle Resources. Existing
 * files are never overwritten, so user changes survive app updates.
 */

#ifdef __APPLE__

#include <copyfile.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <SDL.h>

#include "macos_support.h"

static const char *seedItems[] = {
	"sqlux.ini", "roms", "mdv1", "mdv2", "mdv3", "shaders", NULL
};

static int pathExists(const char *path)
{
	struct stat st;

	return stat(path, &st) == 0;
}

static int hasConfigArg(int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-f") || !strncmp(argv[i], "--config", 8))
			return 1;
	}
	return 0;
}

static int endsWith(const char *s, const char *suffix)
{
	size_t sl = strlen(s), xl = strlen(suffix);

	return sl >= xl && !strcmp(s + sl - xl, suffix);
}

void macosPrepareEnvironment(int argc, char *argv[])
{
	char src[PATH_MAX], dst[PATH_MAX];
	char *resources, *prefs;
	int i;

	if (hasConfigArg(argc, argv) || pathExists("sqlux.ini"))
		return;

	/* For a bundle SDL returns the Contents/Resources directory */
	resources = SDL_GetBasePath();
	if (!resources)
		return;
	if (!endsWith(resources, ".app/Contents/Resources/")) {
		SDL_free(resources);
		return;
	}

	prefs = SDL_GetPrefPath("", "sQLux");
	if (!prefs) {
		SDL_free(resources);
		return;
	}

	for (i = 0; seedItems[i]; i++) {
		snprintf(src, sizeof(src), "%s%s", resources, seedItems[i]);
		snprintf(dst, sizeof(dst), "%s%s", prefs, seedItems[i]);
		if (!pathExists(src) || pathExists(dst))
			continue;
		if (copyfile(src, dst, NULL, COPYFILE_ALL | COPYFILE_RECURSIVE) < 0)
			perror(dst);
	}

	if (chdir(prefs) < 0)
		perror(prefs);

	SDL_free(prefs);
	SDL_free(resources);
}

#endif /* __APPLE__ */
