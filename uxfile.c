/*
 * (c) UQLX - see COPYRIGHT
 */

#include "QL68000.h"

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <utime.h>
#include <ctype.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#include <string.h>
#include <stdio.h>

#include "unix.h"
#include "dummies.h"
#include "QL.h"
#include "QLfiles.h"
#include "QFilesPriv.h"
#include "QSerial.h"

#include "QInstAddr.h"
#include "QDisk.h"
#include "emudisk.h"
#include "QDOS.h"
#include "unixstuff.h"
#include "QL_driver.h"

#include "memaccess.h"
#include "SDL2screen.h"

#include "q-emulator.h"
#include "qdos-file-header.h"

#ifdef __WIN32__
#include <sqlux_windows.h>
#endif

/* DIR_SEPARATOR specifies how directories will be listed, I prefer the unixish variant */
/* but QPAC2 doesn't */
#define DIR_SEPARATOR '_'

#define ERR_BUFFER_FULL -5
#define ERR_FULL -11
#define ERR_EOF -10

#define min(_a_, _b_) (_a_ < _b_ ? _a_ : _b_)
#define max(_a_, _b_) (_a_ > _b_ ? _a_ : _b_)

int match(char *, char *, char *, int, int, int, int);

static void addpath(char *to, char *name, int maxnlen);

int eretry(void)
{
	return errno == EINTR || errno == EAGAIN;
}

void qaddpath(char *mount, char *name, int maxnlen)
{
	int i;

	if (*mount == 0)
		return (void)strncpy(mount, name, maxnlen);
	i = strlen(mount);
	if ((mount[i - 1]) != '/')
		return (void)addpath(mount, name, maxnlen);
	strncat(mount, name, maxnlen - strlen(mount));
}
static int qunlinkfile(char *mount, char *name, int maxnlen)
{
	int res, q = strlen(mount);
	qaddpath(mount, name, maxnlen);

	res = unlink(mount);

	mount[q] = 0;
	return res;
}

int qopenfile(char *mount, char *name, int flags, int mode, int maxnlen)
{
	int res, q = strlen(mount);
	qaddpath(mount, name, maxnlen);

	res = open(mount, flags, mode);

	mount[q] = 0;
	return res;
}

DIR *qopendir(char *mount, char *name, int maxnlen)
{
	DIR *res;
	int q = strlen(mount);
	qaddpath(mount, name, maxnlen);

	if (*mount)
		res = opendir(mount);
	else
		res = opendir(".");

	mount[q] = 0;
	return res;
}

long qlseek(struct mdvFile *f, off_t pos, int how)
{
	int fd = GET_HFILE(f);
	int r, cpos, end, seekbase;

	seekbase = GET_SEEKBASE(f);

	cpos = lseek(fd, 0, SEEK_CUR);
	end = lseek(fd, 0, SEEK_END);
	lseek(fd, cpos, SEEK_SET);
	if (how == SEEK_SET) {
		r = lseek(fd, pos + seekbase, how);
	} else {
		r = lseek(fd, pos, how);
	}
	if (r > end) {
		*reg = QERR_EF;
		r = lseek(fd, end, SEEK_SET);
	}
	return r;
}

int uxLookupDir(char *mount, char *qdname, struct mdvFile *f, char *uxname,
		int fstype)
{
	int qlen;
	char *p1, *p2, temp[256];

	qlen = RW((Ptr)qdname);

	/*printf("uxLookupDir %s\n",qdname+2);*/
	strncpy(temp, qdname + 2, 36);
	uxname[0] = 0;

	if (qlen && temp[qlen - 1] == '_')
		temp[qlen - 1] = 0;

	return match(mount, uxname, temp, 1, 0, 320, fstype);
}

int uxLookupFile(char *mount, char *qdname, struct mdvFile *f, char *uxname,
		 int create, int fstype)
{
	int qlen;
	char *p1, *p2, temp[256];

	qlen = RW((Ptr)qdname);
	/*printf("uxLookupFile %s\n",qdname+2);*/
	strncpy(temp, qdname + 2, 36);
	uxname[0] = 0;

	return match(mount, uxname, temp, 0, create, 320, fstype);
}

int path_match_char(char p, char u)
{
	if (tolower(p) == (tolower(u)))
		return 1;

	if ((p == '_') && (u == '.'))
		return 1;

	return 0;
}
char *nseg(char *qpath, char *uxname, char separator, int fstype)
{
	char *p = qpath;
	char *u = uxname;
	int l = 0;
	int lp, lu;
	int maxlen;

	lp = strlen(p);
	lu = strlen(u);
	maxlen = min(lp, lu);

	if (fstype == 2)
		while (path_match_char(*p++, *u++) && l++ < maxlen)
			;
	else
		while (*p++ == *u++ && l++ < maxlen)
			;

	if ((*(p - 1) == separator) && (*(u - 1) == 0) && ((p - 1) > qpath)) {
		//if (fstype==2)printf("nseg: %s matches %s, rest %s\n",qpath,uxname,p);
		return p;
	} else
		return NULL;
}

void addpath(char *to, char *name, int maxnlen)
{
	/*printf("addpath: %s %s \n",to,name);*/
	if (*to)
		strcat(to, "/");
	strncat(to, name, maxnlen - strlen(to));
	/*printf("addpath:           -> %s\n",to);*/
}

