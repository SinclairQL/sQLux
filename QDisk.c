/*
 * (c) UQLX - see COPYRIGHT
 */

/*#include "QLtypes.h"*/
#include "QL68000.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "QDisk.h"
#include "QFilesPriv.h"
#include "QL.h"
#include "QL_config.h"
#include "unix.h"
#include "emudisk.h"
#include "QDOS.h"
#include "dummies.h"
#include "memaccess.h"

#include "SDL2screen.h"
#include "SqluxOptions.hpp"

#include "unixstuff.h"
#include "uqlx_cfg.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define min(_a_, _b_) (_a_ < _b_ ? _a_ : _b_)
#define max(_a_, _b_) (_a_ > _b_ ? _a_ : _b_)

#define LF_FOUND -9832
#define ERR_BUFFER_FULL -9833

#if 0
struct qDiscHeader {
  uw16	id;
  uw16	ver;
  char	name[10];
  uw16	random;
  uw32	updates;
  uw16	free;
  uw16	good;
  uw16	total;
  uw16	secTrack;
  uw16	secCylinder;
  uw16	tracks;
  uw16	secBlock;
  uw16	dirEofBlock;
  uw16	dirEofByte;
  uw16	offset;
  uw8		ltp[18];
  uw8		ptl[18];
};
#endif

struct qDiscHeader {
	char dummy[512];
};
#define QDH_VER(_p) ((uw16)RW((char *)(_p) + 2))
#define QDH_ID(_p) ((uw16)RW((char *)(_p)))
#define QDH_NAME(_p) ((char *)((char *)(_p) + 4))
#define QDH_RAND(_p) ((uw16)RW((char *)(_p) + 14))
#define QDH_UPDTS(_p) ((uw32)RL((char *)(_p) + 16))

#define QDH_FREE(_p) ((uw16)RW((char *)(_p) + 20))
#define QDH_GOOD(_p) ((uw16)RW((char *)(_p) + 22))
#define QDH_TOTAL(_p) ((uw16)RW((char *)(_p) + 24))
#define QDH_SPT(_p) ((uw16)RW((char *)(_p) + 26))
#define QDH_SPC(_p) ((uw16)RW((char *)(_p) + 28))
#define QDH_TRACKS(_p) ((uw16)RW((char *)(_p) + 30))
#define QDH_SPB(_p) ((uw16)RW((char *)(_p) + 32))

#define QDH_DIREOFBL(_p) ((uw16)RW((char *)(_p) + 34))
#define QDH_DIREOFBY(_p) ((uw16)RW((char *)(_p) + 36))
#define QDH_OFFSET(_p) ((uw16)RW((char *)(_p) + 38))

#define QDH_LTP(_p) ((uw8 *)((char *)(_p) + 40))
#define QDH_PTL(_p) ((uw8 *)((char *)(_p) + 40 + 18))

#define QDH_SET_VER(_p, _ver) (WW((char *)(_p), _ver + 2))
#define QDH_SET_ID(_p, _ver) (WW((char *)(_p), _ver))
#define QDH_SET_RAND(_p, _rand) (WW((char *)(_p) + 14, _rand))
#define QDH_SET_UPDTS(_p, _up) (WL((char *)(_p) + 16, _up))
#define QDH_SET_FREE(_p, _ver) (WW((char *)(_p) + 20, _ver))
#define QDH_SET_GOOD(_p, _ver) (WW((char *)(_p) + 22, _ver))
#define QDH_SET_TOTAL(_p, _ver) (WW((char *)(_p) + 24, _ver))
#define QDH_SET_SPT(_p, _ver) (WW((char *)(_p) + 26, _ver))
#define QDH_SET_SPC(_p, _ver) (WW((char *)(_p) + 28, _ver))
#define QDH_SET_TRACKS(_p, _ver) (WW((char *)(_p) + 30, _ver))
#define QDH_SET_SPB(_p, _ver) (WW((char *)(_p) + 32, _ver))

#define QDH_SET_DIREOFBL(_p, _ver) (WW((char *)(_p) + 34, _ver))
#define QDH_SET_DIREOFBY(_p, _ver) (WW((char *)(_p) + 36, _ver))
#define QDH_SET_OFFSET(_p, _ver) (WW((char *)(_p) + 38, _ver))

/* and here the definitions for QXL.WIN type disks */
/* VER=="QL", ID=="WA" use QDH_VER,QDH_IF */
/* name use QDH_NAME, 20 chars space padded */
/* ignore interleave 0x20 */
#define QWA_SPC(_p) ((uw16)RW((char *)(_p) + 0x22)) /* sect per cluster(group)*/
/* ignore sect_per_track*/
#define QWA_CC(_p) ((uw16)RW((char *)(_p) + 0x2a)) /*#of groups(clusters)*/
#define QWA_FC(_p) ((uw16)RW((char *)(_p) + 0x2c))
#define QWA_SETFC(_p, _gn) (WW((char *)(_p) + 0x2c, (_gn)))
#define QWA_SPM(_p) ((uw16)RW((char *)(_p) + 0x2e))
/* 0x2e :ignore sectors per map,0x30 #ofmaps */
#define QWA_FFC(_p) ((uw16)RW((char *)(_p) + 0x32)) /* linked list of free cls*/
#define QWA_SETFFC(_p, _gn) (WW((char *)(_p) + 0x32, (_gn)))
#define QWA_ROOT(_p) ((uw16)RW((char *)(_p) + 0x34)) /*1.cluster# of root dir*/
#define QWA_RLEN(_p) ((uw32)RL((char *)(_p) + 0x36)) /* root dir len in bytes*/
#define QWA_SETRLEN(_p, _gn) (WL((char *)(_p) + 0x36, (_gn)))
#define QWA_FAT(_p) (((char *)(_p) + 0x40)) /* beginning of fat */

#define QWDE_FNUM(_p) ((uw16)RW((Ptr)(_p) + 0x3a))
#define QWDE_SETFNUM(_p, _gn) (WW((char *)(_p) + 0x3a, (_gn)))

static int gError;

struct sectorInfo {
	uw32 time;
	int logSector;
	FileNum fileNum;
	Cond free;
	Cond changed;
	Cond locked;
};

struct formatInfo {
	w32 blocks;
	uw16 tracks;
	uw8 sides;
	uw8 sectors;
};

static uw8 format_table_dd[36] = { 0,	 3,    6,    0x80, 0x83, 0x86, 1, 4,
				   7,	 0x81, 0x84, 0x87, 2,	 5,    8, 0x82,
				   0x85, 0x88, 0,    6,	   12,	 1,    7, 13,
				   2,	 8,    14,   3,	   9,	 15,   4, 10,
				   16,	 5,    11,   17 };
static uw8 format_table_hd[36] = {
	0,    2,    4,	  6,	8,    10,   12,	  14,	16,   0x80, 0x82, 0x84,
	0x86, 0x88, 0x8a, 0x8c, 0x8e, 0x90, 1,	  3,	5,    7,    9,	  11,
	13,   15,   17,	  0x81, 0x83, 0x85, 0x87, 0x89, 0x8b, 0x8d, 0x8f, 0x91
};

struct FLP_FCB {
	struct qDiscHeader *qdh;
	struct sectorInfo *si /*[NPOOL]*/;
	char *buffer /*[512*NPOOL]*/;
	int bufcount;

	int refNum; /* file descriptor */
	int counter;
	int file_count;
	time_t lastclose;
	int isdev;
	int readonly;

	Ptr lastSector;

	short fatSectors;
	Cond isValid;
	/*Cond		isDisk;   */
	enum DISK_TYPE { floppyDD, floppyHD, qlwa } DiskType;
};

struct FLP_FCB *curr_flpfcb;

static OSErr DiskRead(long, Ptr, long, long);
static OSErr LoadLogSector(int, Ptr);
static void FixLogical(uw8 *, Cond);
static OSErr WriteLogSector(int, Ptr);
static OSErr WriteBlock0(void);
w32 qfLen(FileNum);

/* replaces dangerous FlushFile */
void FlushSectors()
{
	int k;

	/* this should not be necessary but the code is not yet very clean..*/
	WriteBlock0();

	for (k = 0; k < curr_flpfcb->bufcount; k++) {
		if (!curr_flpfcb->si[k].free && curr_flpfcb->si[k].changed) {
			WriteLogSector(curr_flpfcb->si[k].logSector,
				       curr_flpfcb->buffer + ((long)k << 9));
			curr_flpfcb->si[k].changed = false;
			/*printf("flushing buffer %d\n",k);*/
		}
	}
}

/* routine called every few seconds to close open devices */
/* - to allow diskswapping */
void TestCloseDevs()
{
	int i, j, k;

	for (i = 0; i < MAXDEV; i++)
		for (k = 0; k < 8; k++)
			if ((qdevs[i].Where[k] == 1) &&
			    (curr_flpfcb =
				     qdevs[i].flpp[k])) /* isfloppy/ QXLWIN? */
				if ((curr_flpfcb->file_count == 0) &&
				    (curr_flpfcb->lastclose != -1) &&
				    (time(NULL) - curr_flpfcb->lastclose > 3)) {
					FlushSectors();
					close(curr_flpfcb->refNum);
					free(curr_flpfcb->buffer);
					free(curr_flpfcb->si);
					free(curr_flpfcb);

					qdevs[i].flpp[k] = NULL;
				}
}

/* routines initialization */

static void InitDiskTables()
{
	int i;

	for (i = 0; i < curr_flpfcb->bufcount; i++) {
		curr_flpfcb->si[i].free = true;
		curr_flpfcb->si[i].locked = false;
	}
}

FileNum fat_fn()
{
	FileNum fn;

	fn.dir = 0;
	fn.entrynum = 0;
	fn.file = -1;

	return fn;
}

int is_qlwa()
{
	return (curr_flpfcb->DiskType == qlwa);
}

int sect_per_cluster()
{
	if (curr_flpfcb->DiskType == qlwa)
		return (QWA_SPC(curr_flpfcb->qdh));
	else
		return (QDH_SPB(curr_flpfcb->qdh));
}

FileNum root_fn()
{
	FileNum DirFn;

	DirFn.dir = 0;
	DirFn.entrynum = 0;

	if (curr_flpfcb->DiskType == qlwa)
		DirFn.file = QWA_ROOT(curr_flpfcb->qdh);
	else
		DirFn.file = 0;

	return DirFn;
}

int root_flen()
{
	if (curr_flpfcb->DiskType == qlwa)
		return (QWA_RLEN(curr_flpfcb->qdh));
	else
		return (QDH_DIREOFBL(curr_flpfcb->qdh) << 9) +
		       (QDH_DIREOFBY(curr_flpfcb->qdh));
}

