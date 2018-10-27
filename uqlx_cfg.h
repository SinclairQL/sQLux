/*
 * (c) UQLX - see COPYRIGHT
 */


#ifndef _qm_parse_h
#define _qm_parse_h
#include "emudisk.h"

typedef struct _qmlist
{
  struct _qmlist *next;
  void *udata;
} QMLIST;

typedef struct _rominfo
{
  char *romname;
  long romaddr;
} ROMITEM;

typedef struct
{
  EMUDEV_t *qdev;
  QMLIST *romlist;
  long ramtop;
  char romdir[128];
  short color;
  char sysrom[64];
  char ser1[64];
  char ser2[64];
  char ser3[64];
  char ser4[64];
  char prtcmd[64];
#if 0
  char hda[512];
  char hdb[512];
  char cdrom[512];
#endif
  short cpu_hog;
  short fastStartup;
  short skip_boot;
  short fwhite;
  short xkey_on;
  short do_grab;
  short strict_lock;
  short no_patch;
  
  short pref_vid;
  short pref_depth;
  char pref_class[64];

  char size_x[128];
  char size_xx[128];
  char size_xxx[128];
  char xkey_switch[128];
  char xkey_alt[128];
} QMDATA;

#define QMFILE "~/.uqlxrc"

extern FILE *lopen(const char *s, const char *mode);
extern char *ExpandName(char *);

#include <stddef.h>

typedef void (*PVFV)(void *, void *,...);

typedef union { int mval; void *(*mfun)();} uxt;

typedef struct
{
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


