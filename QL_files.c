/*
 * (c) UQLX - see COPYRIGHT
 */

/*#include "QLtypes.h"*/
#include "QL68000.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "QL.h"
#include "QLfiles.h"
#include "QFilesPriv.h"
#include "QSerial.h"
/*#include "SelectFolder.h"*/
#include "QInstAddr.h"
#include "QDisk.h"
#include "unix.h"
#include "QDOS.h"
#include "qx_proto.h"

#define MDV_ID 0x28b07ae4

extern void rts(void);

extern uw32 mdv_doopen(struct mdvFile *, int, int, int, uw32);
extern int QDiskOpenDir(struct mdvFile *);

w32 floppy_ref, win_ref, mdv_ref;

Str255 flpName[2];

#define STATICDEVS 1
#include "emudisk.h"
short mdvVol[2];
long mdvDir[2];

short mdvOn = 0;
unsigned char mdvHeaders[28];

void FilenameFromQL(unsigned char *fName)
{
	short l, i, j;
	unsigned char c;
	unsigned char *s = "-noASCII-                      ";
	Cond lastAscii = false;
	char *hexDigit = "0123456789ABCDEF";

	return;

	*fName = 0;
	l = fName[1];
	if (l > 31)
		l = fName[1] = 31;
	if (l == 0)
		BlockMoveData("-noname-", fName + 1, 9);
	else { /* examine filename for non ascii characters */
		for (i = 1; i <= l; i++) {
			c = fName[1 + i];
			if (c < 32 || c == ':') /* || c>127 */
			{
				if (l > 7)
					l = 7;
				j = 10;
				for (i = 1; i <= l; i++) {
					c = fName[1 + i];
					if (c < 34 || c > 127 || c == ':') {
						if (i > 1)
							s[j++] = 32;
						if (c > 15)
							s[j++] = hexDigit[(c >>
									   4) &
									  15];
						s[j++] = hexDigit[c & 15];
						lastAscii = false;
					} else {
						if (!lastAscii) {
							s[j++] = 33;
							lastAscii = true;
						}
						s[j++] = c;
					}
				}
				s[0] = j - 1;
				BlockMoveData(s, fName + 1, j);
				break;
			}
		}
	}
}

static void NameToQL(Str255 name)
{
	unsigned char *p;
	short l, j;

	return;

	l = name[0];
	if (l < 10)
		return;
	p = name + 1;
	if (*p++ != '-')
		return;
	if (((*p++) & 223) != 'N')
		return;
	if (((*p++) & 223) != 'O')
		return;
	if (((*p++) & 223) != 'A')
		return;
	if (((*p++) & 223) != 'S')
		return;
	if (((*p++) & 223) != 'C')
		return;
	if (((*p++) & 223) != 'I')
		return;
	if (((*p++) & 223) != 'I')
		return;
	if (*p++ != '-')
		return;
	l -= 9;
	j = 0;
	while (l > 0) {
		if (*p == 33) {
			p++;
			l--;
			while (l > 0 && *p != 32) {
				name[++j] = *p++;
				l--;
			}
			p++;
			l--;
		} else {
			name[++j] = (*p++) - '0';
			if (name[j] > 9)
				name[j] -= 7;
			l--;
			if (l > 0 && *p != 32) {
				name[j] = (name[j] << 4) +
					  (*p > '9' ? *p + 10 - 'A' : *p - '0');
				l--;
			}
			p++;
			l--;
		}
	}
	name[0] = j;
}

static Cond HasName(Str255 name)
{
	unsigned char *p;
	p = (unsigned char *)name;

	if (*p++ != 8)
		return true;
	if (*p++ != '-')
		return true;
	if (((*p++) & 223) != 'N')
		return true;
	if (((*p++) & 223) != 'O')
		return true;
	if (((*p++) & 223) != 'N')
		return true;
	if (((*p++) & 223) != 'A')
		return true;
	if (((*p++) & 223) != 'M')
		return true;
	if (((*p++) & 223) != 'E')
		return true;
	if (*p != '-')
		return true;
	return false;
}

static void DrawMdvLeds(void)
{
	/* draw microdrive leds:
     all off if mdvOn==0
     otherwise the#mdvOn led is on, the others are off
     */
}

void StopMotor(void)
{
	mdvOn = 0;
	DrawMdvLeds();
}

