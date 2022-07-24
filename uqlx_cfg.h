/*
 * (c) UQLX - see COPYRIGHT
 */

#ifndef _qm_parse_h
#define _qm_parse_h

#include <stdio.h>
#include <limits.h>

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
	char romport[64];
	char iorom1[64];
	char iorom2[64];
	char ser1[64];
	char ser2[64];
	char ser3[64];
	char ser4[64];
	short joy1;
	short joy2;
	char prtcmd[64];
	char resolution[64];
	char bootdev[5];
	char bdi1[64];
	char winsize[5];
	short cpu_hog;
	short fastStartup;
	short skip_boot;
	short strict_lock;
	short no_patch;
	short aspect;
	short filter;
	float speed;
	short sound;
	char kbd[3];	// Two characters + null terminator
    int gray;
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

void QMParams(void);
int QMParseParam(char *pbuf);

#endif