#define memoize(xx, yy) (1) /* add later !!*/
#define memoized(xx, yy) (0)
#define delmemoize()
int match(char *mount, char *where, char *name, int isdir, int createp,
	  int maxnlen, int fstype)
{
	char *p, *wt;
	DIR *dirp;
	struct dirent *dp;
	int fd, res;

	/*printf("match %s in dir %s\n",name,where);*/

	wt = where + strlen(where);

	if ((res = memoized(where, name)))
		return res;
	addpath(where, name, maxnlen);
	if (isdir && (dirp = qopendir(mount, where, maxnlen))) {
		closedir(dirp);
		return memoize(where, name);
	}
	if (!isdir &&
	    (fd = qopenfile(mount, where, O_RDONLY | O_BINARY, 0, maxnlen)) > -1) {
		close(fd);
		return memoize(where, name);
	} else {
		*wt = 0;
		delmemoize();
	}
	/**/
	*wt = 0;
	if (!(dirp = qopendir(mount, where, maxnlen)))
		return 0;
	/**/

	if ((p = max(strchr(name, '/'), strchr(name, '_'))) == NULL &&
	    createp) {
		addpath(where, name, maxnlen);
		return memoize(where, name);
	}

#if 0
  if (!(dirp=qopendir(mount,where,maxnlen)))
    {
    if (createp)
      {
	addpath(where,name,maxnlen);
	return memoize(where,name);
      }
    else
      return 0;
    }
#endif

	if (p == NULL && isdir) {
		closedir(dirp);
		return 1;
	}

	while ((dp = readdir(dirp))) {
		if (!strcmp(dp->d_name, ".-UQLX-"))
			continue;
		if (fstype == 2 && (p = nseg(name, dp->d_name, 0x0, fstype))) {
			// exact match didn�t work so try ignore case match..
			//printf("fst2 found %s, %s, %s\n",p,name,dp->d_name);
			addpath(where, dp->d_name, maxnlen);
			// must test existence, type and permission to create!
			if (!isdir || match(mount, where, p, isdir, createp,
					    maxnlen, fstype)) {
				closedir(dirp);
				return memoize(where, dp->d_name);
			} else
				*wt = 0;
		}
		if ((p = nseg(name, dp->d_name, '/', fstype))) {
			addpath(where, dp->d_name, maxnlen);
			if ((res = match(mount, where, p, isdir, createp,
					maxnlen, fstype))) {
				closedir(dirp);
				return memoize(where, dp->d_name);
			} else
				*wt = 0;
		}
		if ((p = nseg(name, dp->d_name, '_', fstype))) {
			addpath(where, dp->d_name, maxnlen);
			if ((res = match(mount, where, p, isdir, createp,
					maxnlen, fstype))) {
				closedir(dirp);
				return memoize(where, dp->d_name);
			} else
				*wt = 0;
		}
#if 0
      /* this can be added when memoizing works */
      if (p=cinseg(name,dp->d_name,'/'))
	{
	  addpath(where,dp->d_name,maxnlen);
	  if (res=match(mount,where,p,isdir,createp,maxnlen)){
	    closedir(dirp);
	    return memoize(where,dp->d_name);
	  }

	  else *wt=0;
	}
      if (p=cinseg(name,dp->d_name,'_'))
	{
	  addpath(where,dp->d_name,maxnlen);
	  if (res=match(mount,where,p,isdir,createp,maxnlen)){
	    closedir(dirp);
	    return memoize(where,dp->d_name);
	  }
	  else *wt=0;

	}
#endif
	}
	closedir(dirp);
	*wt = 0;

	if (isdir)
		return 1;

	if (createp) {
		addpath(where, name, maxnlen);
		return memoize(where, name);
	}
	return 0;
}

int HCreate(struct mdvFile *f, unsigned char *name, unsigned char *s1,
	    unsigned char *s2, int fstype)
{
	int fd, err;
	char mname[64], mount[400];

	strncpy(mount, qdevs[GET_FILESYS(f)].mountPoints[GET_DRIVE(f)], 320);

	/*printf("calling HCreate\n");*/
	err = uxLookupFile(mount, (char *)name, f, mname, 0, fstype);
	if (!err)
		return -7;

	fd = creat(mname, 0666);
	if (fd >= 0) {
		close(fd);
		return 0;
	} else
		return -7;
}

void getpath(char *respath, char *name, int maxnlen)
{
	char *p;

	strncpy(respath, name, maxnlen);
	p = strrchr(respath, '/');
	if (p)
		*p = 0;
	else
		*respath = 0;
}

void getname(char *resname, char *name, int maxnlen)
{
	char *p = strrchr(name, '/');
	if (!p)
		p = name - 1;
	strncpy(resname, p + 1, maxnlen);
}