static OSErr LoadSector0(void)
{
	OSErr e;
	int i;
	char buffer[512];

	e = DiskRead(curr_flpfcb->refNum, buffer, 512, 0);
	if (e == 0) {
		if (QDH_ID(buffer) != (('Q' << 8) | 'L'))
			return QERR_NI;
		switch (QDH_VER(buffer)) {
		case ('W' << 8) | 'A':
			curr_flpfcb->DiskType = qlwa;
			break;
		case ('5' << 8) | 'A':
			curr_flpfcb->DiskType = floppyDD;
			break;
		case ('5' << 8) | 'B':
			curr_flpfcb->DiskType = floppyHD;
			break;
		default:
			return QERR_NI;
		}

		if (curr_flpfcb->DiskType == qlwa)
			curr_flpfcb->fatSectors = QWA_SPM(buffer);
		else {
			/* **FIXME**,plain wrong also fixlogoical!! */
			curr_flpfcb->fatSectors =
				(3 * QDH_TOTAL(buffer) / QDH_SPB(buffer) + 96) /
				512;
			/* printf("guessing %d sectors FAT length\n",curr_flpfcb->fatSectors); */
		}

		curr_flpfcb->bufcount =
			curr_flpfcb->fatSectors + 9; /* Fit FAT + some buffer */

		/* free old buffers */
		if (curr_flpfcb->buffer)
			free(curr_flpfcb->buffer);

		curr_flpfcb->buffer =
			(char *)malloc(512 * (curr_flpfcb->bufcount));
		curr_flpfcb->si = (struct sectorInfo *)calloc(
			curr_flpfcb->bufcount, sizeof(struct sectorInfo));
		curr_flpfcb->qdh = (struct qDiscHeader *)curr_flpfcb->buffer;

		e = DiskRead(curr_flpfcb->refNum, curr_flpfcb->buffer, 512, 0);

		if (curr_flpfcb->DiskType != qlwa) {
			if (curr_flpfcb->DiskType == floppyHD)
				curr_flpfcb->fatSectors <<= 1;
			FixLogical(QDH_LTP(curr_flpfcb->qdh),
				   curr_flpfcb->DiskType == floppyDD);
		}

		InitDiskTables();
		for (i = 1; i < curr_flpfcb->fatSectors && e == 0; i++) {
			e = LoadLogSector(i,
					  curr_flpfcb->buffer + ((long)i << 9));
		}
		for (i = 0; i < curr_flpfcb->fatSectors; i++) {
			curr_flpfcb->si[i].free = false;
			curr_flpfcb->si[i].changed = false;
			curr_flpfcb->si[i].locked = true;
			curr_flpfcb->si[i].logSector = i;
			curr_flpfcb->si[i].fileNum = fat_fn();
		}
	}

	return e;
}

/* Host OS disk access functions */

OSErr QFOpenDisk(struct mdvFile *f)
{
	int fs, drnum, fd;
	int res;
	struct stat sbuf;
	int isdev, readonly;
	mode_t xmode;

	fs = GET_FILESYS(f);
	drnum = GET_DRIVE(f);

	curr_flpfcb = qdevs[fs].flpp[drnum];

	if (!curr_flpfcb) {
		readonly = 0;

		res = stat(qdevs[fs].mountPoints[drnum], &sbuf);
		if (res < 0)
			perror("could not stat file/device");

#ifndef NO_LOCK
		if (optionInt("STRICT_LOCK")) {
			xmode = (sbuf.st_mode | S_ISGID) & (~S_IXGRP);
			if (chmod(qdevs[fs].mountPoints[drnum], xmode))
				if (errno != EROFS)
					perror("warning, could not change mode, locking may not be available");
		}
#endif

		fd = open(qdevs[fs].mountPoints[drnum], O_RDWR | O_BINARY);
		if (fd < 0 && (errno == EACCES || errno == EROFS)) {
			fd = open(qdevs[fs].mountPoints[drnum], O_RDONLY | O_BINARY);
			readonly = 1;
		}

		/*  printf("calling OpenDisk, res %d\n",fd);*/

		/*qdevs[fs].Present[drnum]= (fd>=0);*/

		if (fd < 0) {
			perror("file/device not available");
			return -1;
		}

#ifndef NO_LOCK
		isdev = (sbuf.st_mode & S_IFBLK) | (sbuf.st_mode & S_IFCHR);

		if (!readonly) {
#ifndef BSD44
			res = lockf(fd, F_TLOCK, 1024);
#else
			{
				struct flock flk;

				flk.l_start = 0;
				flk.l_len = 1024;
				flk.l_type = F_WRLCK;
				flk.l_whence = SEEK_SET;
				res = fcntl(fd, F_SETLK, &flk);
			}
#endif
			if (res < 0 && !isdev) {
				perror("could not lock file/device");
				return -1;
			}
			if (res < 0 && isdev)
				perror("warning - could not lock file/device");
			/* subject to hope that devices are exclusive... */
		}
#endif

		if (qdevs[fs].flpp[drnum] == NULL)
			curr_flpfcb = qdevs[fs].flpp[drnum] =
				(struct FLP_FCB *)calloc(
					1, sizeof(struct FLP_FCB));

		(qdevs[fs].flpp[drnum])->refNum = fd;

		/*printf("flp%d_ allocated as %s\n",k,qdevs[i].mountPoints[k]);*/

		curr_flpfcb->lastSector = nil;
		curr_flpfcb->counter = 0;

		curr_flpfcb->file_count = 0;
		curr_flpfcb->isdev = isdev;
		curr_flpfcb->lastclose = -1;
		curr_flpfcb->readonly = readonly;

		res = LoadSector0();
		if (res < 0) {
			printf("unrecognised format, not a QDOS medium?\n");
			/*
	     carefull here: buffer has not been allocated
	     or is already freed
	  */

			free(curr_flpfcb);
			curr_flpfcb = qdevs[fs].flpp[drnum] = NULL;

			return -1;
		}
		curr_flpfcb->isValid = 1;

		curr_flpfcb->qdh = (struct qDiscHeader *)curr_flpfcb->buffer;
		/*InitDiskTables();*/
		return 0;
	} else
		return 0; /* nothing to do */
}

static OSErr DiskRead(long fref, Ptr dest, long count, long offset)
{
	int res, fd;

	fd = fref;

	/*printf("positioning at %d\n",offset);*/
	res = lseek(fd, offset, SEEK_SET);
	if (res < 0) {
		perror("DiskRead:lseek");
		return -1;
	}

	res = x_read(fd, dest, count);
	/*  printf("calling DiskRead, res %d\n",res);*/
	if (res < 0) {
		perror("DiskRead:read");
		return -1;
	}

	return 0;
}

static OSErr DiskWrite(long fref, Ptr dest, long count, long offset)
{
	int res, fd;
	fd = fref;

	res = lseek(fd, offset, SEEK_SET);
	if (res < 0) {
		perror("DiskWrite:lseek");
		return -1;
	}

	res = write(fd, dest, count);
	/*  printf("calling DiksWrite\n");*/
	if (res < 0)
		return -1;
	return 0;
}

/* read/write sectors */

static OSErr LoadPhySector(uw8 side, uw16 track, uw16 sector, Ptr p)
{
	OSErr e;

	if (curr_flpfcb->buffer != nil) {
		e = DiskRead(curr_flpfcb->refNum, p, 512,
			     ((long)track * QDH_SPC(curr_flpfcb->qdh) + sector +
			      side * QDH_SPT(curr_flpfcb->qdh))
				     << 9);
	} else
		e = ERR_UNINITIALIZED_DISK;
	return e;
}

static OSErr LoadLogSector(int sector, Ptr p)
{
	uw8 side;
	uw16 track;

	if (curr_flpfcb->buffer == nil)
		return ERR_UNINITIALIZED_DISK;

	if (curr_flpfcb->DiskType == qlwa) {
		return DiskRead(curr_flpfcb->refNum, p, 512, sector * 512);
	} else {
		side = QDH_LTP(curr_flpfcb->qdh)
			[sector % QDH_SPC(curr_flpfcb->qdh)]; /* ??????????? */
		track = sector / QDH_SPC(curr_flpfcb->qdh);
		sector = side & 0x7f;
		if ((side &= 0x80) != 0)
			side = 1;
		return LoadPhySector(
			side, track,
			(track * QDH_OFFSET(curr_flpfcb->qdh) + sector) %
				QDH_SPT(curr_flpfcb->qdh),
			p);
	}
}

static OSErr WritePhySector(uw8 side, uw16 track, uw16 sector, Ptr p)
{
	OSErr e;
	long abs;

	if (curr_flpfcb->buffer != nil) {
		abs = (long)track * QDH_SPC(curr_flpfcb->qdh) + sector +
		      side * QDH_SPT(curr_flpfcb->qdh);
		QDH_SET_UPDTS(curr_flpfcb->qdh,
			      QDH_UPDTS(curr_flpfcb->qdh) + 1);
		e = DiskWrite(curr_flpfcb->refNum, p, 512, abs << 9);
		/*		if(abs!=0) si->changed=true;	slows down disk operations! */
	} else
		e = ERR_UNINITIALIZED_DISK;
	return e;
}

static OSErr WriteLogSector(int sector, Ptr p)
{
	uw8 side;
	uw16 track;

	if (curr_flpfcb->buffer == nil)
		return ERR_UNINITIALIZED_DISK;

	if (curr_flpfcb->DiskType == qlwa) {
		return DiskWrite(curr_flpfcb->refNum, p, 512, sector * 512);
	} else {
		side = QDH_LTP(
			curr_flpfcb->qdh)[sector % QDH_SPC(curr_flpfcb->qdh)];
		track = sector / QDH_SPC(curr_flpfcb->qdh);
		sector = side & 0x7f;
		if ((side &= 0x80) != 0)
			side = 1;
		return WritePhySector(
			side, track,
			(track * QDH_OFFSET(curr_flpfcb->qdh) + sector) %
				QDH_SPT(curr_flpfcb->qdh),
			p);
	}
}

/* read/write sectors through buffer */