static void InitDirDevDriver(Str255 name, w32 addr, w32 *fsid)
{
	w32 savedRegs[4];
	w32 *p;

	/*printf("Init driver %s\n",name);*/

	BlockMoveData(aReg, savedRegs, 4 * sizeof(w32));
	reg[1] = 38 + strlen(name);
	if ((strlen(name) & 1) != 0)
		reg[1]++;
	reg[2] = 0;
	QLtrap(1, 0x18,
	       20000l); /* allocate memory for the driver linkage block */
	if ((*reg) == 0) {
#if 1
		*fsid = aReg[0];
		p = (w32 *)(aReg[0] + (Ptr)theROM + 4);
		WL(p++, addr);
		WL(p++, addr + 2);
		WL(p++, addr + 4);
		WL(p, addr + 6);
		p += 3;
		WL(p++, addr + 8);
		WL(p++, 36);
		WL(p, 0);
#endif
#if 0
      *fsid=aReg[0];
      p=(w32*)(aReg[0]+(Ptr)theROM+4);
      *p++=addr;
      *p++=addr+2;
      *p++=addr+4;
      *p=addr+6;
      p+=3;
      *p++=addr+8;
      *p++=36;
      *p=0;
#endif
		BlockMoveData(name, (Ptr)p + 2, strlen(name) + 1);
		WW((Ptr)p, strlen(name)); /* *(uw16*)p=strlen(name);*/

		QLtrap(1, 0x22,
		       20000l); /* link directory device driver in IOSS */
	}
	BlockMoveData(savedRegs, aReg, 4 * sizeof(w32));
}
#if 0
void InitFileDrivers(Str255 floppyName,Str255 hDiskName)
{
  short i;
  mdvHeaders[0]=mdvHeaders[14]=255;
  mdvHeaders[1]=mdvHeaders[15]=0;
  if(floppyName!=nil)
    {
      i=strlen(floppyName)-1;
      /*while(i>-1) floppyName[i--]&=223; */
      InitDirDevDriver(floppyName,0x14000,&floppy_ref);
    }
  if(hDiskName!=nil)
    {
      i=strlen(hDiskName)-1;
      /*while(i>-1) hDiskName[i--]&=223;*/
      InitDirDevDriver(hDiskName,0x14000,&win_ref);
    }
  InitDirDevDriver("MDV",0x14000,&mdv_ref);

  InstallSerial();
}
#endif

void *Cleandir(char *nam)
{
	DIR *d;
	struct stat s;
	int n;

	d = opendir(nam);
	if (d) {
		struct dirent *p;
		while (p = readdir(d)) {
			char *pd, d_name[PATH_MAX];

			if (*p->d_name == '.' &&
			    (*(p->d_name + 1) == 0 || *(p->d_name + 1) == '.'))
				continue;

			pd = (char *)stpcpy(d_name, nam);
			if (*(pd - 1) != '/') {
				*pd++ = '/';
			}
			strcpy(pd, p->d_name);

			n = stat(d_name, &s);
			if (S_ISDIR(s.st_mode)) {
				Cleandir(d_name);
			} else {
				unlink(d_name);
			}
		}
		closedir(d);
		rmdir(nam);
	}
	return d;
}

void InitRAMDev(char *dev)
{
	short i, j;

	for (i = 0; i < MAXDEV; i++) {
		if (qdevs[i].qname && strcmp(qdevs[i].qname, dev) == 0) {
			for (j = 0; j < 8; j++) {
				char *ptr;
				if ((ptr = qdevs[i].mountPoints[j]) != NULL) {
					ptr += strlen(ptr);
					if (*(ptr - 1) == '/') {
						*--ptr = '\0';
					}
					/*printf("Making dir %s\n",qdevs[i].mountPoints[j]);*/

					mkdir(qdevs[i].mountPoints[j], 0755);
					*ptr = '/';
					/*printf("Making dir %s\n",qdevs[i].mountPoints[j]);*/
				}
			}
			break;
		}
	}
}

void CleanRAMDev(char *dev)
{
	short i, j;
	for (i = 0; i < MAXDEV; i++) {
		if (qdevs[i].qname && strcmp(qdevs[i].qname, dev) == 0) {
			for (j = 0; j < 8; j++) {
				char *ptr;
				if ((ptr = qdevs[i].mountPoints[j]) != NULL &&
				    qdevs[i].clean[j]) {
					ptr += (strlen(ptr) - 1);
					if (*ptr == '/') {
						*ptr = '\0';
					}
					/*printf("Removing dir %s\n",qdevs[i].mountPoints[j]);*/
					Cleandir(qdevs[i].mountPoints[j]);
				}
			}
			break;
		}
	}
}

