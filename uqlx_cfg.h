/*
 * (c) UQLX - see COPYRIGHT
 */

#ifndef _qm_parse_h
#define _qm_parse_h
#ifdef __APPLE__
#include <sys/syslimits.h>
#else
#include <linux/limits.h>
#endif
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
	char config_file[PATH_MAX + 1];
	char config_file_opt;
	EMUDEV_t *qdev;
	long ramtop;
	char romdir[128];
	char sysrom[64];
	char romim[64];
	char ser1[64];
	char ser2[64];
	char ser3[64];
	char ser4[64];
	char prtcmd[64];
	char bootdev[5];
	short cpu_hog;
	short fastStartup;
	short skip_boot;
	short strict_lock;
	short no_patch;
} QMDATA;

#define QMFILE "~/.uqlxrc"
#define QLUXFILE "~/.sqluxrc"
#define QLUXFILE_LOC "sqlux.ini"

extern FILE *lopen(const char *s, const char *mode);
extern char *ExpandName(char *);

#include <stddef.h>

typedef void (*PVFV)(void *, void *, ...);

typedef union {
	int mval;
	void *(*mfun)(QMLIST *, char *);
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