static Ptr GetSector(int sector, FileNum fileNum)
{
	int i, k;
	long t = 2000000000l;
	Ptr p;

	/*printf("GetSector sector %d, file %d\n",sector,fileNum);*/

	if (curr_flpfcb->buffer == nil) {
		gError = ERR_UNINITIALIZED_DISK;
		return nil;
	}
	for (i = 0; i < curr_flpfcb->bufcount; i++) {
		if (curr_flpfcb->si[i].free) {
			k = i;
			t = -1;
		} else if (curr_flpfcb->si[i].logSector == sector) {
			curr_flpfcb->si[i].time = ++(curr_flpfcb->counter);
			gError = 0;
			/*printf("... returns cached sector %d\n",i);*/
			return curr_flpfcb->buffer + ((long)i << 9);
		} else if (!curr_flpfcb->si[i]
				    .locked /* && curr_flpfcb->si[i].time<t  */) {
			t = curr_flpfcb->si[i].time;
			k = i;
		}
#if 0
      else {
        printf("GetSector1 : Dropped out of if clause!\n");
      }

      printf("GetSector1 : !locked = %d, time %d t %d \t\t",!curr_flpfcb->si[i].locked,curr_flpfcb->si[i].time,t);
      printf(": time<t = %d\n",((long)curr_flpfcb->si[i].time)<t);
      printf("GetSector1 : i = %d, k = %d\n",i, k);
#endif
	}
	/*printf("... uses buffer %d,\t",k);*/

	p = curr_flpfcb->buffer + ((long)k << 9);
	if (!curr_flpfcb->si[k].free && curr_flpfcb->si[k].changed)
		WriteLogSector(curr_flpfcb->si[k].logSector, p);
	gError = LoadLogSector(sector, p);
	if (gError == 0) {
		curr_flpfcb->si[k].logSector = sector;
		curr_flpfcb->si[k].fileNum = fileNum;
		curr_flpfcb->si[k].time = ++(curr_flpfcb->counter);
		curr_flpfcb->si[k].free = false;
		curr_flpfcb->si[k].changed = false;
	} else {
		curr_flpfcb->si[k].free = true;
		p = nil;
	}
	/*printf("returns %d\n",p);*/

	return p;
}

static void PutSector(Ptr p)
{
	int k;
	k = ((char *)p - (char *)(curr_flpfcb->buffer)) >> 9;
	if (k < 0 || k >= curr_flpfcb->bufcount)
		CustomErrorAlert("Bad written sector buffer");
	else {
		curr_flpfcb->si[k].changed = true;
		curr_flpfcb->si[k].time = ++(curr_flpfcb->counter);
	}
}

static OSErr WriteBlock0(void)
{
	int i;
	OSErr e = 0;

	for (i = 1; i < curr_flpfcb->fatSectors && e == 0; i++) {
		e = WriteLogSector(i, curr_flpfcb->buffer + ((long)i << 9));
		curr_flpfcb->si[i].changed = false;
	}
	if (e == 0) {
		e = WriteLogSector(0, curr_flpfcb->buffer);
		curr_flpfcb->si->changed = false;
	}
	return e;
}

static int QLWA_KillFileTail(FileNum fileNum, int sector)
{
	uw16 *p, *pl;
	int group, sr, g;
	int rels = 0;

	p = (uw16 *)(curr_flpfcb->buffer + 0x40);
	group = (sector + QWA_SPC(curr_flpfcb->qdh) - 1) /
			QWA_SPC(curr_flpfcb->qdh) +
		1;
	sr = sector % QWA_SPC(curr_flpfcb->qdh);

	for (p += fileNum.file, g = 0; g != (group - 1) && *p; g++) {
		/*pl=p;*/
		p = (uw16 *)(curr_flpfcb->buffer + 0x40) + (uw16)(RW(p));
	}

	if (g == group - 1) {
		uw16 *p1;

		for (p1 = p; *p1; rels++)
			p1 = (uw16 *)(curr_flpfcb->buffer + 0x40) +
			     (uw16)(RW(p1));

		WW(p1, QWA_FFC(curr_flpfcb->qdh));
		QWA_SETFFC(curr_flpfcb->qdh, (uw16)RW(p));
		WW(p, 0);

		curr_flpfcb->si[((char *)p - curr_flpfcb->buffer) >> 9].changed =
			true;
		curr_flpfcb->si[((char *)p1 - curr_flpfcb->buffer) >> 9]
			.changed = true;

		QWA_SETFC(curr_flpfcb->qdh, QWA_FC(curr_flpfcb->qdh) + rels);

		/*WriteBlock0();*/ /* appears clean */
#if 0 /* expensive, can wait */
      FlushSectors();
#endif
	}

	return 0;
}

/* FAT */
/* get disk logical sector number for a logical sector in a file */
/* QLWA version */
static int QLWA_GetFileSectNum(FileNum fileNum, int sector)
{
	uw16 *p;
	int group, sr, g;

	p = (uw16 *)(curr_flpfcb->buffer + 0x40);
	group = sector / QWA_SPC(curr_flpfcb->qdh);
	sr = sector % QWA_SPC(curr_flpfcb->qdh);

	/*get first cluster of file */
	/*printf("\n SpC %d, first cluster %d\t",QWA_SPC(curr_flpfcb->qdh),fileNum.file);*/
	for (p += fileNum.file, g = 0; g != group && *p; g++) {
		/*printf("p-(buffer+0x40)=%d\t",(int)p-((int) (curr_flpfcb->buffer+0x40)));
      printf("next block nr. %d\t",(uw16)RW(p));*/
		p = (uw16 *)(curr_flpfcb->buffer + 0x40) + (uw16)RW(p);
	}
	if (g == group)
		return ((int)((uintptr_t)p -
			      (uintptr_t)(curr_flpfcb->buffer + 0x40))) /
			       2 * QWA_SPC(curr_flpfcb->qdh) +
		       sr;

	gError = ERR_NO_FILE_BLOCK;
	return -1;
}
/* floppy version */
static int FileBlockSector(FileNum fileNum, int sector)
{
	register uw8 *p;
	register w32 value;
	register w16 i, n;
	int res;
	w16 w;

	if (curr_flpfcb->DiskType == qlwa) {
		res = QLWA_GetFileSectNum(fileNum, sector);
		return res;
	}

	n = (QDH_TOTAL(curr_flpfcb->qdh) / QDH_SPB(curr_flpfcb->qdh)) >> 1;
	p = (uw8 *)(curr_flpfcb->buffer + 96);
	value = ((w32)(fileNum.file & 0x0fff) << 12) +
		((sector / QDH_SPB(curr_flpfcb->qdh)) & 0x0fff);
	w = -1;
	for (i = 0; i < n; i++) {
		if ((RL((w32 *)p) >> 8) == value) {
			w = i << 1;
			break;
		} else if ((RL((w32 *)(p + 2)) & 0x00ffffff) == value) {
			w = (i << 1) + 1;
			break;
		}
		p += 6;
	}
	if (w >= 0) {
		w = w * QDH_SPB(curr_flpfcb->qdh) +
		    (sector % QDH_SPB(curr_flpfcb->qdh));
	} else
		gError = ERR_NO_FILE_BLOCK;
	return w;
}

/* special function for 0th block.. (1th allocation) :*/
static Ptr QLWA_GetFreeBlock0(FileNum *fe)
{
	int g, i;
	int nb;
	uw16 *p;

	if (QWA_FC(curr_flpfcb->qdh) > 0 && QWA_FFC(curr_flpfcb->qdh)) {
		nb = QWA_FFC(curr_flpfcb->qdh);
		p = (uw16 *)(curr_flpfcb->buffer + 0x40);
		p += nb;
		QWA_SETFFC(curr_flpfcb->qdh,
			   (uw16)RW(p)); /* relink free block list */
		WW(p, 0);

		/* mark FAT as changed */
		curr_flpfcb->si[0].changed = true;
		curr_flpfcb->si[((char *)p - curr_flpfcb->buffer) >> 9].changed =
			true;

		QWA_SETFC(curr_flpfcb->qdh, QWA_FC(curr_flpfcb->qdh) - 1);

		(*fe).file = nb;

		return GetSector(nb * QWA_SPC(curr_flpfcb->qdh), *fe);
	} else {
		gError = QERR_DF;
		return nil;
	}
}

/* block ignored for QLWA format */
static Ptr QLWA_GetFreeBlock(FileNum fileNum, int block)
{
	int g, i;
	int nb;
	uw16 *p;

	if (QWA_FC(curr_flpfcb->qdh) > 0 && QWA_FFC(curr_flpfcb->qdh)) {
		nb = QWA_FFC(curr_flpfcb->qdh);
		p = (uw16 *)(curr_flpfcb->buffer + 0x40);
		p += nb;
		QWA_SETFFC(curr_flpfcb->qdh,
			   (uw16)RW(p)); /* relink free block list */
		WW(p, 0);

		/* mark FAT as changed */
		curr_flpfcb->si[((char *)p - curr_flpfcb->buffer) >> 9].changed =
			true;

		p = (uw16 *)(curr_flpfcb->buffer + 0x40);
		for (p += fileNum.file; *p;
		     p = (uw16 *)(curr_flpfcb->buffer + 0x40) + (uw16)RW(p)) {
			/*printf("GetFreeBlock: %d\n",((int)p-(int)(curr_flpfcb->buffer+0x40))/2);*/
		}

		if (*p == 0)
			WW(p, nb); /* append block to file */

		/*printf("GetFreeBlock -last current block: %d \t adding %d\n",((int)p-(int)(curr_flpfcb->buffer+0x40))/2,nb);*/

		/* mark FAT as changed */
		curr_flpfcb->si[0].changed = true;
		curr_flpfcb->si[((char *)p - curr_flpfcb->buffer) >> 9].changed =
			true;
		QWA_SETFC(curr_flpfcb->qdh,
			  QWA_FC(curr_flpfcb->qdh) -
				  1 /*QWA_SPC(curr_flpfcb->qdh)*/);

#if 0 /* delay flush till close,open,flush or delete */
      WriteBlock0();
#endif

		return GetSector(nb * QWA_SPC(curr_flpfcb->qdh), fileNum);
	} else {
		gError = QERR_DF;
		return nil;
	}
}

/* allocate a file block and return a pointer to its first sector */
/* block meaningless for QLWA Format */
static Ptr GetFreeBlock(FileNum fileNum, w16 block)
{
	register uw8 *p;
	register w16 i, n;
	w16 k;

	if (curr_flpfcb->DiskType == qlwa)
		return QLWA_GetFreeBlock(fileNum, block);

	if (QDH_FREE(curr_flpfcb->qdh) >= 1) {
		n = (QDH_TOTAL(curr_flpfcb->qdh) / QDH_SPB(curr_flpfcb->qdh)) >>
		    1;
		p = (uw8 *)(curr_flpfcb->buffer + 96);
		for (i = 0; i < n; i++) {
			if (*p == 0xfd) {
				WW(((w16 *)p),
				   (fileNum.file << 4) | (block >> 8));
				p[2] = (uw8)block;
				k = i << 1;
				curr_flpfcb->si[((i * 6) + 96) >> 9].changed =
					true;
				curr_flpfcb->si[((i * 6) + 98) >> 9].changed =
					true;
				goto found;
			}
			if (p[3] == 0xfd) {
				p[3] = (uw8)(fileNum.file >> 4);
				WW(((w16 *)(p + 4)),
				   (fileNum.file << 12) | (block & 0x0fff));
				k = (i << 1) + 1;
				curr_flpfcb->si[((i * 6) + 99) >> 9].changed =
					true;
				curr_flpfcb->si[((i * 6) + 101) >> 9].changed =
					true;
			found:
				QDH_SET_FREE(curr_flpfcb->qdh,
					     QDH_FREE(curr_flpfcb->qdh) -
						     QDH_SPB(curr_flpfcb->qdh));
				curr_flpfcb->si->changed = true;
				return GetSector(k * QDH_SPB(curr_flpfcb->qdh),
						 fileNum);
			}
			p += 6;
		}
	}
	gError = QERR_DF;
	return nil;
}