void InitFileDrivers()
{
	short i, p, j;

	mdvHeaders[0] = mdvHeaders[14] = 255;
	mdvHeaders[1] = mdvHeaders[15] = 0;

	for (i = MAXDEV - 1; i >= 0; i--) {
		if (qdevs[i].qname) {
			/*printf("Initialising %s\n", qdevs[i].qname);*/

			for (j = 0; j < 8; j++)
				if (qdevs[i].Present[j])
					break;
			if (j < 8)
				InitDirDevDriver(qdevs[i].qname, 0x14000,
						 (w32 *)&qdevs[i].ref);
		}
	}

	InstallSerial();
	InitRAMDev("RAM");
}

w32 PhysicalDefBlock(void)
{
	w8 driveID; /* drive number */
	w32 pdb; /* physical definition block pointer */

	driveID = ReadByte(*aReg + 0x1d);
	pdb = ReadLong(0x28100 + ((w32)driveID << 2));
	return pdb;
}

static void VolBytes(short vol, long *total, long *empty)
{ /* returns #total and #empty sectors on the selected file device (deleted) */
	printf("calling VolBytes\n");
}

static struct mdvFile *MacFile(Cond check)
{
	struct mdvFile *f;
	if (*aReg < 131072 || *aReg > RTOP - 130) {
		CustomErrorAlert("bad channel block address");
		return nil;
	}
	f = (struct mdvFile *)((Ptr)theROM + ((*aReg) & ADDR_MASK_E) + 0x1e);
	if (!check)
		return f;
	if (GET_ID(f) == MDV_ID)
		return f;
	return nil;
}

int file_reopen(struct mdvFile *f, int frk)
{
	int a, filesys, drive, i, key, res;
	uw32 pdb;
	struct mdvFile *next;

	a = RL((char *)f - 0x1e + 4);

	filesys = -1;

	for (i = 0; i < MAXDEV; i++) {
		if (qdevs[i].qname) {
			if (a == qdevs[i].ref) {
				filesys = i;
				break;
			}
		}
	}
	if (filesys == -1)
		return -1;

	drive = *((char *)f + 5 - (0x1e - 0x18));
	key = *((char *)f + 4 - (0x1e - 0x18));

	drive = ReadByte(pdb = ReadLong(0x28100 + drive * 4) + 20);

	if (frk)
		next = GET_NEXT(f);

	res = mdv_doopen(f, filesys, drive, key, pdb);

	if (frk)
		SET_NEXT(f, next);

	/*printf("file_reopen res=%d\n",res);*/
	return res;
}

void MdvIO(void)
{
	struct mdvFile *f;
	struct fileHeader h;
	short op;
	long eof, fpos;
	long total, empty;
	long count;
	int isdisk;
	short i;
	short n;
	w8 *p;
	w16 *w;
	w8 b;

	if ((Ptr)gPC - (Ptr)theROM - 2 != 0x14000l) {
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
		return;
	}
	f = MacFile(0);
	/*  printf("MacFile res: %x\n",f);*/

	if (f == nil) {
	xerr:
		*reg = -18; /* overflow */
		rts();
		return;
	}

	/* test if it is a shared file' duplicate */
	if ((GET_ID(f) != MDV_ID) || (!GET_OPEN(f))) {
		/* we MUST assume it is... */

		/*printf("need reopen file : %s\n",NAME_REF(f)+2);*/
		if (file_reopen(f, 0) < 0)
			goto xerr;
	}

	if (!GET_OPEN(f)) {
		*reg = -7; /* not found */
		rts();
		return;
	}
#if 0
  if ( (GET_ISDIR(f) || GET_KEY(f)==1) && (op==5 || op==7 || op==0x49 || op==0x46))
    {
      *reg=QERR_RO;
      rts();
      return;
    }
#endif
	op = *reg;
	*reg = 0;

	if (GET_ISDIR(f) && (op == 5 || op == 7 || op == 0x49 || op == 0x46))
		debug2("Writing to directory. op=", op);
	isdisk = GET_ISDISK(f);
	if (isdisk == 0 || isdisk == 2)
		(void)QHostIO(f, op, isdisk);
	else
		QDiskIO(f, op);

	rts();
}

//#define TRACE_OPEN