/* doesn't need fstype, uxname always exact */
void reopen_uxfile(struct mdvFile *f)
{
	size_t cpos;
	char mount[400];
	int fd = GET_HFILE(f);

	cpos = lseek(fd, 0, SEEK_CUR);
	close(fd);
	printf("reopening file %s, pos %zu\n", GET_FCB(f)->uxname, cpos);

	strncpy(mount, qdevs[GET_FILESYS(f)].mountPoints[GET_DRIVE(f)], 320);
	fd = qopenfile(mount, GET_FCB(f)->uxname, O_RDWR | O_BINARY, 0666, 400);
	if (fd < 0)
		fd = qopenfile(mount, GET_FCB(f)->uxname, O_RDONLY | O_BINARY, 0666, 400);
	if (fd < 0)
		perror("sorry, could not reopen file:");

	SET_HFILE(f, fd);
	lseek(fd, cpos, SEEK_SET);
}

int FillXH(int fd, char *name, struct fileHeader *h, int fstype)
{
	struct fileHeader buf;
	int nlen = strlen(name);
	short found = 0;
	int (*cmp_fun)(const char *, const char *, size_t);

	lseek(fd, 0, SEEK_CUR);

	cmp_fun = fstype ? strncasecmp : strncmp;
	while (64 == read(fd, &buf, 64)) {
		if (nlen == RW((Ptr)REF_FNAME(&buf)) &&
		    !cmp_fun(name, REF_FNAME(&buf) + 2, nlen) &&
		    RL((Ptr)&buf)) {
			WW(((Ptr)h) + 4, RW(((Ptr)&buf) + 4));
			WL(((Ptr)h) + 6, RL(((Ptr)&buf) + 6));
			WL(((Ptr)h) + 10, RL(((Ptr)&buf) + 10));
			WW(((Ptr)h) + _fdvers, RW(((Ptr)&buf) + _fdvers));
			found = 1;
			break;
		}
	}
	if (!found) {
		/*WW(((Ptr)h)+4, 0);*/
		WL(((Ptr)h) + 6, 0);
		WL(((Ptr)h) + 10, 0);
	}

	return found;
}

int FillQemulator(int fd, qdos_file_hdr *h, struct mdvFile *m)
{
	q_emulator_hdr header;
	int res = 0, curpos, bytes;

	assert(sizeof(q_emulator_hdr) == 44);
	assert(sizeof(qdos_file_hdr) == 64);

	/* save file position */
	curpos = lseek(fd, 0, SEEK_CUR);

	lseek(fd, 0, SEEK_SET);
	bytes = read (fd, (void *)&header, sizeof(header));

	if(!strncmp((char *)header.h_header, "]!QDOS File Header", 18)) {
		h->f_type = header.f_type;
		h->f_datalen = header.f_datalen;

		if (m) {
			SET_SEEKBASE(m, header.h_wordlen * 2);
		}
		lseek(fd, header.h_wordlen * 2, SEEK_SET);

		res = 1;
	}

	/* restore file position */
	lseek(fd, curpos, SEEK_SET);

	return res;
}

void FillXHXtcc(int fd, qdos_file_hdr *h)
{
	int curpos;
	uint8_t buffer[8];
	ssize_t res;

	curpos = lseek(fd, 0, SEEK_CUR);

	lseek(fd, -8, SEEK_END);
	res = read(fd, buffer, 8);

	if (!strncmp((char *)buffer, "XTcc", 4)) {
		h->f_type = 1;
		h->f_datalen = *(uint32_t *)(buffer +4);
	}

	lseek(fd, curpos, SEEK_SET);
}

const q_emulator_hdr q_em_template = {
	.h_header = "]!QDOS File Header",
	.h_wordlen = 15,
	.f_access = 0,
	.f_type = 0,
	.f_datalen = 0,
};

void QHSetHeader(qdos_file_hdr *h, int fd, struct mdvFile *f, int fstype)
{
	char *fsmount, *uxname;
	struct stat stat;
	uint8_t buffer[512];
	q_emulator_hdr q_em_hdr;
	int startpos, curpos, flen, perms, err;
	void *fbuffer = NULL;

	if (!fd) {
		printf("Unexpected closed file\n");
		return;
	}

	startpos = lseek(fd, 0, SEEK_CUR);

	/*
	 * if f_type has gone to zero we need to delete the
	 * old header.
	 */
	if (!h->f_type && GET_SEEKBASE(f)) {
		if (fstat(fd, &stat) < 0) {
			perror("QHSetHeader: fstat:");
		}
		flen = stat.st_size;

		fbuffer = malloc(flen - GET_SEEKBASE(f));

		lseek(fd, GET_SEEKBASE(f), SEEK_SET);
		err = read(fd, fbuffer, flen - GET_SEEKBASE(f));
		if (err < flen) {
			fprintf(stderr, "QHSetHeader: truncated read");
		}

		lseek(fd, 0, SEEK_SET);

		err = write(fd, fbuffer, flen - GET_SEEKBASE(0));
		err = ftruncate(fd, flen - GET_SEEKBASE(f));

		lseek(fd, startpos - GET_SEEKBASE(f), SEEK_SET);

		free(fbuffer);

		SET_SEEKBASE(f, 0);

		return;
	}

	/* We only need a header if type is set */
	if (!h->f_type && (h->f_type != 255))
		return;

	/* Do we already have space or do we need to move */
	if(!GET_SEEKBASE(f)) {
		if (fstat(fd, &stat) < 0) {
			perror("QHSetHeader: fstat:");
		}
		flen = stat.st_size;

		/* start moving data */
		curpos = lseek(fd, -(flen % 512), SEEK_END);
		if (ftruncate(fd, flen + QEMULATOR_SHORT_HEADER) < 0) {
			perror("QHSetHeader: ftruncate:");
		}

		if (flen % 512) {
			err = read(fd, buffer, flen % 512);
			lseek(fd, curpos + QEMULATOR_SHORT_HEADER, SEEK_SET);
			err = write(fd, buffer, flen % 512);
		}

		while (flen / 512) {
			curpos = lseek(fd, curpos - 512, SEEK_SET);
			err = read(fd, buffer, 512);
			lseek(fd, curpos + QEMULATOR_SHORT_HEADER, SEEK_SET);
			err = write(fd, buffer, 512);

			flen -= 512;
		}

		SET_SEEKBASE(f, QEMULATOR_SHORT_HEADER);

		startpos += QEMULATOR_SHORT_HEADER;
	}

	memcpy(&q_em_hdr, &q_em_template, sizeof(q_em_hdr));
	q_em_hdr.f_type = h->f_type;
	q_em_hdr.f_datalen = h->f_datalen;

	lseek(fd, 0, SEEK_SET);
	err = write(fd, &q_em_hdr, QEMULATOR_SHORT_HEADER);
	lseek(fd, startpos, SEEK_SET);
}