static OSErr QLWA_KillFile(FileNum fileNum)
{
	uw16 *p;
	int rels = 1;

	p = (uw16 *)(curr_flpfcb->buffer + 0x40);

	for (p += fileNum.file; *p; rels++)
		p = (uw16 *)(curr_flpfcb->buffer + 0x40) + (uw16)(RW(p));

	WW(p, QWA_FFC(curr_flpfcb->qdh));

	/* mark FAT as changed */
	curr_flpfcb->si[0].changed = true;
	curr_flpfcb->si[((char *)p - curr_flpfcb->buffer) >> 9].changed = true;

	QWA_SETFFC((curr_flpfcb->qdh), fileNum.file);

	QWA_SETFC(curr_flpfcb->qdh, QWA_FC(curr_flpfcb->qdh) + rels);

	return WriteBlock0(); /* not much use to delay */
}

static OSErr KillFile(FileNum fileNum)
{
	register uw8 *p;
	register w16 i, n;

	if (curr_flpfcb->DiskType == qlwa) {
		return QLWA_KillFile(fileNum);
	} else {
		n = (QDH_TOTAL(curr_flpfcb->qdh) / QDH_SPB(curr_flpfcb->qdh)) >>
		    1;
		p = (uw8 *)(curr_flpfcb->buffer + 96);
		for (i = 0; i < n; i++) {
			if ((RW((w16 *)p) >> 4) == fileNum.file) {
				*p = 0xfd;
				QDH_SET_FREE(curr_flpfcb->qdh,
					     QDH_FREE(curr_flpfcb->qdh) +
						     QDH_SPB(curr_flpfcb->qdh));
			}
			if ((((w16)p[3] << 4) | (p[4] >> 4)) == fileNum.file) {
				p[3] = 0xfd;
				QDH_SET_FREE(curr_flpfcb->qdh,
					     QDH_FREE(curr_flpfcb->qdh) +
						     QDH_SPB(curr_flpfcb->qdh));
			}
			p += 6;
		}
	}
	return WriteBlock0(); /* not clean, must be called */
}

/* remove block allocations , nBlock=512 bytes    */
/* keep only the first nBlock+1 (0 to nBlock) of the file */
/* called as (fn,0) from mdv_doopen() */
OSErr KillFileTail(FileNum fileNum, int nBlock)
{
	register uw8 *p;
	register w16 i, n;
	Cond changed = false;

	if (curr_flpfcb->DiskType == qlwa)
		return QLWA_KillFileTail(fileNum, nBlock);

	n = (QDH_TOTAL(curr_flpfcb->qdh) / QDH_SPB(curr_flpfcb->qdh)) >> 1;

	nBlock = (nBlock + QDH_SPB(curr_flpfcb->qdh) - 1) /
		 QDH_SPB(curr_flpfcb->qdh);

	p = (uw8 *)(curr_flpfcb->buffer + 96);
	for (i = 0; i < n; i++) {
		if ((RW((w16 *)p) >> 4) == fileNum.file) {
			if (((w16)p[2] | (((w16)p[1] & 15) << 8)) >= nBlock) {
				*p = 0xfd;
				QDH_SET_FREE(curr_flpfcb->qdh,
					     QDH_FREE(curr_flpfcb->qdh) +
						     QDH_SPB(curr_flpfcb->qdh));
				changed = true;
			}
		}
		if ((((w16)p[3] << 4) | (p[4] >> 4)) == fileNum.file) {
			if ((RW((w16 *)(p + 4)) & 0x0fff) >= nBlock) {
				p[3] = 0xfd;
				QDH_SET_FREE(curr_flpfcb->qdh,
					     QDH_FREE(curr_flpfcb->qdh) +
						     QDH_SPB(curr_flpfcb->qdh));
				changed = true;
			}
		}
		p += 6;
	}
	return changed ? WriteBlock0() : 0;
}

/* get file blocks */

static Ptr GetFileSector(FileNum fileNum, int sector)
{
	int s;
	s = FileBlockSector(fileNum, sector);
	/*printf("access sector %d\n",s);*/
	if (s >= 0)
		return GetSector(s, fileNum);

	if (!gError)
		gError = QERR_BM; /* HACK */
	return nil;
}

/* directory */

struct fileHeader *GetFileHeader(FileNum fileNum)
{
	Ptr p;
	FileNum fx;

	if (fileNum.file < 0)
		CustomErrorAlert("Bad file number in GetFileHeader");

#if 0
  if (curr_flpfcb->DiskType==qlwa)
    {
      int i=0;
      Ptr e;

      while (p=GetFileSector(fx,i))
	{
	  for (e=p;e-p<512;e+=64)
	    if (QWDE_FNUM(e)==fileNum.file)
	      return (struct fileHeader*)e;
	}
    }
  else
#endif
	{
		fx.dir = 0; /* fake */
		fx.file = fileNum.dir;

		p = curr_flpfcb->lastSector =
			GetFileSector(fx, fileNum.entrynum >> 3);
		if (p != nil)
			p += (fileNum.entrynum & 7) << 6;
	}

	return (struct fileHeader *)p;
}

void RewriteHeader(void)
{
	FileNum fn;

	fn.dir = fn.file = 0;
	if (curr_flpfcb->lastSector != nil) {
		PutSector(curr_flpfcb->lastSector);
		curr_flpfcb->lastSector = nil;
		/*FlushSectors(); */ /* delay it..*/
	}
}

/* should return type FileNumber ...*/
static FileNum Sub_FileNumber(uw8 *name, FileNum DirFn, int DirLen, int isdir,
			      int *nlen)
{
	struct fileHeader *h;
	uw32 *z;
	uw16 i, j, n, l;
	uw8 *p1, *p2;
	Ptr p;
	Cond free;
	FileNum res;

	res.file = 0;
	res.entrynum = -1;
	res.dir = DirFn.file;

	/*printf("Sub_FileNumber %s, DirFileNumber %d DirLen %d\n",name,DirFn,DirLen);*/

	p = GetFileSector(DirFn, 0);
	/*  write(1,p,512); */

	if (p == nil)
		return res;
	h = (struct fileHeader *)(p + 64);
	l = (uw16)RW((uw16 *)name);
	/*  printf("FileNumber: name %s, len %d\n",name+2,l);*/

	if (l > 36)
		l = 36;
	n = DirLen;
	/*printf("FileNumber: num of directory entries : %d\n",n);*/
	for (i = 1; i < n; i++) {
		if ((i & 7) == 0) {
			h = (struct fileHeader *)(p = GetFileSector(DirFn,
								    i >> 3));
			if (p == nil)
				return res;
		}
		z = (uw32 *)h;
		j = 16;
		free = (RL(z) == 0);

		if (!free /* skip free entries */
		    /* directories only if so requested */
		    && (!isdir || (GET_FTYP(h) == 255))) {
			/* printf("entry at %d, name %s, len %d\n",h,REF_FNAME(h)+2,RW((Ptr)REF_FNAME(h)));*/
			if ((uw16)RW((uw16 *)(REF_FNAME(h))) <= l) {
				/*printf("length matches \n");*/
				p1 = name + 2;
				p2 = (uw8 *)REF_FNAME(h) + 2;
				for (j = 0; j < l; j++)
					if ((*p1++ & 223) != (*p2++ & 223))
						break;
				*nlen = j; /* guess real *** name length */
				if ((l == j &&
				     l > 0) || /* exact name match or ...*/
				    /* dangerously brain damaged FS2 semantics */
				    /* - this opens a direcotry through normal open */
				    (j == l - 1 && l > 0 &&
				     GET_FTYP(h) == 255 && *(--p1) == '_')) {
					res.entrynum = i;
					if (curr_flpfcb->DiskType == qlwa)
						res.file = QWDE_FNUM(h);
					else
						res.file = i;
					return res;
				}
				/* next try search in subdir */
				if (j > 0 && *(name + 2 + j) == '_' &&
				    j == RW(REF_FNAME(h)) &&
				    GET_FTYP(h) == 255) {
					FileNum subDirFn;

					subDirFn.dir = DirFn.file;
					subDirFn.file = QWDE_FNUM(h);
					subDirFn.entrynum = i;

					/* see qflen WARNING */
					res = Sub_FileNumber(name, subDirFn,
							     qfLen(subDirFn),
							     isdir, nlen);
					if (res.entrynum > -1) /* found */
						return res;
					if (isdir)
						return subDirFn;
					else
						return res;
				}
			}
		}
		h = (struct fileHeader *)(64 + (uintptr_t)h);
	}
	if (isdir) {
		*nlen = 0;
		return root_fn();
	}

	return res;
}

/* scan directory looking for file name. Returns file number or zero if not found or -1 if error */
FileNum FileNumber(uw8 *name, int isdir, int *nlen)
{
	FileNum DirFn;
	int DirLen;
	/*FileNum fn;*/

	DirFn = root_fn();
	DirLen = root_flen() / 64;

	if (isdir && RW(name) == 0)
		return DirFn;
	else
		return Sub_FileNumber(name, DirFn, DirLen, isdir, nlen);
}

int QDiskOpenDir(struct mdvFile *f)
{
	FileNum fn;
	int nlen;

	fn.dir = 0;

	curr_flpfcb = qdevs[GET_FILESYS(f)].flpp[GET_DRIVE(f)];

	if (!curr_flpfcb || curr_flpfcb->buffer == nil || !curr_flpfcb->isValid)
		if (QFOpenDisk(f) < 0)
			return QERR_NF;

			/*printf("opendir %s\n",NAME_REF(f)+2);*/

#if 0
   fn=root_fn();
   SET_FNUMBER(f,fn);
#else
	SET_FNUMBER(f, FileNumber((uw8 *)NAME_REF(f), 1, &nlen));
#endif
	WW(NAME_REF(f), nlen);
	SET_REF(f, (unsigned)-1);
	(curr_flpfcb->file_count)++;
	return 0;
}