uw32 mdv_doopen(struct mdvFile *f, int filesys, int drive, int key, uw32 pdb)
{
	struct HF_FCB *fcb;
	w32 res;
	short onDisk, alloc_fcb, canExist, canCreate, perm, e;

	/*
	 * BEWARE : the logic of this function is so broken that gdb can't
   	 * debug it -- adding a few more bizare statements a'la "1==1;"
   	 */

	alloc_fcb = 0;

	SET_FILESYS(f, filesys);

	SET_OPEN(f, false);

	drive--;
	if (qdevs[filesys].Present[drive]) {
#ifdef TRACE_OPEN
		printf("MDVOpen : name %s namelen %d %s\n", NAME_REF(f) + 2,
		       RW(NAME_REF(f)), (key == 4 ? "as directory" : ""));
#endif
		SET_ISDISK(f, onDisk = qdevs[filesys].Where[drive]);
		SET_DRIVE(f, drive);

		/* TK2 opens the device as a file so kludge here */
		if (!strlen(NAME_REF(f) + 2) && onDisk != 0 && onDisk != 2 &&
		    key == 1)
			key = 4;

		fcb = (Ptr)malloc(sizeof(struct HF_FCB));
		alloc_fcb = 1;
		SET_FCB(f, fcb);

		SET_KEY(f, key);
		res = 0;
		if (key != Q_DIR && !onDisk)
			FilenameFromQL(NAME_REF(f));
		if (key >= Q_DIR || key < 0) {
			if (key == Q_DIR) /* open directory */
			{
				SET_ISDIR(f, true);
				if (onDisk == 0 || onDisk == 2)
					res = QHOpenDir(f, onDisk);
				else
					res = QDiskOpenDir(f);
				/* printf("OpenDir retval %d\n",res); */
			} else if (key < 0) /* delete file */
			{
				if (onDisk == 0 || onDisk == 2)
					res = HDelete(mdvVol[drive],
						      mdvDir[drive],
						      NAME_REF(f), f, 2);
				else
					res = QDiskDelete(f);
			} else
				res = -15; /* bad parameter */
		} else /* open file */
		{
			canExist = key != Q_IO_NEW;
			canCreate = key >= Q_IO_NEW;
			/*printf("open %s key=%d, cancreate: %d\n",NAME_REF(f)+2,key,canCreate);*/
			if (onDisk == 0 || onDisk == 2) {
				perm = (key == 1 ? O_RDONLY : O_RDWR) |
				       (canExist && canCreate ? O_CREAT : 0);
				e = HOpenDF(mdvVol[drive], mdvDir[drive],
					    NAME_REF(f), perm, f, 0, onDisk);

				if (e && (key != 2)) {
					perm = O_RDONLY |
					       (canExist && canCreate ?
							      O_CREAT :
							      0);
					e = HOpenDF(mdvVol[drive],
						    mdvDir[drive], NAME_REF(f),
						    perm, f, 0, onDisk);
				}

				if (e == 0 && !canExist) {
					res = -8; /* already exists */
					FSClose(GET_HFILE(f));
				} else if (e == 0) /* file succesfully opened */
					SET_ISDIR(f, false);
				else {
					if (e == -49)
						res = -9; /* in use */
					else {
						res = QERR_NF;
						if (canCreate) {
							/*printf("file name %s\n",NAME_REF(f)+2);*/
							perm = (key == 1 ?
									      O_RDONLY :
									      O_RDWR) |
							       (canExist && canCreate ?
									      O_CREAT :
									      0);
							e = HOpenDF(
								mdvVol[drive],
								mdvDir[drive],
								NAME_REF(f),
								perm | O_CREAT,
								f, canCreate,
								onDisk);
							if (e && (key != 2)) {
								perm = O_RDONLY |
								       (canExist && canCreate ?
										      O_CREAT :
										      0);
								e = HOpenDF(
									mdvVol[drive],
									mdvDir[drive],
									NAME_REF(
										f),
									perm, f,
									0,
									onDisk);
							}
							res = 0;
							if (e != 0)
								res = QERR_NF;
						}
					}
				}
			} else
				res = QDiskOpen(f, drive, canExist, canCreate);
		}
		StopMotor();
	} else
		res = QERR_NF;
	drive++;

noBlock:
	if (res == 0 && GET_KEY(f) >= 0) {
		drive--;
		if (GET_KEY(f) == 3)
			if (qdevs[filesys].Where[drive] == 1) {
				struct fileHeader *h;

				h = GetFileHeader(GET_FNUMBER(f));
				if (h) {
					SET_FLEN(h, 64);
					RewriteHeader();
					KillFileTail(GET_FNUMBER(f), 0);
				}
			} else
				ftruncate(GET_HFILE(f), 0);

		SET_POS(f, 64); /*f->pos=64;*/
		if (onDisk == 0 || onDisk == 2)
			SET_EOF(f, HDfLen(f, onDisk) + 64);
		else
			SET_EOF(f, QDiskLen(f) + 64);

		/*printf("File pos %d, File len %d\n",GET_POS(f),GET_EOF(f));*/
		qdevs[filesys].OpenFiles[drive]++;
		SET_NEXT(f, qdevs[filesys].FileList[drive]);
		qdevs[filesys].FileList[drive] = f;
		if (onDisk == 0 ||
		    onDisk == 2) { /* linking for QDisk not necessary */
			fcb->next = qdevs[filesys].fcbList[drive];
			qdevs[filesys].fcbList[drive] = fcb;
		}
		SET_OPEN(f, true);
		SET_ID(f, MDV_ID);
	} else if (alloc_fcb)
		free(fcb);

#ifdef TRACE_OPEN
	printf("mdv_doopen returns %d\n", res);
#endif

	return res;
}

