/*
 * (c) UQLX - see COPYRIGHT
 */

#ifndef QFPH
#define QFPH

#define rOpenFiles	133
#define SOFT_ERROR	-9845
#define DEMO_ERROR	-9846


/* it would be so easy to use this struct ..*/
#if 0
#pragma options align=mac68k
struct mdvFile {
	w16		fNumber;
	w32		pos;
	w32		eof;
	w32		slave;
	char	boh[6];
	unsigned char	name[38];
	struct	mdvFile *next;
	short	vol;
	long	dir;
	short	ref;
	short	key;
	short	drive;
	Cond	isDirectory;   /* dirs can't be written !! */
	Cond	open;
	Cond qlDisk;
	long	id;			/* to detect overwritten file block */
};
#else
struct mdvFile{ char dummy[130];};
#endif

/* unfortunately it gets packed differently on many systems so we need 
   assembler style offstes instead   */

#define PACKED  __attribute__ ((packed))

typedef struct {
    int dir PACKED; /* file number of (parent) direcory */
    int file PACKED;
    int entrynum PACKED;   /* number of entry in directory */

}FileNum;

struct fileHeader *GetFileHeader(FileNum fileNum);
  
#define _fNumber 0
#define _pos 2
#define _eof 6
#define _slave 10
#define _name 20
#define _next (20+38)
#define _vol (_next+4)
#define _dir (_vol+2)
#define _ref (_dir+4)             /* abuse this field to hold update status */
#define _key (_ref+4)
#define _drive (_key+2)
#define _isdir (_drive+2)
#define _open (_isdir+2)
#define _qlDisk (_open+2)
#define _id (_qlDisk+2)
#define _filesys (_id+4)
#define _hfileref (_filesys+4)
/*#define _nfn   (_hfileref+4)*/
#define _hf_fcb (_hfileref+4)     /* sizeof == 8 !!*/
#define mdvFile_len (_hf_fcb+8)

/* Macros needed to avoid alignment errors on certain architectures  */
#if 0
#define SET_FNUMBER(_mdvf_,_num_) (WW((Ptr)((Ptr)(_mdvf_)+_fNumber),(_num_)))
#define GET_FNUMBER(_mdvf_)  (RW((Ptr)((Ptr)(_mdvf_)+_fNumber)))
#else
#if 0
#define SET_FNUMBER(_mdvf_,_num_) (*(FileNum*)((Ptr)(_mdvf_)+_nfn)=(_num_))
#define GET_FNUMBER(_mdvf_)  (*(FileNum*)((Ptr)(_mdvf_)+_nfn))
#endif
#endif

#define SET_POS(_mdvf_,_pos_) (WL((Ptr)((Ptr)(_mdvf_)+_pos),(_pos_)))
#define GET_POS(_mdvf_)   (RL((Ptr)((Ptr)(_mdvf_)+_pos)))
#define SET_EOF(_mdvf_,_pos_)  (WL((Ptr)((Ptr)(_mdvf_)+_eof),(_pos_)))
#define GET_EOF(_mdvf_)  (RL((Ptr)((Ptr)(_mdvf_)+_eof)))
#define SET_REF(_mdvf_,_ref_) (WL((Ptr)((Ptr)(_mdvf_)+_ref),(_ref_)))
#define GET_REF(_mdvf_) (RL((Ptr)((Ptr)(_mdvf_)+_ref)))
#define SET_NEXT(_mdvf_,_nxt_) (WL((Ptr)((Ptr)(_mdvf_)+_next),(w32)(_nxt_)))
#define GET_NEXT(_mdvf_)  ((struct mdvFile *)RL((Ptr)((Ptr)(_mdvf_)+_next)))
#define SET_ID(_mdvf_,_id_) (WL((Ptr)((Ptr)(_mdvf_)+_id),(_id_)))
#define GET_ID(_mdvf_) (RL((Ptr)((Ptr)(_mdvf_)+_id)))
#define GET_OPEN(_mdvf_) (RW((Ptr)((Ptr)(_mdvf_)+_open)))
#define SET_OPEN(_mdvf_,_val_) (WW((Ptr)((Ptr)(_mdvf_)+_open),(_val_)))
#define GET_DRIVE(_mdvf_) (RW((Ptr)((Ptr)(_mdvf_)+_drive)))
#define GET_ISDIR(_mdvf_) (RW((Ptr)((Ptr)(_mdvf_)+_isdir)))
#define GET_ISDISK(_mdvf_) (RW((Ptr)((Ptr)(_mdvf_)+_qlDisk)))
#define NAME_REF(_mdvf_) ((char *)(Ptr)(_mdvf_)+_name)
#define GET_KEY(_mdvf_) ((w16)RW((Ptr)((Ptr)(_mdvf_)+_key)))
#define SET_DRIVE(_mdvf_,_drv_) (WW((Ptr)((Ptr)(_mdvf_)+_drive),(_drv_)))
#define SET_ISDIR(_mdvf_,_val_) (WW((Ptr)((Ptr)(_mdvf_)+_isdir),(_val_)))
#define SET_ISDISK(_mdvf_,_val_) (WW((Ptr)((Ptr)(_mdvf_)+_qlDisk),(_val_)))
#define SET_KEY(_mdvf_,_val_) (WW((Ptr)((Ptr)(_mdvf_)+_key),(_val_)))
#define GET_FILESYS(_mdvf_)  ((w32)(RL((Ptr)((Ptr)(_mdvf_)+_filesys))))
#define SET_FILESYS(_mdvf_,_val_)  (WL((Ptr)((Ptr)(_mdvf_)+_filesys),(_val_)))
#define GET_HFILE(_mdvf_)  ((RL((Ptr)((Ptr)(_mdvf_)+_hfileref))))
#define SET_HFILE(_mdvf_,_val_)  (WL((Ptr)((Ptr)(_mdvf_)+_hfileref),(_val_)))