static OSErr QLWA_CreateNewFile(struct mdvFile *f)
{
	uw32 *z;
	uw16 i, j, n, l;
	Ptr p;
	struct fileHeader *h;
	Cond free;
	int blockEntries;
	long time;
	FileNum fn0, fe;
	int dummy;

	if (!curr_flpfcb || curr_flpfcb->buffer == nil || !curr_flpfcb->isValid)
		QFOpenDisk(f);

#if 0
  fn0=root_fn();
#else
	fn0 = FileNumber((uw8 *)NAME_REF(f), 1, &dummy);
#endif

	if (QWA_FC(curr_flpfcb->qdh) < 1)
		return QERR_DF;

	blockEntries = QWA_SPC(curr_flpfcb->qdh) << 3; /* SPC*512/64 */
	gError = 0;

	n = 1 + (qfLen(fn0) >> 6); /* see warning */

	p = GetFileSector(fn0, 0);
	if (p == nil)
		return gError;

	z = (uw32 *)(p + 64);
	//for(i=1; i<n; i++,(char*)z+=64)
	for (i = 1; i < n; i++, z += 16) {
		if ((i & 7) == 0) /* get new sector of directory ... */
		{
			p = GetFileSector(fn0, i >> 3);
			if (p == nil)
				return gError;
			z = (uw32 *)p;
		}
		free = (RL(z) == 0);
		/*for(j=0;j<16;j++) if(RL(z++)!=0) free=false;*/
		if (free)
			goto slotFound;
	}

	if ((n % blockEntries) == 0) /* search ended on cluster boundary ?*/
	{
		if (QWA_FC(curr_flpfcb->qdh) < 2 /**QWA_SPC(curr_flpfcb->qdh)*/)
			return QERR_DF;
		p = GetFreeBlock(fn0,
				 n / blockEntries); /* alloc new cluster !!! */
		if (p == nil)
			return gError;
		z = (uw32 *)p;
		for (j = 0; j < 128; j++)
			WL(z++, 0);
	} else /* make sure p is the last sector of directory file */
	{
		p = GetFileSector(fn0, n >> 3);
		if (p == nil)
			return gError;
		z = (uw32 *)(p + ((n & 7) << 6));
		for (j = 0; j < 16; j++)
			WL(z++, 0);
	}
	if (fn0.dir == 0)
		QWA_SETRLEN(curr_flpfcb->qdh, (n + 1) << 6);
	else {
		struct fileHeader *hh;

		hh = GetFileHeader(fn0);
		if (hh != nil) {
			SET_FLEN(hh, ((n + 1) << 6));
			RewriteHeader();
		} else
			return gError;
	}

	i = n;
	PutSector(p); /* mark as changed */
	curr_flpfcb->si->changed = true;
slotFound:
	fe.dir = fn0.file;
	fe.entrynum = i;

	p = QLWA_GetFreeBlock0(&fe); /* alloc cluster#0 of file */

	SET_FNUMBER(f, fe);

	if (p == nil)
		goto errore;

	z = (uw32 *)p;
	for (j = 0; j < 16; j++)
		WL(z++, 0); /* write header copy ???? */
	PutSector(p);

	p = GetFileSector(fn0, i >> 3); /* get sector containing direntry */
	if (p == nil)
		goto errore;

	h = (struct fileHeader *)(p +
				  ((i & 7) << 6)); /* setup direcory entry */

	for (i = 0; i < 16; i++)
		WL((char *)h + i * 4, 0);
	SET_FLEN(h, 64);
	l = (uw16)RW((uw16 *)NAME_REF(f)) + 2;
	if (l > 36)
		l = 36;
	BlockMoveData(NAME_REF(f), REF_FNAME(h),
		      (uw16)RW((uw16 *)NAME_REF(f)) + 2);
	time = ReadQlClock();
	SET_FDTBAC(h, time);
	SET_FDUPDT(h, time);
	SET_REF(f, (unsigned)time);
	QWDE_SETFNUM(h, fe.file);

	PutSector(p); /* and write back directory */
errore:
#if 0 /* currently a flush is too expensive, delay it */
  /*WriteBlock0();*/                  /* .... FAT */
  FlushSectors();                   /* flush all sectors */
  /*  if(gError==0) FlushFile(fe);*/
#endif
	return gError;
}

static OSErr CreateNewFile(struct mdvFile *f)
{
	uw32 *z;
	uw16 i, j, n, l;
	Ptr p;
	struct fileHeader *h;
	Cond free;
	int blockEntries;
	long time;
	FileNum fn0, fe;

	if (!curr_flpfcb || curr_flpfcb->buffer == nil || !curr_flpfcb->isValid)
		QFOpenDisk(f);

	if (curr_flpfcb->DiskType == qlwa)
		return QLWA_CreateNewFile(f);

	fn0 = root_fn();

	if (QDH_FREE(curr_flpfcb->qdh) < 1)
		return QERR_DF;
	blockEntries = QDH_SPB(curr_flpfcb->qdh) << 3;
	gError = 0;
	p = GetFileSector(fn0, 0);
	if (p == nil)
		return gError;
	n = (QDH_DIREOFBL(curr_flpfcb->qdh) << 3) +
	    (QDH_DIREOFBY(curr_flpfcb->qdh) >> 6);
	z = (uw32 *)(p + 64);
	for (i = 1; i < n; i++) {
		if ((i & 7) == 0) /* get new sector of directory ... */
		{
			p = GetFileSector(fn0, i >> 3);
			if (p == nil)
				return gError;
			z = (uw32 *)p;
		}
		free = true;
		for (j = 0; j < 16; j++)
			if (RL(z++) != 0)
				free = false;
		if (free)
			goto slotFound;
	}
	if ((n % blockEntries) == 0) /* search ended on cluster boundary ?*/
	{
		if (QDH_FREE(curr_flpfcb->qdh) < 2)
			return QERR_DF;
		p = GetFreeBlock(fn0,
				 n / blockEntries); /* alloc new cluster !!! */
		if (p == nil)
			return gError;
		z = (uw32 *)p;
		for (j = 0; j < 128; j++)
			WL(z++, 0);
	} else /* make sure p is the last sector of directory file */
	{
		p = GetFileSector(fn0, n >> 3);
		if (p == nil)
			return gError;
		z = (uw32 *)(p + ((n & 7) << 6));
		for (j = 0; j < 16; j++)
			WL(z++, 0);
	}
	QDH_SET_DIREOFBL(curr_flpfcb->qdh, n >> 3);
	QDH_SET_DIREOFBY(curr_flpfcb->qdh, ((n & 7) + 1) << 6);
	i = n;
	PutSector(p); /* mark as changed */
	curr_flpfcb->si->changed = true;
slotFound:
	fe.dir = 0;
	fe.file = i;
	fe.entrynum = i;

	SET_FNUMBER(f, fe);
	p = GetFreeBlock(fe, 0); /* alloc cluster#0 of file */
	if (p == nil)
		goto errore;
	z = (uw32 *)p;
	for (j = 0; j < 16; j++)
		WL(z++, 0); /* write header copy ???? */
	PutSector(p);
	p = GetFileSector(fn0, i >> 3); /* get sector containing direntry */
	if (p == nil)
		goto errore;
	h = (struct fileHeader *)(p +
				  ((i & 7) << 6)); /* setup direcory entry */
	SET_FLEN(h, 64);
	l = (uw16)RW((uw16 *)NAME_REF(f)) + 2;
	if (l > 38)
		l = 38;
	BlockMoveData(NAME_REF(f), REF_FNAME(h),
		      (uw16)RW((uw16 *)NAME_REF(f)) + 2);
	time = ReadQlClock();
	SET_FDTBAC(h, time);
	SET_FDUPDT(h, time);
	SET_REF(f, (unsigned)time);
	PutSector(p); /* and write back directory */
errore:
	/*WriteBlock0();*/ /* .... FAT */
	/* not so expensive like for QLWA files so don't delay.. */
	FlushSectors(); /* flush all sectors */
	/*if(gError==0) FlushFile(fe);*/
	return gError;
}

/* disk files */

/* DANGEROUS - replaces buffer !! */
w32 qfLen(FileNum fn)
{
	struct fileHeader *h;

#if 0
  if (fn.dfn==0 ||
      ( fn.dir==0 && curr_flpfcb->DiskType==qlwa &&
	fn.file==QWA_ROOT(curr_flpfcb->qdh) ) )   /* root directory requires special treatment */
    {
      if (curr_flpfcb->DiskType==qlwa)
	return QWA_RLEN(curr_flpfcb->qdh)-64;
      else
	return ((w32)QDH_DIREOFBL(curr_flpfcb->qdh)<<9)+QDH_DIREOFBY(curr_flpfcb->qdh)-64;
    }
#endif

	if (fn.file == root_fn().file)
		return root_flen() - 64;

	h = GetFileHeader(fn);
	if (h != nil)
		return GET_FLEN(h) - 64;
	CustomErrorAlert("File not found (looking for file length)");
	return 0;
}

w32 QDiskLen(struct mdvFile *f)
{
	/*  struct fileHeader *h;*/
	FileNum fn;

	curr_flpfcb = qdevs[GET_FILESYS(f)].flpp[GET_DRIVE(f)];

	if (!curr_flpfcb || curr_flpfcb->buffer == nil || !curr_flpfcb->isValid)
		if (QFOpenDisk(f) < 0)
			return -1;

	if (curr_flpfcb->buffer == nil || !curr_flpfcb->isValid) {
		ErrorAlert(ERR_UNINITIALIZED_DISK);
		return 0;
	}

	fn = GET_FNUMBER(f);

	return qfLen(fn);
}

static OSErr QWrite(struct mdvFile *f, Ptr p, uw32 *n)
{
	OSErr e = 0;
	uw32 l;
	uw32 nn;
	Ptr s;
	struct fileHeader *h;

	if (GET_ISDIR(f)) {
		CustomErrorAlert("Writing to directory File");
		*reg = QERR_RO; /* not implemented */
		return 0;
	}
	nn = *n;
	*n = 0;
	while (nn > 0 && e == 0) {
		s = GetFileSector(GET_FNUMBER(f), GET_POS(f) >> 9);
		if (s == nil) {
			if (gError == ERR_NO_FILE_BLOCK) {
				s = GetFreeBlock(GET_FNUMBER(f),
						 (GET_POS(f) >> 9) /
							 sect_per_cluster());
			}
		}
		if (s != nil) {
			l = 512 - (GET_POS(f) & 511);
			if (nn < l)
				l = nn;
			BlockMoveData(p, s + (GET_POS(f) & 511), l);
			nn -= l;
			SET_POS(f, GET_POS(f) + l);
			p += l;
			(*n) += l;
			PutSector(s);
		} else
			e = gError;
	}

	/* moved here to improve efficiency ..*/
	if (GET_EOF(f) < GET_POS(f)) {
		SET_EOF(f, GET_POS(f));
		h = GetFileHeader(GET_FNUMBER(f));
		if (h != nil) {
			SET_FLEN(
				h,
				GET_EOF(f)); /* attenzione !! se il file � shared anche gli altri canali aperti dovrebbero venire informati del cambio di lunghezza !! */
			RewriteHeader();
		} else
			e = gError;
	}

	return (*n > 0 ? 0 : e);
}