void MdvOpen(void)
{
	w8 drive;
	struct mdvFile *f = nil;
	struct HF_FCB *fcb;
	Cond canCreate;
	Cond canExist;
	Cond onDisk;
	w32 filesys;
	int perm, alloc_fcb = 0;
	OSErr e;
	w32 a;
	short i;
	w8 *p;

	int key;

	if ((Ptr)gPC - (Ptr)theROM - 2 != 0x14002l) {
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
		return;
	}
	drive = ReadByte(aReg[1] + 0x14);
	if (drive > 0 && drive < 9) {
		f = MacFile(false);
		/*    printf("macfile res: %x\n",f);*/
		if (f == nil) {
			*reg = -18; /* overflow */
			goto noBlock;
		}
		a = aReg[3] + 0x18;

		filesys = -1;

		for (i = 0; i < MAXDEV; i++) {
			if (qdevs[i].qname) {
				if (a == qdevs[i].ref) {
					filesys = i;
					break;
				}
			}
		}

		if (filesys == -1) {
			printf("wrong filesystem\n");
			*reg = -7;
			goto noBlock;
		}
		key = ReadByte(*aReg + 0x1c);

		reg[0] = mdv_doopen(f, filesys, drive, key, aReg[1]);
	}
noBlock:
	rts();
}

/* create new filedescriptors for all files inherited across fork*/
void fork_files()
{
	int fs, dr;
	int nfd;
	struct mdvFile *f;

#if 0
  OpenDisk(0);  /* that was it for diskimage access :-) */
#endif

	for (fs = 0; fs < MAXDEV; fs++)
		for (dr = 0; dr < 8; dr++)
			if (qdevs[fs].Present[dr]) {
				if ((qdevs[fs].Where[dr] == 0 ||
				     qdevs[fs].Where[dr] == 2) &&
				    (f = (qdevs[fs].FileList[dr])))
					while (f != nil && ((int)f & 1) == 0 &&
					       GET_ID(f) == MDV_ID) {
						nfd = GET_HFILE(f);
						close(nfd);
						file_reopen(f, 1);
						/*reopen_uxfile(f);*/
						f = GET_NEXT(f);
					}
			}
}

void CloseAllMdvFiles(short drive, int filesys)
{
	struct mdvFile *f;
	int isdisk;

	f = qdevs[filesys].FileList[drive];
	while (f != nil && ((int)f & 1) == 0 && GET_ID(f) == MDV_ID) {
		if (GET_OPEN(f)) {
			isdisk = GET_ISDISK(f);
			if (isdisk == 0 || isdisk == 2) {
				FSClose(GET_HFILE(f));
				free(GET_FCB(f));
#if 0
	      if(GET_ISDIR(f)) HDelete(mdvVol[drive],mdvDir[drive],NAME_REF(f));
#endif
			} else {
				QDiskClose(f);
				free(GET_FCB(f));
			}
		}
		SET_OPEN(f, false);
		SET_ID(f, 0);
		f = GET_NEXT(f);
	}
	qdevs[filesys].FileList[drive] = nil;
	qdevs[filesys].fcbList[drive] = nil;
	qdevs[filesys].OpenFiles[drive] = 0;
}