#define GET_FCB(_mdvf_)  ((struct HF_FCB *)(GET_POINTER((Ptr)(_mdvf_) + _hf_fcb)))
/*(RL((Ptr)((Ptr)(_mdvf_)+_hf_fcb))))*/
#define SET_FCB(_mdvf_,_val_)  SET_POINTER((Ptr)(_mdvf_)+_hf_fcb,(_val_))
/*(WL((Ptr)((Ptr)(_mdvf_)+_hf_fcb),(_val_)))*/

#define SET_FNUMBER(_mxdvf_,_num_)  (GET_FCB(_mxdvf_)->fileNum=_num_) 
#define GET_FNUMBER(_mxdvf_)    (GET_FCB(_mxdvf_)->fileNum) 


/* the addr of this struct is accessed by GET_FCB(f) */
/* most of the attribs now stored in mdvFile should go into it */
struct HF_FCB
{
  struct HF_FCB *next;
  int mode;
  FileNum fileNum; /* DiskOp only */
  /* %--------------% */
  char uxname[256]; /* must be last and unpadded in this struct for QVFS */
};


#if 0
struct fileHeader{
	w32	len;
	w8	access;
	w8	type;
	w32	info1;
	w32	info2;
	unsigned char name[38];
	w32	date_update;
	w32	date_ref;
	w32	date_backup;
};
#else
struct fileHeader { char xx[64];};
#endif

#define _flen 0
#define _facc 4
#define _ftyp 5
#define _finf1 6
#define _finf2 10
#define _fname 14
#define _fdupdt (0x34)
/*#define _fdref (_0x3c)*/
#define _fdtbak (0x3c)
#define _fdvers  0x38

#define SET_FLEN(_fh_,_len_) (WL((Ptr)_fh_,_len_))
#define GET_FLEN(_fh_)   (RL((Ptr)_fh_))
#define SET_FACC(_fh_,_len_) (WB((Ptr)_fh_+_facc,_len_))
#define GET_FACC(_fh_)   (RB((Ptr)_fh_+_facc))
#define SET_FTYP(_fh_,_len_) (WB((Ptr)_fh_+_ftyp,_len_))
#define GET_FTYP(_fh_)   (RB((Ptr)_fh_+_ftyp))
#define REF_FNAME(_fh_) ((char *)((char *)(_fh_)+_fname))
#define SET_FDUPDT(_fh_,_tme_) (WL((Ptr)_fh_+_fdupdt,_tme_))
#define SET_FDTBAC(_fh_,_tme_) (WL((Ptr)_fh_+_fdtbak,_tme_))

#define SET_FVER(_fh_,_tme_) (WW((Ptr)_fh_+_fdvers,_tme_))
#define GET_FVER(_fh_) (RW((Ptr)_fh_+_fdvers))

#define GET_FDUPDT(_fh_) (RL((Ptr)_fh_+_fdupdt))
#define GET_FDTBAC(_fh_) (RL((Ptr)_fh_+_fdtbak))

typedef char Str255[256];


extern	unsigned char	mdvHeaders[];
extern	short	mdvOn;

void	CloseAllMdvFiles(short,int);
void	StopMotor(void);
Cond FileAlreadyOpen(short,int,FileNum,w16 *);
void FilenameFromQL(unsigned char *);
void GetWDparms(short,short *,long *);
OSErr GetDirName(Str255,short,long);

OSErr	QDiskDelete(struct mdvFile*);
OSErr	QDiskOpen(struct mdvFile*,int ,Cond, Cond);
void	QDiskClose(struct mdvFile*);
w32		QDiskLen(struct mdvFile*);
OSErr	QDiskIO(struct mdvFile*,short);
void	QDiskFormat(uw8*,long*,long*);

void	AdjustSetDirMenu(void);
void	InitFileDrivers(void);
w32		PhysicalDefBlock(void);


void QHDeleteHeader(char *,int , char *, struct mdvFile *, int);
int HDelete(int , int , unsigned char *, struct mdvFile *, int);
int HOpenDF(int , long ,unsigned char *, long, struct mdvFile *, int, int);
int QHOpenDir(struct mdvFile *, int);
int HCreate(struct mdvFile *, unsigned char *, unsigned char *, unsigned char *, int);
int HDfLen(struct mdvFile *, int);
int QHostIO(struct mdvFile *, int, int );

#endif
