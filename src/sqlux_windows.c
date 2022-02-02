/*
 * Copyright (c) 2021 Graeme Gregory
 *
 * SPDX: Zlib
 */

#ifdef __WIN32__

#include <string.h>
#include <fcntl.h>
#include <fileapi.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sysinfoapi.h>
#include <sys/stat.h>
#include <time.h>

#define POW10_7                 10000000
#define DELTA_EPOCH_IN_100NS    INT64_C(116444736000000000)

void sqlux_getemppath(int length, char *buffer)
{
	GetTempPath(length, buffer);
}

char *__randname(char *template)
{
	int i;
	struct timespec ts;
	unsigned long r;
	union {
		unsigned __int64 u64;
		FILETIME ft;
	} ct;

	GetSystemTimeAsFileTime(&ct.ft);

	r = (((ct.u64 - DELTA_EPOCH_IN_100NS) % POW10_7) * 100) * 65537 ^ (uintptr_t)&ts / 16 + (uintptr_t)template;
	for (i=0; i<6; i++, r>>=5)
		template[i] = 'A'+(r&15)+(r&16)*2;

	return template;
}

int sqlux_mkstemp(char *template)
{
	size_t l = strlen(template);
	if (l<6 || memcmp(template+l-6, "XXXXXX", 6)) {
		errno = EINVAL;
		return -1;
	}

	int fd, retries = 100;
	do {
		__randname(template+l-6);
		if ((fd = _open(template,
				_O_CREAT | _O_TEMPORARY | _O_RDWR | O_EXCL | _O_SHORT_LIVED | _O_BINARY,
				_S_IREAD | _S_IWRITE))>=0)
			return fd;
	} while (--retries && errno == EEXIST);

	memcpy(template+l-6, "XXXXXX", 6);
	return -1;
}

#endif /* __WIN32__ */