typedef union {
	char *pth;
	int fd;
} pvf;

#define FILDES 0
#define PATH 1

void QGetHeaderFromFile(qdos_file_hdr *h, struct mdvFile *f)
{
	int fd, err, seekbase, curpos, i, j;
	struct stat buf;

	fd = GET_HFILE(f);
	curpos = lseek(fd, 0, SEEK_CUR);

	err = fstat(fd, &buf);
	if (err) {
		perror("QGetHeaderFromFile fstat");
		h->f_length = 0;
		h->f_access = 0;
		h->f_type = 0;
		h->f_datalen = 0;
		h->f_reserved = 0;
	} else {

		seekbase = GET_SEEKBASE(f);

		h->f_length = SDL_SwapBE32((buf.st_size -seekbase) + 64);
		h->f_update = SDL_SwapBE32(ux2qltime(buf.st_mtime));

		if (buf.st_mode & S_IFDIR)
			i = 255;
		else
			i = 0;
		h->f_type = i;

		if (i == 255 && ((j = SDL_SwapBE16(h->f_szname)) < 36)) {
			h->f_name[j] = DIR_SEPARATOR;
		}
	}

	if (!FillQemulator(fd, h, f))
		FillXHXtcc(fd, h);

	lseek(fd, curpos, SEEK_SET);
}

void QGetHeaderFromPath(qdos_file_hdr *h, char *path, char *mount)
{
	int fd;
	char mpath[4200];
	struct mdvFile f;

	strcpy(mpath, mount);

	fd = qopenfile(mpath, path, O_RDONLY | O_BINARY, 0, 4000);
	if (fd < 0) {
		perror("QGetHeaderFromPath: open");

		return;
	}

	SET_HFILE(&f, fd);

	QGetHeaderFromFile(h, &f);

	close(fd);
}

void ux2qlpath(char *qname, char *uxname)
{
	char c;

	while ((c = *qname++ = *uxname++))
		if (c == '/')
			qname[-1] = '_';
}

static int templ_count = 0;
static char template[100] = TEMPDIR "/QDOSXXXXXX";

int QHOpenDir(struct mdvFile *f, int fstype)
{
	FILE *xx;
	int fd, res;
	char *pname;
	char mname[256];
	char qlpath[36];
	char mount[400];
	char templ[100];

	DIR *dirp;
	struct dirent *dp;
	struct fileHeader h;

	struct stat buf;
	int fd1, err, qlen, qplen;

	long i, j, mlen;

#ifdef __WIN32__
	sqlux_getemppath(sizeof(templ), templ);
	strcat(templ, "/QDOSXXXXXX");
	fd = sqlux_mkstemp(templ);
#else
	strcpy(templ, template);
	fd = mkstemp(templ);
#endif
	if (fd < 0) {
		perror("QHOpenDir mkstemp");
		return -7;
	}
	res = unlink(templ);
	if (res < 0) {
		perror("QHOpenDir Cannot Unlink");
	}

	strncpy(mount, qdevs[GET_FILESYS(f)].mountPoints[GET_DRIVE(f)], 320);

	err = uxLookupDir(mount, NAME_REF(f), f, mname, fstype);
	if (!err)
		return -7;

	/*printf("dirname is %s, orig name is %s\n",
	 mname,NAME_REF(f)+2);*/

	/* now fill the correct (sub)directory name into QDOS header */
	mlen = strlen(mname);
	mlen = (mlen == 0 ? 0 : mlen);
	WW(NAME_REF(f), mlen);
#if 0
  if (mlen)
    *(char*)(NAME_REF(f)+2+mlen)=DIR_SEPARATOR;
#endif

	if (!(dirp = qopendir(mount, mname, 400))) {
		perror("could not find directory");
		printf("mount point %s \t name %s \n", mount, mname);
		return -7;
	}
#if 0
  else printf("opened dir %s\n",mname);
#endif

	if (mname[0] != 0) {
		i = strlen(mname);
		if (mname[i - 1] != '/') {
			mname[i++] = '/';
			mname[i] = 0;
		}
	}

	strncpy(GET_FCB(f)->uxname, mname, 256);
	ux2qlpath(qlpath, mname);
	qplen = strlen(qlpath);
	mlen = strlen(mname);
	pname = mname + mlen;

	/*printf("pathname %s %s\n",mname,qlpath);*/

	while ((dp = readdir(dirp)) != NULL) {
		int unused;
		char *dotptr;

		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") ||
		    !strcmp(dp->d_name, ".-UQLX-"))
			continue;

		for (i = 0; i < 17; i++)
			*(((char *)&h) + i) = 0;

		strncpy(REF_FNAME(&h) + 2, qlpath, 32);
		strncpy(REF_FNAME(&h) + 2 + qplen, dp->d_name,
			37 - min(qplen, 32));
		WW((Ptr)REF_FNAME(&h), min(36, qplen + strlen(dp->d_name)));

		strncpy(pname, dp->d_name, 255 - mlen);
		/* printf("pathname: %s\n",mname); */

		QGetHeaderFromPath((qdos_file_hdr *)&h, mname, mount);

		/* convert dots to underscores */
		if (fstype == 2) {
			while ((dotptr=memchr(REF_FNAME(&h) + 2, '.', 36))) {
				*dotptr = '_';
			}
		}

		unused = write(fd, &h, 64);
	}
	for (i = 0; i < 64; i++)
		*(((char *)&h) + i) = 0;

	closedir(dirp);

	lseek(fd, 0, SEEK_SET);
