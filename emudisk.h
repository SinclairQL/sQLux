/*
 * (c) UQLX - see COPYRIGHT
 */


#ifndef __emudisk_h
#define __emudisk_h

/* Emulated device structure */

/* Where has new meaning :
 *           0: unixfilesys
 *           1: QL floppy/QXL.WIN
 *           2: unixfilesys, case-insensitive
 */

typedef struct 
{
    char *qname;
    char Where[8];
    char Present[8];
    short OpenFiles[8];
    struct mdvFile *FileList[8];
    struct HF_FCB *fcbList[8];
    char *mountPoints[8];
    int clean[8];
    struct FLP_FCB *flpp[8];
    int ref;
} EMUDEV_t;

extern EMUDEV_t qdevs[];
#define MAXDEV 16

/* 6/12/97 RZ removed non-essential defaults */

#ifdef STATICDEVS
EMUDEV_t qdevs[16] = {
#if 0
    {"FLP", {0}, {1,1,0,0,0,0,0,0}, {0}, {NULL}, {NULL},
	 {"/ql/qldata/","/ql/qlsoft/"},{0},{0},0},
    {"WIN", {0}, {1,1,1,0,0,0,0,0}, {0}, {NULL}, {NULL},
	 {HOME,"/","/ql/"},{0},{0},0},
#endif
#if 0
    {"RAM", {0}, {1,1,1,1,1,1,1,1}, {0}, {NULL}, {NULL},
	 {
	     "/tmp/.ram1/","/tmp/.ram2/",
	     "/tmp/.ram3/","/tmp/.ram4/",
	     "/tmp/.ram5/","/tmp/.ram6/",
	     "/tmp/.ram7/","/tmp/.ram8/",},{0,0,0,0,0,0,0,0},{0},0},
#endif
#if 0
    {"CD", {0}, {1,0,0,0,0,0,0,0}, {0}, {NULL}, {NULL},
	 {"/cdrom/"},{0},{0},0},
    {"MS", {0}, {1,0,0,0,0,0,0,0}, {0}, {NULL}, {NULL},
	 {"/a/"},{0},{0},0},
#endif
    {0}
};

#endif
#endif