static OSErr QRead(struct mdvFile *f, Ptr p, uw32 *n, Cond lf, w32 *an)
{
	OSErr e = 0;
	uw32 l, i;
	uw32 nn;
	w8 c;
	w8 *p2;
	Ptr s;

	nn = *n;
	*n = 0;
	while (nn > 0 && e == 0) {
		if (GET_POS(f) < GET_EOF(f)) {
			s = GetFileSector(GET_FNUMBER(f), GET_POS(f) >> 9);
			if (s != nil) {
				l = 512 - (GET_POS(f) & 511);
				if (nn < l)
					l = nn;
				if (lf) {
					i = 0;
					p2 = (w8 *)s + (GET_POS(f) & 511);
					do {
						WriteByte((*an)++, c = *p2++);
						SET_POS(f, GET_POS(f) + 1);
						i++;
					} while (i < l && c != 10 &&
						 GET_POS(f) < GET_EOF(f));
					l = i;
					if (c == 10)
						e = LF_FOUND;
					else if (GET_POS(f) >= GET_EOF(f))
						e = QERR_EOF;
				} else {
					if (GET_POS(f) + l > GET_EOF(f)) {
						l = GET_EOF(f) - GET_POS(f);
						e = QERR_EOF;
					}
					BlockMoveData(s + (GET_POS(f) & 511), p,
						      l);
					p += l;
					SET_POS(f, GET_POS(f) + l);
				}
				nn -= l;
				(*n) += l;
			} else
				e = gError;
		} else {
			SET_POS(f, GET_EOF(f));
			e = QERR_EOF;
		}
	}
	if (lf) {
		if (e == 0)
			e = QERR_BF;
		else if (e == LF_FOUND)
			e = 0;
	} else if (*n > 0)
		e = 0;

	return e;
}

/*#define TEST*/

OSErr QDiskIO(struct mdvFile *f, short op)
{
	struct fileHeader *h;
	uw32 count;
	long from, to;
	short i;
	/*short	e;*/
	w8 *p;
	w16 *w;
	Ptr s;

#ifdef TEST
	printf("QDiskIO op %x, d1 %x, d2 %x, a1 %x\n", op, reg[1], reg[2],
	       aReg[1]);
#endif

	curr_flpfcb = qdevs[GET_FILESYS(f)].flpp[GET_DRIVE(f)];

	if (!curr_flpfcb || curr_flpfcb->buffer == nil || !curr_flpfcb->isValid)
		if (QFOpenDisk(f) < 0) {
			*reg = QERR_BM;
			return -1;
		}

	if (curr_flpfcb->buffer == nil || !curr_flpfcb->isValid)
		return ERR_UNINITIALIZED_DISK;
	if (GET_POS(f) < 0) {
		*reg = -10;
		return 0;
	}

	if (op == 5 || (op == 7 && (uw16)reg[2] > 0) || op == 0x49 ||
	    op == 0x46) {
		if (curr_flpfcb->readonly || GET_KEY(f) == Q_DIR ||
		    GET_KEY(f) == Q_RDONLY) {
			*reg = QERR_RO;
			return -1;
		} else
			SET_REF(f, ReadQlClock());
	}

	*reg = 0;

	switch (op) {
	case 0: /* check for pending input */
		if (GET_POS(f) >= GET_EOF(f))
			*reg = -10;
		break;
	case 1: /* fetch byte */
		if (GET_POS(f) < GET_EOF(f)) {
			s = GetFileSector(GET_FNUMBER(f), GET_POS(f) >> 9);
			if (s == nil)
				*reg = gError;
			else {
				*((char *)reg + 4 + RBO) =
					*((char *)(s + (GET_POS(f) & 511)));
				SET_POS(f, GET_POS(f) + 1);
			}
		} else
			*reg = QERR_EF;
		break;
	case 2: /* fetch LF-terminated line */
		count = (uw16)reg[2];

		reg[0] = QRead(f, nil, &count, true, aReg + 1);
		reg[1] = count;

		/*printf("io.fline res: %d, count= %d\n",*reg,count);*/
		break;
	case 3: /* fetch string */
		/* printf("File %x Pos %d, File len %d\n",f,GET_POS(f),GET_EOF(f));*/
		from = aReg[1];
		to = from + ((uw16)reg[2]);
		if (from < 131072) {
			SET_POS(f, GET_POS(f) + 131072 - from);
			from = 131072;
		}
		if (to >= RTOP)
			to = RTOP;
		count = to - from;
		if (count > 0) {
			reg[0] = QRead(f, (Ptr)memBase + from, &count, false,
				       nil);
			/*	printf("io.fstrg res: %d\n",e);*/

			to = from + count;
			reg[1] = count;
			aReg[1] = to;
			ChangedMemory(from, to);
		} else
			reg[1] = 0;
		break;
	case 5: /* send byte */
		count = 1;
		reg[0] = QWrite(f, (Ptr)reg + 4 + RBO, &count);
		break;
	case 7: /* send string */
		count = (uw16)reg[2];
		reg[0] = QWrite(f, (Ptr)memBase + aReg[1], &count);
		reg[1] = count;
		aReg[1] += count;
		break;
	case 0x40: /* check pending ops */
	case 0x41: /* flush buffers */
#if 1
		FlushSectors();
#else
		FlushFile(GET_FNUMBER(f));
		FlushFile(root_fn());
		FlushFile(fat_fn());
#endif
		break;
	case 0x43: /* position file pointer relative */
		reg[1] += GET_POS(f) - 64;
	case 0x42: /* position file pointer absolute */
		if (reg[1] < 0)
			reg[1] = 0;
		else if (reg[1] > GET_EOF(f) - 64) {
			reg[1] = GET_EOF(f) - 64;
			*reg = QERR_EF;
		}
		SET_POS(f, reg[1] + 64);
		/* printf("File %x Pos %d, File len %d\n",f,GET_POS(f),GET_EOF(f)); */
		break;
	case 0x45: /* medium information */
		if (is_qlwa()) {
			reg[1] = ((w32)QWA_FC(curr_flpfcb->qdh) << 16) *
					 sect_per_cluster() +
				 QWA_CC(curr_flpfcb->qdh) * sect_per_cluster();
			p = (w8 *)(curr_flpfcb->qdh) + 10;
			for (i = 0; i < 10; i++)
				WriteByte(aReg[1]++, *p++);
		} else {
			reg[1] = ((w32)QDH_FREE(curr_flpfcb->qdh) << 16) +
				 QDH_TOTAL(curr_flpfcb->qdh);
			p = (w8 *)(curr_flpfcb->qdh) + 4;
			for (i = 0; i < 10; i++)
				WriteByte(aReg[1]++, *p++);
		}
		break;
	case 0x46: /* set file header */
		if (GET_ISDIR(f)) /*(GET_FNUMBER(f).dfn==0)*/
		{ /*Writing directory sector header*/
			reg[1] = 0;
			*reg = -19; /* not implemented */
		}
		h = GetFileHeader(GET_FNUMBER(f));
		if (h == nil) {
			reg[1] = 0;
			*reg = QERR_BM; /* bad medium */
		} else {
			uw32 a1 = aReg[1];
			w = (w16 *)((Ptr)h + 4);
			aReg[1] += 4;
			for (i = 0; i < 5; i++, aReg[1] += 2)
				WW(w++, ReadWord(aReg[1]));
			reg[1] = 14 | (reg[1] & (~0xffff));
			RewriteHeader();
			/*aReg[1]=a1+14;*/
			aReg[1] = a1;
		}
		break;
	case 0x47: /* read file header */
		/*printf("File %x Pos %d, File len %d\n",f,GET_POS(f),GET_EOF(f)); */

		count = reg[2] & 0x0fffe;
		if (count > 64)
			count = 64;
		if (count < 4)
			count = 4;
		if (GET_FNUMBER(f).file == root_fn().file) {
			char *p = (char *)memBase + aReg[1];

			reg[1] = count;
			count = (count - 4) >> 1;

			WriteLong(aReg[1], QDiskLen(f));
			aReg[1] += 4;

			while (count--) {
				WriteWord(aReg[1], 0);
				aReg[1] += 2;
			}
			if (reg[1] > 5)
				SET_FTYP(p, 255);
		} else {
			h = GetFileHeader(GET_FNUMBER(f));
			if (h == nil)
				*reg = gError;
			else {
				reg[1] = count;
				count = (count - 4) >> 1;
				w = (w16 *)h;
				WriteLong(aReg[1], RL((w32 *)w) - 64);
				aReg[1] += 4;
				w += 2;
				while (count--) {
					WriteWord(aReg[1], (uw16)RW(w++));
					aReg[1] += 2;
				}
			}
		}
		/* printf("File %x Pos %d, File len %d\n",f,GET_POS(f),GET_EOF(f));*/
		break;
	case 0x48: /* read file into memory */
		from = aReg[1];
		to = from + reg[2];
		if (from < 131072) {
			SET_POS(f, GET_POS(f) + 131072 - from);
			from = 131072;
		}
		if (to >= RTOP)
			to = RTOP;
		count = to - from;

		if (count > 0) {
			*reg = QRead(f, (Ptr)memBase + from, &count, false, nil);
			to = from + count;
			aReg[1] = to;
			ChangedMemory(from, to);
		}
		break;
	case 0x49: /* save file from memory */
		count = reg[2];
		*reg = QWrite(f, (Ptr)memBase + aReg[1], &count);
		aReg[1] += count;
		break;
	case 36: /* clear righthand end of cursor line (viene mandato dal Basic durante un save) */
	case 10: /* read cursor position (viene mandato da ZOO) */
	case 11: /* get window size and cursor position */
	case 19: /* decrement cursor */
	case 20: /* increment cursor */
		*reg = -15; /* mandati da tabelleneu_obj mentre legge un file */
		break;
	case 75: /* truncate file to current position */
		SET_EOF(f, GET_POS(f));
		h = GetFileHeader(GET_FNUMBER(f));
		if (h == nil)
			*reg = gError;
		else {
			SET_FLEN(h, GET_EOF(f));
			RewriteHeader();
			*reg = KillFileTail(GET_FNUMBER(f),
					    (GET_EOF(f) + 511) >> 9);
		}
		break;

	case 76:
		/* FS.DATE */
		/* D1=-1 read, 0 current time, >0 secs count */
		/* D2.b 0=Update, 1=Backup */
		/* returns D1 = date */
		h = GetFileHeader(GET_FNUMBER(f));
		if (reg[1] < 0) {
			if ((uw8)(reg[2]))
				reg[1] = GET_FDTBAC(h);
			else
				reg[1] = GET_FDUPDT(h);
		} else {
			if (reg[1] == 0)
				reg[1] = ReadQlClock();
			if ((uw8)(reg[2]))
				SET_FDTBAC(h, ReadQlClock());
			else
				SET_FDUPDT(h, ReadQlClock());
			RewriteHeader();
		}
		*reg = 0;
		break;
	case 74:
	case 77:
		*reg = -15;
		break;
	case 78:
		/*  FS.VERS : read/write file version  */
		/*  D1=0  read, >0 set to this version */
		/*  D1=-1 read, increment version      */
		/*  returns D1 = vers */

		h = GetFileHeader(GET_FNUMBER(f));
		if (reg[1] == 0) {
			reg[1] = GET_FVER(h);
		} else {
			if (reg[1] == -1) {
				reg[1] = GET_FVER(h);
				SET_FVER(h, reg[1] + 1);
			} else
				SET_FVER(h, reg[1]);

			RewriteHeader();
		}
		break;
	case 79:
		/* FX.XINF */
		/* A1 ptr to parblk */
		/* A1 unchanged */
#if 1
	{
		short nl;
		char *p;
		long free, mxs;

		memset((char *)memBase + aReg[1] + 0, -1, 64);
		if (GET_FILESYS(f) >= 0) {
			strncpy((char *)memBase + aReg[1] + 2,
				(p = qdevs[GET_FILESYS(f)]
					     .mountPoints[GET_DRIVE(f)]),
				20);
		} else {
			strcpy((char *)memBase + aReg[1] + 2,
			       (p = "uQVFSx root"));
		}
		nl = strlen(p);
		WriteWord(aReg[1], min(20, nl));
		nl = strlen(qdevs[GET_FILESYS(f)].qname);
		WriteWord(aReg[1] + 0x16, nl);
		strncpy((char *)memBase + aReg[1] + 0x18,
			qdevs[GET_FILESYS(f)].qname, nl);
		free = is_qlwa() ? QWA_FC(curr_flpfcb->qdh) :
					 QDH_FREE(curr_flpfcb->qdh);
		mxs = is_qlwa() ? QWA_CC(curr_flpfcb->qdh) :
					QDH_TOTAL(curr_flpfcb->qdh);
		*((char *)memBase + aReg[1] + 0x1c) = 1 + GET_DRIVE(f);
		*((char *)memBase + aReg[1] + 0x1d) = 0;
		WriteWord(aReg[1] + 0x1e, 1024);
		WriteLong(aReg[1] + 0x24, free * sect_per_cluster() / 2);
		WriteLong(aReg[1] + 0x20, mxs * sect_per_cluster() / 2);
		WriteLong(aReg[1] + 0x28, 64);
	}
#else
		*reg = -15;
#endif
	break;
	default:
		*reg = -15;
		break;
	}

#ifdef TEST
	printf("QDiskIO exit: d0 %d, d1 %x, a1 %x\n", reg[0], reg[1], aReg[1]);
#endif /*TEST*/

	return 0;
}