#ifndef __WIN32__
	fcntl(fd, F_SETFL, O_RDONLY);
#endif
	SET_HFILE(f, fd);

	return 0;
}

int FSClose(int fd)
{
	int err;

	err = close(fd);
	/*printf("FSClose file %d, res %d\n",fd,err);*/
	return 0;
}

int HOpenDF(int drive, long dir, unsigned char *name, long perm,
	    struct mdvFile *f, int canCreate, int fstype)
{
	int fd, i, err;
	char mname[64], mount[400];
	struct fileHeader h;

	/*printf("calling HOpenDF %s, perm %d\n",name+2,perm);*/
	strncpy(mount, qdevs[GET_FILESYS(f)].mountPoints[GET_DRIVE(f)], 320);
	err = uxLookupFile(mount, (char *)name, f, mname, canCreate, fstype);
	strncpy(GET_FCB(f)->uxname, mname, 256);
	/*printf("HOpenDF name lookup ->%s, %s found, res %d\n",mname,(err?"":"not"),err);*/

	if (!err)
		return -7;
	fd = qopenfile(mount, mname, perm | O_BINARY, 0666, 400);

	FillQemulator(fd, (qdos_file_hdr *)&h, f);

	SET_HFILE(f, fd);
	if (fd < 0)
		return -7;

	lseek(fd, GET_SEEKBASE(f), SEEK_SET);

	return 0;
}

int HDelete(int drive, int dir, unsigned char *name, struct mdvFile *f,
	    int fstype)
{
	char mname[64], mount[400];
	int err;

	strncpy(mount, qdevs[GET_FILESYS(f)].mountPoints[GET_DRIVE(f)], 320);
	err = uxLookupFile(mount, (char *)name, f, mname, 0, fstype);

	if (!err)
		return -7;

	/*printf("calling HDelete %s\n",name+2);*/
	qunlinkfile(mount, mname, 400);

	return 0;
}

int flen(int fd)
{
	struct stat buf;
	int err;

	err = fstat(fd, &buf);
	if (!err)
		return buf.st_size;
	else
		return -1;
}

int HDfLen(struct mdvFile *f, int fstype)
{
	return flen(GET_HFILE(f));
}

/*static char buff[512];*/
int QHread(int fd, w32 *addr, long *count, Cond lf)
{
	int cnt, startpos;
	int c, fn, err, sz, e;
	Ptr /*to,from,*/ p, i = 0;
	w32 from, to;

	cnt = *count;
	from = *addr;
	startpos = lseek(fd, 0, SEEK_CUR);

	if (cnt + startpos > flen(fd))
		cnt = flen(fd) - startpos;
	to = from + cnt;

	if (from < 131072) {
		lseek(fd, 131072 - from, SEEK_CUR);
		from = 131072;
	}
	if (to >= RTOP)
		to = RTOP;
	cnt = to - from;

	if (cnt > 0) {
		if (lf) {
			for (i = 0, fn = cnt, p = (Ptr)memBase + from; fn > 0;
			     fn = fn - 1024, p = p + 1024) {
				sz = min(1024, fn);
				err = read(fd, p, sz);
				if (err <= 0) {
					cnt = (Ptr)p - (Ptr)memBase - from;
					if (cnt > 0) {
						e = QERR_NC;
						goto ret;
					} else {
						cnt = 0;
						e = qmaperr();
						goto ret;
					}
				}
				if ((i = memchr(p, 10, sz)))
					break;
			}
			if (i)
				cnt = (Ptr)i - (Ptr)memBase - from + 1;

		} else {
			cnt = read(fd, (Ptr)memBase + from, cnt);
			if (cnt < 0 && eretry()) {
				cnt = 0;
				e = QERR_NC;
				goto ret;
			}
		}
	}
	if (cnt < *count && (!lf || (lf && !i))) {
		e = QERR_EF;
	} else if (!i && lf && cnt == *count)
		e = QERR_BF;
	else
		e = 0;
	lseek(fd, startpos + cnt, SEEK_SET);

ret:
	*count = cnt;
	ChangedMemory(from, from + cnt);

	return e;
}