void CloseAllFiles(void)
{
#if 0
  CloseAllMdvFiles(0);
  CloseAllMdvFiles(1);
  mdvOn=0;
  DrawMdvLeds();
#endif
}

Cond FileAlreadyOpen(short drive, int filesys, FileNum fileNum,
		     w16 *perm) /* works for ql disks only */
{
	struct mdvFile *f;

	f = qdevs[filesys].FileList[drive];
	while (f != nil) {
		if (GET_OPEN(f) && (GET_ISDISK(f) == 1) &&
		    GET_FNUMBER(f).file == fileNum.file) {
			*perm = GET_KEY(f);
			return true;
		}
		f = GET_NEXT(f);
	}
	return false;
}

static void UnlinkOpenFile(short drive, struct mdvFile *f)
{
	struct mdvFile *p;
	struct HF_FCB *fcb, *fb;
	int isdisk;

	int filesys = GET_FILESYS(f);
	fb = GET_FCB(f);

	p = qdevs[filesys].FileList[drive];
	if (p == f)
		qdevs[filesys].FileList[drive] = GET_NEXT(f);
	else {
		while (p != nil && GET_NEXT(p) != f)
			p = GET_NEXT(p);
		if (GET_NEXT(p) == f)
			SET_NEXT(p, GET_NEXT(f));
	}
	isdisk = GET_ISDISK(f);
	if (isdisk == 0 || isdisk == 2) {
		fcb = qdevs[filesys].fcbList[drive];
		if (fcb == fb)
			qdevs[filesys].fcbList[drive] = fcb->next;
		else {
			while (fcb && fcb->next != fb)
				fcb = fcb->next;
			if (fb == fcb->next)
				fcb->next = fb->next;
		}
	}
}

void MdvClose(void)
{
	w32 pdb; /* physical definition block pointer */
	w8 d;
	struct mdvFile *f;
	int filesys;

	if ((Ptr)gPC - (Ptr)theROM - 2 != 0x14004l) {
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
		return;
	}
	f = MacFile(true);
	if (f == nil) {
		*reg = -18; /* overflow */
		rts();
		return;
	}
	if ((GET_ID(f) != MDV_ID) || (!GET_OPEN(f))) {
		*reg = -18; /* overflow */
		rts();
		return;
	}
	if (GET_OPEN(f)) {
		if (GET_ISDISK(f) == 1)
			QDiskClose(f);
		else
			FSClose(GET_HFILE(f));
	}

	filesys = GET_FILESYS(f);
	qdevs[filesys].OpenFiles[GET_DRIVE(f)]--;
	UnlinkOpenFile(GET_DRIVE(f), f);
	SET_OPEN(f, false);
	SET_ID(f, 0);
#if 0
  if(GET_ISDIR(f) && (isdisk == 0 || isdisk == 2))  /* delete directory file */
    {
      HDelete(mdvVol[GET_DRIVE(f)],mdvDir[GET_DRIVE(f)],NAME_REF(f),isdisk);
    }
#endif

	/* parte finale della routine : QL Technical Guide pag. 43 */
	pdb = PhysicalDefBlock();
	d = ReadByte(pdb + 0x22);
	WriteByte(pdb + 0x22, d - 1); /* decrement number of open files */
	(*aReg) += 0x18;
	aReg[1] = 0x28140;
	QLvector(0xd4, 20000l); /* unlink file from list */
	(*aReg) -= 0x18;
	QLvector(0xc2, 20000l); /* release channel block memory */
	rts();
}

void MdvSlaving(void)
{
	if ((Ptr)gPC - (Ptr)theROM - 2 != 0x14006l) {
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
		return;
	}
	rts();
}