OSErr QDiskDelete(
	struct mdvFile *
		f) /* struct mdvFile passed, but only the name is valid (not the fNumber) and the drive */
{
	FileNum fileNum;
	w16 perm;
	uw32 *z;
	int i, err, dummy;

	curr_flpfcb = qdevs[GET_FILESYS(f)].flpp[GET_DRIVE(f)];

	if (!curr_flpfcb || curr_flpfcb->buffer == nil || !curr_flpfcb->isValid)
		if (QFOpenDisk(f) < 0)
			return QERR_NF;

	if (curr_flpfcb->buffer == nil || !curr_flpfcb->isValid)
		return ERR_UNINITIALIZED_DISK;
	SET_FNUMBER(f, fileNum = FileNumber((uw8 *)NAME_REF(f), 0, &dummy));
	if (fileNum.file > 0) {
		err = 0;
		SET_OPEN(f, false);
		if (FileAlreadyOpen(GET_DRIVE(f), GET_FILESYS(f), fileNum,
				    &perm)) {
			err = QERR_IU;
		} else {
			z = (uw32 *)GetFileHeader(fileNum);
			if (z) {
				/* currently directories can't be deleted, would require
	     * check fn!=root_fn & scanning dir conts for valid entries */
				if (GET_FTYP(z) == 255 /*&& RL(z)>64*/) {
					err = QERR_IU;
				} else {
					for (i = 0; i < 16; i++)
						WL(z++, 0);
					RewriteHeader();
					gError = KillFile(fileNum);
#if 1
					FlushSectors();
#else
					WriteBlock0(); /* .... FAT */
					FlushFile(
						root_fn()); /* flush all sectors */
					FlushFile(fileNum);
#endif
				}
			}
		}
	} else {
		err = QERR_NF;
	}
	/*printf("QDiskDelete returns %d\n",err);*/

	if (err == 0)
		curr_flpfcb->lastclose = time(NULL);

	return err;
}

OSErr QDiskOpen(struct mdvFile *f, int drive, Cond canExist, Cond canCreate)
{
	int res;
	FileNum fileNum;
	w16 perm;
	unsigned char c;
	int dummy;

	curr_flpfcb = qdevs[GET_FILESYS(f)].flpp[GET_DRIVE(f)];
	SET_REF(f, (unsigned)-1);

	if (!curr_flpfcb || curr_flpfcb->buffer == nil || !curr_flpfcb->isValid)
		if (QFOpenDisk(f) < 0)
			return QERR_NF;

	res = 0;

	gError = 0;
	fileNum = FileNumber((uw8 *)NAME_REF(f), 0, &dummy);
	/*  printf("QDiskOpen: file %s, filenum %d canexist %d cancreate %d\n",NAME_REF(f)+2, fileNum,canExist,canCreate);*/

	if (fileNum.file >= 0) {
		if (fileNum.file == 0) /* file doesn't exist */
		{
			res = QERR_NF; /* file not found */
			if (canCreate) {
				gError = CreateNewFile(f);
				if (gError == 0) {
					res = 0;
					fileNum = GET_FNUMBER(f);
				} else if (gError == QERR_DF) {
					gError = 0;
					res = -11; /* drive full */
				}
			}
		} else /* file exists */
		{
			if (!canExist)
				res = -8; /* already exists */
			else {
				if (FileAlreadyOpen(
					    GET_DRIVE(f), GET_FILESYS(f),
					    fileNum,
					    &perm)) { /* File already open: check for file in use if either or both don't have shared permission !! */
					if (perm != 1 || GET_KEY(f) != 1)
						res = QERR_IU;
				}
			}
		}
		if (res == 0) {
			SET_FNUMBER(f, fileNum);
			SET_ISDIR(f, false);
			(curr_flpfcb->file_count)++;
		}
	} else {
		debug("Can't read directory block");
		res = QERR_BM; /* bad medium */
	}
	return res;
}

void QDiskClose(struct mdvFile *f)
{
	struct fileHeader *h;

	curr_flpfcb = qdevs[GET_FILESYS(f)].flpp[GET_DRIVE(f)];

	if (!curr_flpfcb || curr_flpfcb->buffer == nil || !curr_flpfcb->isValid)
		if (QFOpenDisk(f) < 0)
			return; /*QERR_NF*/

	if (GET_KEY(f) != Q_RDONLY && GET_KEY(f) != Q_DIR &&
	    (GET_REF(f) != (unsigned)-1)) {
		h = GetFileHeader(GET_FNUMBER(f));
		if (!GET_ISDIR(f)) {
			if (h) {
				/* SET_FDUPDT(h,ReadQlClock()); */
				SET_FDUPDT(h, GET_REF(f));
				RewriteHeader();
			} else
				printf("Header not found?!?\n");
		}
	}
	if (curr_flpfcb->buffer != nil /*&& GET_FNUMBER(f).dfn!=0*/) {
#if 1
		FlushSectors();
#else
		FlushFile(GET_FNUMBER(f));
		FlushFile(root_fn());
		FlushFile(fat_fn());
#endif
	}
	(curr_flpfcb->file_count)--;
	if (curr_flpfcb->file_count < 0)
		printf("file count<0 ?!?!?!?\n");
	curr_flpfcb->lastclose = time(NULL);
}

/* altre interfacce */

/* check and fix logical to physical translation table */

static void FixLogical(uw8 *ltp, Cond ql5a)
{
	Cond t[36];
	short s, n, i, x;
	Cond error;

	n = (s = ql5a ? 9 : 18) << 1; /* wow! what a nice cunning expression */
	for (i = 0; i < n; i++)
		t[i] = false;
	error = false;
	for (i = 0; i < n; i++) {
		x = ltp[i] & 127;
		if (x >= s) {
			error = true;
			break;
		}
		if ((ltp[i] & 0x80) != 0)
			x += s;
		if (t[x]) {
			error = true;
			break;
		}
		t[x] = true;
	}
	if (error)
		BlockMoveData(ql5a ? format_table_dd : format_table_hd, ltp,
			      36);
}

/***********************************************************************/

/* eject the disk */
#if 0
OSErr QDiskEject(void)
{
  OSErr e=0;
  short x;
  int filesys=1;

  if(curr_flpfcb->isDisk)
    {
      if(curr_flpfcb->buffer!=nil)
      {
	if(qdevs[filesys].OpenFiles[flpSlot])
	{
	  x=CautionAlert(rOpenFiles,nil);
	  if(x!=1) return 0;
	  CloseAllMdvFiles(flpSlot,filesys);
	  qdevs[filesys].OpenFiles[flpSlot]=0;
	}
	if(mdvOn==flpSlot+1) StopMotor();
	qdevs[filesys].Present[flpSlot]=false;
	qdevs[filesys].Where[flpSlot]=false;
	/*    InvalWindow(mdvWindow); */
	FlushSectors();
	e=DiskEject(driveNumber);
	curr_flpfcb->isDisk=false;
      }
    }
  return e;
}
#endif

/* bad disk inserted event (Macintosh) */