int QWrite(int fd, Ptr addr, long *count)
{
	int err;

	err = write(fd, addr, *count);
	if (err <= 0 && *count > 0) {
		*count = 0;
		return qmaperr();
	}
	*count = err;
	return 0;
}

/* qln name, not null-terminated */
int rename_file(struct mdvFile *f, int fd, char *qln, int qlen, int fstype)
{
	char *u, *name, mount[4200], ren[400], mname[64];
	char temp[256];
	struct fileHeader hh;
	struct fileHeader *h;
	int dnlen, res = 0;

	if (GET_FILESYS(f) < 0) {
		char nname[4200];

		name = GET_FCB(f)->uxname;
		strncpy(nname, qln, qlen);
		nname[qlen] = 0;

		res = rename(name, nname);
	} else {
		name = mount;
		strncpy(mount, qdevs[GET_FILESYS(f)].mountPoints[GET_DRIVE(f)],
			320);
		strncpy(ren, qdevs[GET_FILESYS(f)].mountPoints[GET_DRIVE(f)],
			320);
		qaddpath(mount, GET_FCB(f)->uxname, 320); /*qaddpath !?*/

		/* Rather assume that the new device is the same as the old one
	     * otherwise it will be interpreted as part of new name */
		dnlen = strlen(qdevs[GET_FILESYS(f)].qname);
		if ('_' == *(qln + dnlen + 1) && isdigit(*(qln + dnlen)) &&
		    !strncasecmp(qln, qdevs[GET_FILESYS(f)].qname, dnlen)) {
			qln += (dnlen + 2);
			qlen -= (dnlen + 2);
		}
		if (qlen > 36) {
			res = QERR_BN;
			goto rename_error;
		}
		/* can't rename readonly files, might be shared! */
		if (*((char *)f + 4 - (0x1e - 0x18)) == 1) {
			res = QERR_RO;
			goto rename_error;
		}
		strncpy(temp, qln, 256); /* c-strings !! */
		temp[qlen] = 0;
		mname[0] = 0;
		res = match(ren, mname, temp, 0, 0, 320, fstype);
		if (res) {
			res = QERR_EX; /* already exists */
			goto rename_error;
		}
		strncpy(ren, qdevs[GET_FILESYS(f)].mountPoints[GET_DRIVE(f)],
			320);
		mname[0] = 0;
		res = match(ren, mname, temp, 0, 1, 320, fstype);
		if (!res) {
			res = QERR_NF;
			goto rename_error;
		}
		qaddpath(ren, mname, 320);
		/* printf("rename file %s, %.*s %s\n", mount, qlen, qln, ren); */
		res = rename(mount, ren);
		res = (res) ? qmaperr() : 0;
		if (res == 0) {
			WW(NAME_REF(f), qlen);
			strncpy(NAME_REF(f) + 2, temp,
				36); /* set new name for IOSS !*/

			if (GET_FILESYS(f) < 0)
				mount[0] = 0;
			else
				strncpy(mount,
					qdevs[GET_FILESYS(f)]
						.mountPoints[GET_DRIVE(f)],
					320);
			/* now change internal def so QHSetHeader applies to the
		 correct file */
			strncpy(GET_FCB(f)->uxname, mname,
				(GET_FILESYS(f) < 0 ? 4000 : 256));
		}

	rename_error:
		return res;
	}

	return res;
}

