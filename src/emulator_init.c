#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include "QL68000.h"
#include "sds.h"
#include "unixstuff.h"

int emulatorLoadRom(char *romDir, char *romName, uint32_t addr, size_t size)
{
	struct stat romStat;
	int ret, romFile;
	sds romPath;

	if (romDir[0] == '~') {
		romPath = sdscatprintf(sdsnew(""), "%s/%s/%s",
			homedir, romDir + 1, romName);
	} else {
		romPath = sdscatprintf(sdsnew(""), "%s/%s", romDir, romName);
	}

	ret = lstat(romPath, &romStat);
	if (ret < 0) {
		fprintf(stderr, "FUNC: %s ERR: %s VAL: %s\n",
			__func__, strerror(errno), romPath);
		return ret;
	}

	if (romStat.st_size != size) {
		fprintf(stderr, "FUNC: %s ERR: Rom Size Error VAL: %zd != %jd\n",
			__func__, size, (intmax_t)romStat.st_size);
		return -1;
	}

	romFile = open(romPath, O_RDONLY);
	if (romFile < 0) {
		fprintf(stderr, "FUNC: %s ERR: %s VAL: %s\n",
			__func__, strerror(errno), romPath);
		return -1;
	}
    	ret = read(romFile, (char *)memBase + addr, size);
	if (ret < 0) {
		fprintf(stderr, "FUNC: %s ERR: %s VAL: %s\n",
			__func__, strerror(errno), romPath);
	}
    	close(romFile);

	sdsfree(romPath);

	return ret;
}