#if 0
Boolean MyBadDisk(short mountError,short drive)
{	/* an unreadable disk has been inserted. Deal with the event and return true if you succeed,
	   or false if you want the default action (ask the user to format the disk) to be taken */
  /* Note: the Blank library always call the routine with an error in mountError, but mountError is zero when the routine is called by the 'format disk' dialog */
  short e=0;
  short i,k;
  Boolean format=true;
  long total,empty;
  char *head="Untitled  xx";
  Boolean editName=false;

  if(buffer==nil) e=AllocateDisk();
  if(e==0) e=OpenDisk(&refNum);	/* get reference number for .Sony disk driver */
  if(e==0)
    {
    install:
      e=LoadSector0();
      if(e==0)
	{
	  if(editName)
	  {
	    if(EditDiskName(QDH_NAME(qdh))) si->changed=true;
	    editName=false;
	  }
	  format=false;
	  i=Alert(rFlpSlot,nil);
	  k=0;
	  if(i==3) e=DiskEject(drive);
	  else
	    {
	      k=i;
	      if(mdvPresent[k-1] && mdvOpenFiles[k-1]!=0) /* check for open files in slot */
	    {
	      i=CautionAlert(rOpenFiles,nil);
	      if(i==1) CloseAllMdvFiles(k-1);
	      else k=0;
	    }
	      driveNumber=drive;
	    }
	  if(k>0)
	    {
	      if(mdvOn==k) StopMotor();
	      k--;
	      mdvPresent[k]=true;
	      mdvOpenFiles[k]=0;
	      mdvWhere[k]= 1;
	      BlockMoveData(QDH_NAME(qdh), mdvHeaders+k*14+2,12);
	      BlockMoveData(QDH_NAME(qdh),mdvName[k]+1,mdvName[k][0]=10);
	      /*	  InvalWindow(mdvWindow); */
	      isDisk=true;
	      flpSlot=k;
	      /*				AdjustSetDirMenu(); */
	    }
	}
    } else format=false;
  if(e==ERR_NO_DRIVE)
    {
      e=0;
      CustomErrorAlert("your drive is not able to read QL-formatted floppy disks");
      format=false;
    }
  else if(format)
    {
      if(e!=0 && e!=-36 && e!=-67 && e!=ERR_NO_QL) ErrorAlert(e);
      isDisk=false;
      format=false;
      if(mountError) i=StopAlert(rChooseFormat,nil);
      else
	{
	  i=theDiskName[0];
	  if(i>10) i=10;
	  for(k=0;k<i;k++) head[k]=theDiskName[k+1];
	  while(i<10) head[i++]=' ';
	  i=3;
	}
      if(i==1) e=DiskEject(drive);
      else
	{
	  if(i==3)
	    {
	      isDisk=true;
	      e=QFormat(drive,(uw8*)head,&total,&empty);
	      isDisk=false;
	      if(e==0)
	  {
	    editName=true;
	    goto install;
	  }
	      e=DiskEject(drive);
	    }
	  else return false;
	}
    }
  if(e!=0) ErrorAlert(e);
  return e==0;
}
#endif /*0*/

#if 0 /* UNUSED - luckilly */
static w16 LogicalSector(uw8 side,uw8 track,uw8 sector)
{
  short i;
  if(side!=0) sector|=0x80;
  for(i=0; i<QDH_SPC(curr_flpfcb->qdh); i++) if(QDH_LTP(curr_flpfcb->qdh)[i]==sector) break;
  return (w16)track*QDH_SPC(curr_flpfcb->qdh)+i;
}
#endif

#if 0
void QDiskFlush(void)
{	if(curr_flpfcb->buffer!=nil) FlushSectors();
}

Cond QDiskPresent(void)
{
  return curr_flpfcb->isDisk;
}

/* drive and format */

static OSErr QFormat(short driveNumber,uw8 *head,long *total,long *empty)
{
  OSErr e;
  short i;
  uw8 *p;
  uw32 *z;
  Cond dd; /* DD floppy? If false the disk is HD */

  e=OpenDisk(driveNumber);	/* get reference number for .Sony disk driver */
  if(e==0)
    {	/* do low level disk format here */
    }
  if(e!=0) return e;

  /*InitDiskTables();*/
  z=(uw32*)curr_flpfcb->buffer;
  curr_flpfcb->fatSectors=dd? 3:6;
  for(i=0;i<128*curr_flpfcb->fatSectors;i++) WL(z++,-1);
  /*qdh=(struct qDiscHeader*)buffer;*/
  if(curr_flpfcb->DiskType=(dd ? floppyDD : floppyHD))
    {
      QDH_SET_VER(curr_flpfcb->qdh,'5A');
      QDH_SET_TOTAL(curr_flpfcb->qdh,1440);
      QDH_SET_SPT(curr_flpfcb->qdh,9);
      QDH_SET_SPC(curr_flpfcb->qdh,18);
      QDH_SET_OFFSET(curr_flpfcb->qdh,5);
      BlockMoveData(format_table_dd,QDH_LTP(curr_flpfcb->qdh),36);
    }
  else
    {	QDH_SET_VER(curr_flpfcb->qdh,'5B');
    QDH_SET_TOTAL(curr_flpfcb->qdh,2880);
    QDH_SET_SPT(curr_flpfcb->qdh,18);
    QDH_SET_SPC(curr_flpfcb->qdh,36);
    QDH_SET_OFFSET(curr_flpfcb->qdh,2);
    BlockMoveData(format_table_hd,QDH_LTP(curr_flpfcb->qdh),36);
    }
  QDH_SET_ID(curr_flpfcb->qdh,'QL');
  BlockMoveData(head,QDH_NAME(curr_flpfcb->qdh),10);	/* copy name */
  QDH_SET_RAND(curr_flpfcb->qdh,Random());			/* set random number */
  QDH_SET_SPB(curr_flpfcb->qdh,3);
  QDH_SET_GOOD(curr_flpfcb->qdh,QDH_TOTAL(curr_flpfcb->qdh));
  QDH_SET_FREE(curr_flpfcb->qdh,QDH_GOOD(curr_flpfcb->qdh)-QDH_SPB(curr_flpfcb->qdh)-curr_flpfcb->fatSectors);
  QDH_SET_UPDTS(curr_flpfcb->qdh,0);
  QDH_SET_TRACKS(curr_flpfcb->qdh,80);
  QDH_SET_DIREOFBL(curr_flpfcb->qdh,0);
  QDH_SET_DIREOFBY(curr_flpfcb->qdh,64);
  p=(uw8*)curr_flpfcb->buffer+96;
  for(i=QDH_TOTAL(curr_flpfcb->qdh)/QDH_SPB(curr_flpfcb->qdh); i>0; i--)
    {	*p=0xfd;
    p+=3;
    }
  p=(uw8*)curr_flpfcb->buffer+96;
  for(i=0;i<curr_flpfcb->fatSectors/QDH_SPB(curr_flpfcb->qdh); i++)
    {	*p++=0xf8;
    *p++=0;
    *p++=0;
    }
  *p++=0;
  *p++=0;
  *p=0;
  for(i=0;i<curr_flpfcb->fatSectors;i++)
    {	curr_flpfcb->si[i].free=false;
    curr_flpfcb->si[i].changed=true;
    curr_flpfcb->si[i].locked=true;
    curr_flpfcb->si[i].logSector=i;
    curr_flpfcb->si[i].fileNum.file=-1;
    }
  /*	FlushFile(-1); */
  p=(uw8*)GetFileSector(0,0);
  if(p!=nil)
    {	z=(uw32*)p;
    for(i=0;i<16;i++) WL(z++,'0000');
    for(i=0;i<112;i++) WL(z++,0);
    PutSector((Ptr)p);
    }
  else e=gError;
  for(i=1;e==0 && i<QDH_SPB(curr_flpfcb->qdh);i++)
    {	p=(uw8*)GetFileSector(0,i);
    if(p==nil) e=gError;
    else
      {	z=(uw32*)p;
      for(i=0;i<128;i++) WL(z++,0);
      PutSector((Ptr)p);
      }
    }
  FlushSectors();
  *total=QDH_TOTAL(curr_flpfcb->qdh);
  *empty=QDH_FREE(curr_flpfcb->qdh);
  return e;
}

void QDiskFormat(uw8 *head,long *total,long *empty)
{
#if 0
  gError=0;

  if(curr_flpfcb->buffer==nil) gError=ERR_UNINITIALIZED_DISK;
  else
    {	if(curr_flpfcb->isDisk)
      {	if(StopAlert(rConfirmFormat,nil)==2)
	{	gError=QFormat(driveNumber,head,total,empty);
	if(gError!=0) *reg=-14;	/* format failed */
	}
      else
	{	FlushSectors();
	gError=DiskEject(driveNumber);
	*reg=-14;	/* format failed */
	}
      } else *reg=-14;
    }
  if(gError!=0) ErrorAlert(gError);
#endif
}
#endif

#if 0
OSErr AllocateDisk(int num)  /* num==0 all native devices */
{
#if 0
  InitDiskTables();

  if (OpenDisk(&refNum)<0)
    {
      return;
    }

  LoadSector0();
  qdh=(struct qDiscHeader*)buffer;
  return 0;
#endif
  OpenDisk(0);
}

static void FlushSectors(void);

void DeallocateDisk(void)
{
#if 0
  if(curr_flpfcb->buffer!=nil)
    {	FlushSectors();
    QDiskEject();
    DisposePtr(buffer);
    buffer=nil;
    }
#endif
}
#endif

#if 0
static void FlushSectors(void)
{
  int i;

  if(curr_flpfcb->buffer!=nil)
    for(i=0;i<NPOOL;i++)
      if(!curr_flpfcb->si[i].free && curr_flpfcb->si[i].changed)
	{	WriteLogSector(curr_flpfcb->si[i].logSector,curr_flpfcb->buffer+((long)i<<9));
	curr_flpfcb->si[i].changed=false;
	}
}
#endif

#if 0
OSErr OpenDisk(long num)
{
  int fd;
  int i,j,k;

  for (i=0; i<MAXDEV; i++)
    for(k=0; k<8; k++)
      if (qdevs[i].Where[k] ==1 ) /*&& qdevs[i].Present[k]*/
	{
	  if (qdevs[i].flpp[k]) continue; /* already allocated */

	  fd=open(qdevs[i].mountPoints[k],O_RDWR);

	  /*  printf("calling OpenDisk, res %d\n",fd);*/

	  qdevs[i].Present[k]= (fd>=0);
	  qdevs[i].flpp[k]= (struct FLP_FCB *)malloc(sizeof (struct FLP_FCB));

	  /**fref=fd;*/
	  (qdevs[i].flpp[k])->refNum=fd;
	  if (fd<0) continue;

	  /*printf("flp%d_ allocated as %s\n",k,qdevs[i].mountPoints[k]);*/

	  curr_flpfcb=qdevs[i].flpp[k];
 	  curr_flpfcb->lastSector=nil;
	  curr_flpfcb->counter=0;

	  LoadSector0();
	  curr_flpfcb->qdh=curr_flpfcb->buffer;
	  /*InitDiskTables();*/
	}

  return 0;

}
#endif