void MdvFormat(void)
{
	w8 drive;
	short nameLen;
	long total, empty;
	int filesys;

	if ((Ptr)gPC - (Ptr)theROM - 2 != 0x14008l) {
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
		return;
	}

#if 1
	*reg = QERR_NI;
#else
	if (mdvOn)
		*reg = -9; /* in use */
	else {
		drive = (w8)reg[1]; /* drive number */
		if (drive < 1 || drive > 2)
			*reg = -7; /* not found */
		else {
			filesys = 1;
			drive--;
			if (qdevs[filesys].Present[drive] &&
			    qdevs[filesys].OpenFiles[drive] == 0) {
				w32 a6;

				if (!qdevs[filesys].Where[drive])
					NoteAlert(rFormat, nil);
				/* set medium name */
				nameLen = ReadWord(aReg[1]) - 5;
				if (nameLen > 255)
					nameLen = 255;
				else if (nameLen < 0)
					nameLen = 0;
				flpName[drive][0] = nameLen;
				if (nameLen > 0)
					BlockMoveData((Ptr)theROM + aReg[1] + 7,
						      &(flpName[drive][1]),
						      nameLen);
				if (nameLen > 10)
					nameLen = 10;
				if (nameLen > 0)
					BlockMoveData((Ptr)theROM + aReg[1] + 7,
						      mdvHeaders + 14 * drive +
							      2,
						      nameLen);
				while (nameLen < 10)
					mdvHeaders[14 * drive + 2 + nameLen++] =
						' ';

				/* set header flag, sector and random number */
				mdvHeaders[drive * 14] = 0xff;
				mdvHeaders[drive * 14 + 1] = 0;
				a6 = 0x28000;
				mdvHeaders[drive * 14 + 12] =
					*((uw8 *)theROM + a6 + 0x2e);
				mdvHeaders[drive * 14 + 13] =
					*((uw8 *)theROM + a6 + 0x2f);
				/* set number of total/good sectors */
				*reg = 0;
				if (qdevs[filesys].Where[drive])
					QDiskFormat(mdvHeaders + 14 * drive + 2,
						    &total, &empty);
				else
					*reg = QERR_NI;

				if (*reg == 0) {
					reg[1] = empty;
					if (reg[1] > 30000)
						reg[1] = 30000;
					reg[2] = total;
					if (reg[2] > 30000)
						reg[2] = 30000;
				} else
					qdevs[filesys].Present[drive] = false;
				/*      InvalWindow(mdvWindow); */
			} else {
				if (qdevs[filesys].OpenFiles[drive] != 0)
					*reg = -9; /* in use */
				else
					*reg = -14; /* format failed */
			}
			drive++;
		}
	}
#endif
	rts();
}

void AttachDirectory(short drive, Cond askFile, short vRefNum, long parID)
{ /* attach directory to drive slot */
}
void REGP1 WriteMdvControl(aw8 d)
{
	switch (d) {
	case 3:
		mdvOn = 1;
		DrawMdvLeds();
		break;
	case 2:
		if (mdvOn != 0) {
			mdvOn++;
			if (mdvOn > 8)
				mdvOn = 0;
			DrawMdvLeds();
		}
		break;
	}
}

static void AddReturn(short d)
{
	WriteLong(*m68k_sp, ReadLong(*m68k_sp) + d);
}

void ReadMdvSector(void)
{
	if ((uw16 *)((Ptr)theROM + 0x4002 +
		     RW((uw16 *)((Ptr)theROM + 0x124))) == gPC) {
		if (mdvOn == 1 || mdvOn == 2) {
			AddReturn(2);
			reg[1] = reg[2] = 0;
			aReg[1] += 512;
		}
		rts();
	} else {
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void WriteMdvSector(void)
{
	if ((uw16 *)((Ptr)theROM + 0x4002 +
		     RW((uw16 *)((Ptr)theROM + 0x126))) == gPC) {
		if (mdvOn == 1 || mdvOn == 2)
			aReg[1] += 512;
		rts();
	} else {
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void VerifyMdvSector(void)
{
	if ((uw16 *)((Ptr)theROM + 0x4002 +
		     RW((uw16 *)((Ptr)theROM + 0x128))) == gPC) {
		if (mdvOn == 1 || mdvOn == 2) {
			AddReturn(2);
			reg[1] = reg[2] = 0;
			aReg[1] += 512;
		}
		rts();
	} else {
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}

void ReadMdvHeader(void)
{
	short i;
	w8 *p;
	if ((uw16 *)((Ptr)theROM + 0x4002 +
		     RW((uw16 *)((Ptr)theROM + 0x12a))) == gPC) {
		if (mdvOn == 1 || mdvOn == 2) {
			AddReturn(4);
			p = (w8 *)mdvHeaders;
			if (mdvOn == 2)
				p += 14;
			for (i = 0; i < 14; i++)
				WriteByte(aReg[1]++, *p++);
			reg[7] = mdvHeaders[mdvOn * 14 - 13];
		}
		rts();
	} else {
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
	}
}