extern int rename_single(struct mdvFile *, char *, char *, char *, int);
/* rename all files affected by a directory creation */
int rename_all_files(struct mdvFile *f, char *fsmount, char *uxname)
{
	char *lastseg, *p, mount[4200];
	char temp[256];
	DIR *dirp;
	struct dirent *dp;
	int uxlen;

	/* uxname can't be empty (hint: create a root-dir?)*/
	strncpy(temp, uxname, 256);
	p = strrchr(temp, '/');
	if (p)
		*p = 0;
	else
		temp[0] = 0; /* remove last subdir segment */
	strncpy(mount, fsmount, 4000);
	dirp = qopendir(mount, temp, 4000);

	strncpy(temp, uxname, 256);
	if (p)
		lastseg = p + 1;
	else
		lastseg = temp;
	uxlen = strlen(lastseg);

	while ((dp = readdir(dirp))) {
		if (!strcmp(dp->d_name, ".-UQLX-") ||
		    !strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		if (strncasecmp(dp->d_name, lastseg, uxlen) ||
		    (*(dp->d_name + uxlen) != '_'))
			continue;

		/* OK, needs renaming */

		rename_single(f, fsmount, temp, dp->d_name, uxlen);
	}
	closedir(dirp);
	return 0;
}

int rename_single(struct mdvFile *f, char *fsmount, char *localdir, char *name,
		  int nsdirlen)
{
	char mount[4200];
	char temp[4200];
	struct fileHeader hh;
	int res;

	strncpy(mount, fsmount, 4200);
	/* printf("rs: find header: fsmount %s, name %s\n",fsmount,name); */

	strncpy(mount, fsmount, 4200);

	strncpy(temp, mount, 4200);
	qaddpath(temp, name, 4200);
	temp[nsdirlen + strlen(mount) + 1] = '/';

	qaddpath(mount, name, 4200);
	/* printf("rename file %s, to %s\n", mount, temp); */
	res = rename(mount, temp);
	if (res) {
		perror("rename failed");
		return 0;
	}
	temp[0] = 0;
	/* change internal def for open files */
	strncpy(mount, name, 4200);
	strncpy(temp, name, 4200);
	temp[nsdirlen] = '/';
	for (f = qdevs[GET_FILESYS(f)].FileList[GET_DRIVE(f)];
	     f != nil && ((uintptr_t)f & 1) == 0; f = GET_NEXT(f)) {
		if (!strncasecmp(GET_FCB(f)->uxname, mount, 64))
			strncpy(GET_FCB(f)->uxname, temp, 64);
	}

	strncpy(mount, fsmount, 4200);

	return 0;
}

/* fstype should make difference in rename only? */
int QHostIO(struct mdvFile *f, int op, int fstype)
{
	struct fileHeader hh;
	struct fileHeader *h;
	int fd, err, seekbase;
	long count;
	long from, to;
	long i;

	w8 *p;
	w16 *w;
	Ptr s, addr;
	w32 qaddr;
	int unused;

	char c;

	fd = GET_HFILE(f);
	seekbase = GET_SEEKBASE(f);

	*reg = 0;

	/*printf("QHostIO op %x-%d file %x, d1=%d, d2=%d, a1=%d  \n",op,op,f,reg[1],reg[2],aReg[1]);*/

	switch (op) {
	case 0: /* check for pending input */
		if (!(lseek(fd, 0, SEEK_CUR) < HDfLen(f, fstype)))
			*reg = -10;
		break;
	case 1: /* fetch byte */
		err = read(fd, &c, 1);
		if (err == 1)
			*((char *)reg + 4 + RBO) = c;
		else
			*reg = -10;
		break;
	case 2: /* fetch LF-terminated line */
		count = (uw16)reg[2];
		qaddr = aReg[1];

		reg[0] = QHread(fd, &qaddr, &count, true);
		reg[1] = count;
		aReg[1] = qaddr + count;

		/*printf("io.fline res: %d, count= %d\n",*reg,count);*/
		break;
	case 3: /* fetch string */
		qaddr = aReg[1];
		count = (uw16)reg[2];

		reg[0] = QHread(fd, &qaddr, &count, false);

		reg[1] = count;
		aReg[1] = qaddr + count;

		break;
	case 5: /* send byte */
		count = 1;
		reg[0] = QWrite(fd, (Ptr)reg + 4 + RBO, &count);
		break;
	case 7: /* send string */
		count = (uw16)reg[2];
		reg[0] = QWrite(fd, (Ptr)memBase + aReg[1], &count);
		reg[1] = count;
		aReg[1] += count;
		break;
	case 0x40: /* check pending ops */
	case 0x41: /* flush buffers */
		/* return success */
		break;
	case 0x43: /* position file pointer relative */
		reg[1] = qlseek(f, reg[1], SEEK_CUR);
		break;
	case 0x42: /* position file pointer absolute */
		if (reg[1] < 0) {
			reg[1] = 0;
			/* *reg=-10; */ /* end of file */
		}
		if (reg[1] > (flen(fd) - seekbase)) {
			*reg = -10;
		}
		reg[1] = qlseek(f, reg[1], SEEK_SET);
		break;
	case 0x45: /* medium information */
		reg[1] = 0x7fff7fff; /* fake number for now */
		memset((char *)memBase + aReg[1], ' ', 10);
		if (GET_FILESYS(f) >= 0)
			strncpy((char *)memBase + aReg[1],
				qdevs[GET_FILESYS(f)].mountPoints[GET_DRIVE(f)],
				10);
		else
			strncpy((char *)memBase + aReg[1], "uQVFSx     ", 10);
		break;
	case 0x46: /* set file header */
		h = aReg[1] + (Ptr)memBase;
		/*count=(uw16)reg[1];*/
		reg[1] = 14 | (reg[1] & (~0xffff));
		memcpy(&hh, h, 64);
		QHSetHeader((qdos_file_hdr *)&hh, fd, f, fstype);
		/*aReg[1]+=14;*/ /* correct but nobody complained so far */
		break;
	case 0x47: /* read file header */
		count = reg[2] & 0x0fffe;
		if (count > 64)
			count = 64;
		if (count < 4) {
			reg[0] = QERR_BF;
			goto hrres;
		}

		/*printf("getting headers for file %s, %s\n",(Ptr)REF_FNAME(f)+2,GET_FCB(f)->uxname);*/

		reg[1] = count | (reg[1] & (~0xf));

		for (i = 0; i < 64; i++)
			*(((char *)&hh) + i) = 0;
		QGetHeaderFromFile((qdos_file_hdr *)&hh, f);
		strncpy(REF_FNAME(&hh) + 2, (Ptr)NAME_REF(f) + 2,
			min(36, RW(NAME_REF(f))));
		WW((Ptr)REF_FNAME(&hh), min(36, RW(NAME_REF(f))));

		count = (count - 4) >> 1;
		w = (w16 *)&hh;

		WriteLong(aReg[1], RL((w32 *)w) - 64);

		aReg[1] += 4;
		w += 2;

		while (count--) {
			WriteWord(aReg[1], RW(w++));
			aReg[1] += 2;
		}
	hrres:

		break;
	case 0x48: /* read file into memory */
		qaddr = aReg[1];
		count = reg[2];

		reg[0] = QHread(fd, &qaddr, &count, false);

		aReg[1] = qaddr + count;
		break;
	case 0x49: /* save file from memory */
		count = reg[2];
		reg[0] = QWrite(fd, (Ptr)memBase + aReg[1], &count);
		aReg[1] += count;
		break;
	case 36: /* clear righthand end of cursor line (viene mandato dal Basic durante un save) */
	case 10: /* read cursor position (viene mandato da ZOO) */
	case 11: /* get window size and cursor position */
	case 19: /* decrement cursor */
	case 20: /* increment cursor */
		*reg = QERR_BP; /* mandati da tabelleneu_obj mentre legge un file */
		break;

	case 74:
		/* FS.RENAME : a1= new name */
		{
			short qlen;
			int res;
			char *qln;

			qln = (Ptr)memBase + aReg[1] + 2;
			qlen = RW((Ptr)memBase + aReg[1]);

			res = rename_file(f, fd, qln, qlen, fstype);

			*reg = res;
		}
		break;

	case 75: /* truncate file to current position */
		i = lseek(fd, 0, SEEK_CUR);
		unused = ftruncate(fd, i);
		break;
	case 76:
		/* FS.DATE */
		/* D1=-1 read, 0 current time, >0 secs count */
		/* D2.b 0=Update, 1=Backup */
		/* returns D1 = date*/
#if 1
	{
		struct utimbuf tb;
		struct stat ft;
		struct timeval tp;
		int tm, res;
		char *name, mount[400];

		res = 0;
		fstat(fd, &ft);
		if (reg[1] < 0) {
			reg[1] = ux2qltime(ft.st_mtime);
		} else {
			gettimeofday(&tp, (void *)0);
			tm = reg[1] ? ql2uxtime(reg[1]) : tp.tv_sec;
			tb.actime = ft.st_atime;
			tb.modtime = tm;

			if (GET_FILESYS(f) < 0)
				name = GET_FCB(f)->uxname;
			else {
				name = mount;
				strncpy(mount,
					qdevs[GET_FILESYS(f)]
						.mountPoints[GET_DRIVE(f)],
					320);
				strncat(mount, GET_FCB(f)->uxname, 64);
			}

			res = utime(name, &tb);
		}
		if (res)
			*reg = qmaperr();
		else
			*reg = 0;
	}
#else
		*reg = -15;
#endif
	break;
	case 77:
		/* MKDIR */
		i = lseek(fd, 0, SEEK_END);
		if (i > 0) {
			*reg = -15;
			break;
		}
		/*printf(" MAKEDIR %s \n",GET_FCB(f)->uxname);*/

		if (GET_FILESYS(f) < 0) {
			unlink(GET_FCB(f)->uxname);
#ifdef __WIN32__
			i = mkdir(GET_FCB(f)->uxname);
#else
			i = mkdir(GET_FCB(f)->uxname, 0777);
#endif
			if (i != 0)
				*reg = QERR_NF;
		} else {
			char mount[400];

			strncpy(mount,
				qdevs[GET_FILESYS(f)].mountPoints[GET_DRIVE(f)],
				320);
			qaddpath(mount, GET_FCB(f)->uxname, 400);
			unlink(mount);
#ifdef __WIN32__
			i = mkdir(mount);
#else
			i = mkdir(mount, 0777);
#endif
			if (i != 0)
				*reg = QERR_NF;
			if (fstype == 2)
				rename_all_files(
					f,
					qdevs[GET_FILESYS(f)]
						.mountPoints[GET_DRIVE(f)],
					GET_FCB(f)->uxname);
		}
		break;

	case 78:
		/*  FS.VERS : read/write file version  */
		/*  D1=0  read, >0 set to this version */
		/*  D1=-1 read, increment version      */
		/*  returns D1 = vers */

		QGetHeaderFromFile((qdos_file_hdr *)&hh, f);
		if (reg[1] == 0) {
			reg[1] = GET_FVER(&hh);
		} else {
			if (reg[1] == -1) {
				reg[1] = GET_FVER(&hh);
				SET_FVER(&hh, reg[1] + 1);
			} else
				SET_FVER(&hh, reg[1]);

			QHSetHeader((qdos_file_hdr *)&hh, fd, f, fstype);
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
		long mxs;

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
		mxs = strncasecmp(qdevs[GET_FILESYS(f)].qname, "RAM", 3) ?
				    999999 :
				    32767;
		*((char *)memBase + aReg[1] + 0x1c) = 1 + GET_DRIVE(f);
		*((char *)memBase + aReg[1] + 0x1d) = 0;
		WriteWord(aReg[1] + 0x1e, 1024);
		WriteLong(aReg[1] + 0x20, mxs);
		WriteLong(aReg[1] + 0x24, mxs);
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

	/*printf("QHostIO result d0=%d, d1=%d,   a1=%d\n",reg[0],reg[1],aReg[1]);*/

	return 0;
}
