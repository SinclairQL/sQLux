/*
 * (c) UQLX - see COPYRIGHT
 */

#ifndef _qm_parse_h
#define _qm_parse_h
#include "emudisk.h"

typedef struct _qmlist {
	struct _qmlist *next;
	void *udata;
} QMLIST;

typedef struct _rominfo {
	char *romname;
	long romaddr;
} ROMITEM;

typedef struct {
	EMUDEV_t *qdev;
	QMLIST *romlist;
	long ramtop;
	char romdir[128];
	char sysrom[64];
	char ser1[64];
	char ser2[64];
	char ser3[64];
	char ser4[64];
	char prtcmd[64];
	short cpu_hog;
	short fastStartup;
	short skip_boot;
	short strict_lock;
	short no_patch;
} QMDATA;

#define QMFILE "~/.uqlxrc"

extern FILE* lopen(const char *s, const char *mode);
extern char* ExpandName(char*);

#include <stddef.h>

typedef void (*PVFV)(void*, void*, ...);

typedef union {
	int mval;
	void* (*mfun)();
} uxt;

typedef struct {
	char *id;
	PVFV func;
	long offset;
	uxt mx;
} PARSELIST;

#ifndef PATH_MAX
#include <limits.h>
#endif

extern QMDATA QMD;

#endif

