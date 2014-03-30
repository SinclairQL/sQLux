/*
 * (c) UQLX - see COPYRIGHT
 */


#define BASIC_QL_ROM "jsrom"      /* you favourite ROM */
#define FLP_DISK  "DiskImage2"    /* name of your flp1_ drive */
#define USE_COLOR 0   /* set 1 if your XServer is correctly configured */


#define DATAD   "mdv1_"
#define PROGD   "flp1_"
#define SPOOLD  "SER"
#define PRINT_COMMAND "/usr/bin/lpr" /* where PRT sends it data */
/* experimental raw PAR device (n.i.) */
#define PAR_OUT_COMMAND "cat >/dev/lp0" /* PAR output */
#define PAR_IN_COMMAND "cat /dev/lp0"   /* PAR input */

#define TEMPDIR "/tmp"
#ifndef PATH_MAX 
#define PATH_MAX 400
#endif

extern char *oldscr;

