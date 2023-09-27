/*
 * (c) UQLX - see COPYRIGHT
 */

/*#include "QLtypes.h"*/
#include "QL68000.h"

#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>

#include "QL.h"
#include "xcodes.h"

#include "driver.h"
#include "dummies.h"
#include "emulator_options.h"
#include "QDOS.h"

#include "QInstAddr.h"
#include "QL_cconv.h"
#include "unix.h"
#include "unixstuff.h"
#include "memaccess.h"
#include "QVFS.h"

#define min(_a_, _b_) (_a_ < _b_ ? _a_ : _b_)
#define max(_a_, _b_) (_a_ > _b_ ? _a_ : _b_)

#ifdef __EMX__
#define strncasecmp strncmp
#endif

#ifdef strchr
#undef strchr
#endif
#define strchr(_str_, _c_) my_strchr(_str_, _c_)

extern void rts(void);
void DrvOpen(void);
void DrvIO(void);
void DrvClose(void);

int qerrno;

int prt_init(int, void *);
int prt_open(int, void **);
int prt_test(int, char *);
void prt_close(int, void *);
void prt_io(int, void *);

int num_drivers = 0;
/*#define TEST*/
/*#define IOTEST*/

open_arg prt_opt_vals[] = { 0, 1 };
open_arg prt_tra_vals[] = { 0, 1 };
open_arg prt_cmdoptions[] = { 0 }; /*simpler to check for arg presence ;-) */
open_arg prt_cmd[] = { 0 };
struct PARENTRY prt_pars[] = { { parse_option, "f", prt_opt_vals },
			       { parse_option, "t", prt_tra_vals },
			       { parse_nseparator, "_", prt_cmdoptions },
			       { parse_nseparator, "!", prt_cmd },
			       { NULL, NULL, NULL } };
struct NAME_PARS prt_name = { "PRT", 4, (struct PARENTRY *)&prt_pars };

#define BDEV
#ifdef BDEV
extern int boot_init(int, void *);
extern int boot_open(int, void **);
extern int boot_test(int, char *);
extern void boot_close(int, void *);
extern void boot_io(int, void *);

struct PARENTRY boot_pars[] = { { NULL, NULL, NULL } };
struct NAME_PARS boot_name = { "BOOT", 0, (struct PARENTRY *)&boot_pars };
#endif

#ifdef SERIAL
extern int ser_init(int, void *);
extern int ser_open(int, void **);
extern int ser_test(int, char *);
extern void ser_close(int, void *);
extern void ser_io(int, void *);

open_arg ser_unit[] = { 1 };
open_arg ser_ovals1[] = { 0, 1, 2, 3, 4 };
open_arg ser_ovals2[] = { -1, -1, 0 };
open_arg ser_ovals3[] = { -1, -1, 0, 1 };
open_arg ser_sv0[] = { 0 }; /* Dummy '_' separator*/
open_arg ser_sv[] = { 9600 };

struct PARENTRY ser_pars[] = { { parse_value, NULL, ser_unit },
			       { parse_option, "OEMS", ser_ovals1 },
			       { parse_option, "HI", ser_ovals2 },
			       { parse_option, "RZC", ser_ovals3 },
			       { parse_separator, "_", ser_sv0 }, /* dummy */
			       { parse_separator, "b", ser_sv },
			       { NULL, NULL, NULL } };
struct NAME_PARS ser_name = { "SER", 6, (struct PARENTRY *)&ser_pars };
#endif

#ifdef NEWPTY
extern int pty_init(int, void *);
extern int pty_open(int, void **);
extern int pty_test(int, char *);
extern void pty_close(int, void *);
extern void pty_io(int, void *);

open_arg pty_ovals1[] = { 0, 1, 2 };
open_arg pty_ovals2[] = { -1, -1, 0, 1, 3 };
open_arg pty_val[] = { (open_arg) "" };
struct PARENTRY pty_pars[] = { { parse_option, "IK", pty_ovals1 },
			       { parse_option, "RZCT", pty_ovals2 },
			       { parse_nseparator, "_", pty_val },
			       { NULL, NULL, NULL } };
struct NAME_PARS pty_name = { "PTY", 3, (struct PARENTRY *)&pty_pars };
#endif

#ifdef POPEN_DEV
extern int popen_init(int);
extern int popen_open(int, void **);
extern int popen_test(int, char *);
extern void popen_close(int, void *);
extern void popen_io(int, void *);
open_arg popen_val[] = { (open_arg) "" };
struct PARENTRY popen_pars[] = { { parse_nseparator, "_", popen_val },
				 { NULL, NULL, NULL } };
struct NAME_PARS popen_name = { "popen", 1, (struct PARENTRY *)&popen_pars };
#endif

#include "QLfiles.h"
#include "QFilesPriv.h"

#ifdef IPDEV
#include "QLip.h"
open_arg ip_host[] = { (open_arg) "" };
open_arg ip_port[] = { (open_arg) "" };
struct PARENTRY ip_pars[] = { { parse_mseparator, "_:", ip_host }, /* dummy */
			      { parse_mseparator, ":", ip_port },
			      { NULL, NULL, NULL } };
struct NAME_PARS tcp_name = { "TCP", 2, (struct PARENTRY *)&ip_pars };
struct NAME_PARS udp_name = { "UDP", 2, (struct PARENTRY *)&ip_pars };
struct NAME_PARS uxs_name = { "UXS", 2, (struct PARENTRY *)&ip_pars };
struct NAME_PARS uxd_name = { "UXD", 2, (struct PARENTRY *)&ip_pars };
struct NAME_PARS sck_name = { "SCK_", 0, NULL };

#endif

#ifdef TEST
int bg_init(int, void *);
int bg_open(int, void **);
int bg_test(int, char *);
void bg_close(int, void *);
void bg_io(int, void *);

open_arg bg_val[] = { 133 };
open_arg bg_ovals[] = { -1,  'a', 'b', 'c', 'd', 'e', 'f',
			'g', 'h', 'i', 'j', 'k', 'l' };
open_arg bg_sv1[] = { 11 };
open_arg bg_sv2[] = { 12 };
open_arg bg_sv3[] = { 13 };
open_arg bg_sv4[] = { (open_arg) "string1" };
open_arg bg_sv5[] = { (open_arg) "string2" };
struct PARENTRY bg_pars[] = { { parse_value, NULL, bg_val },
			      { parse_option, "abcdefghijkl", bg_ovals },
			      { parse_separator, "_", bg_sv1 },
			      { parse_separator, "/", bg_sv2 },
			      { parse_separator, "x", bg_sv3 },
			      { parse_nseparator, "--", bg_sv4 },
			      { parse_nseparator, ",", bg_sv5 },
			      { NULL, NULL, NULL } };
struct NAME_PARS bg_name = { "BG", 4, (struct PARENTRY *)&bg_pars };
#endif

#ifdef XSCREEN
#if 0
extern int scr_init(int, void *);
extern int scr_open(int, void**);
extern int scr_test(int, char*);
extern int con_test(int, char*);
extern void scr_close(int, void *);
extern void scr_io(int, void *);
#endif

open_arg scr_sv1[] = { 448 };
open_arg scr_sv2[] = { 180 };
open_arg scr_sv3[] = { 32 };
open_arg scr_sv4[] = { 16 };
open_arg scr_sv5[] = { -1 };

struct PARENTRY scr_pars[] = {
	{ parse_separator, "_", scr_sv1 }, { parse_separator, "x", scr_sv2 },
	{ parse_separator, "a", scr_sv3 }, { parse_separator, "x", scr_sv4 },
	{ parse_separator, "_", scr_sv5 }, { NULL, NULL, NULL }
};
struct NAME_PARS scr_name = { "SCR", 6, (struct PARENTRY *)&scr_pars };
struct NAME_PARS con_name = { "CON", 6, (struct PARENTRY *)&scr_pars };
#endif

struct NAME_PARS qvf_name = { "VFS_", 0, NULL };

struct DRV Drivers[] = {
	{ 0, prt_init, prt_test, prt_open, prt_close, prt_io, &prt_name, 0 },
#ifdef SERIAL
	{ 0, ser_init, ser_test, ser_open, ser_close, ser_io, &ser_name, 0 },
#endif
#ifdef NEWPTY
	{ 0, pty_init, pty_test, pty_open, pty_close, ser_io, &pty_name, 0 },
#endif
#ifdef BDEV
	{ 0, boot_init, boot_test, boot_open, boot_close, boot_io, &boot_name,
	  0 },
#endif
#ifdef TEST
	{ 0, bg_init, bg_test, bg_open, bg_close, bg_io, &bg_name, 0 },
#endif
#ifdef IPDEV
	{ 0, ip_init, ip_test, ip_open, ip_close, ip_io, &tcp_name, 0 },
	{ 0, ip_init, ip_test, ip_open, ip_close, ip_io, &udp_name, 0 },
	{ 0, ip_init, ip_test, ip_open, ip_close, ip_io, &uxs_name, 0 },
	{ 0, ip_init, ip_test, ip_open, ip_close, ip_io, &uxd_name, 0 },
	{ 0, ip_init, ip_test, ip_open, ip_close, ip_io, &sck_name, 0 },
#endif
#ifdef POPEN_DEV
	{ 0, popen_init, popen_test, popen_open, popen_close, popen_io,
	  &popen_name, 0 },
#endif
#if 0 /* XSCREEN */
  {0,scr_init, scr_test, scr_open, scr_close, scr_io, &scr_name, 0x100},
  {0,scr_init, con_test, scr_open, scr_close, scr_io, &con_name, 0x100},
#endif

/* QVFS should always be LAST in the list */
#ifdef QVFS
	{ 0, qvf_init, qvf_test, qvf_open, qvf_close, qvf_io, &qvf_name, 0 },
#endif
	{ 0, NULL, NULL, NULL, NULL, NULL, NULL }
};

#if 0
/* emulator opcodes */
#define DEV_IO_INSTR 0xaaaa
#define DEV_OPEN_INSTR 0xaaab
#define DEV_CLOSE_INSTR 0xaaac
#endif

w32 DEV_IO_ADDR, DEV_CLOSE_ADDR;

struct DRV *dget_drv()
{
	struct DRV *p;

	p = Drivers;
	while (p->open != NULL) {
		if (p->ref == aReg[3] + 0x18)
			return p;
		p++;
	}
	return 0;
	/*if (found=(*(p->open_test))(p-Drivers,name)) break;*/
}

static void InitDevDriver(struct DRV *driver, int indx)
{
	w32 savedRegs[4];
	w32 *p;
	char *name = (driver->namep)->name;

	BlockMoveData(aReg, savedRegs, 4 * sizeof(w32));
	reg[1] = 40 + strlen(name);
	if ((strlen(name) & 1) != 0)
		reg[1]++;

	if (driver->slot != 0) {
		if (driver->slot < reg[1]) {
			printf("requested driver size for driver %s too small: %d\n",
			       name, driver->slot);
			goto ddier;
		}
		reg[1] = driver->slot;
	}

	reg[2] = 0;
	QLtrap(1, 0x18,
	       200000l); /* allocate memory for the driver linkage block */
	if ((*reg) == 0) {
		driver->ref = aReg[0];
		p = (w32 *)(aReg[0] + (Ptr)memBase + 4);
		WL(p, DEV_IO_ADDR); /* io    */
		WL(p + 1, (w32)((Ptr)(p + 3) - (Ptr)memBase)); /* open  */
		WL(p + 2, DEV_CLOSE_ADDR); /* close */

		WW(p + 3, DEVO_CMD_CODE);

		strncpy((Ptr)(p + 6) + 4, name, 36); /* name for QPAC2 etc */
		WW((Ptr)(p + 3 + 3) + 2, strlen(name));
		WL((Ptr)(p + 3) + 2,
		   0x264f4eba); /* so much code is needed to fool QPAC2 ...*/
		WL((Ptr)(p + 4) + 2, 0x2c566046);
		WL((Ptr)(p + 5) + 2, 0x6044604a);

		if ((*(driver->init))(indx, p - 1) < 0)
			goto ddier;

		QLtrap(1, 0x20,
		       20000l); /* link directory device driver in IOSS */
	}
ddier:
	BlockMoveData(savedRegs, aReg, 4 * sizeof(w32));
}

void InitDrivers()
{
	struct DRV *p = Drivers;

	DEV_IO_ADDR = 0x1C020;
	DEV_CLOSE_ADDR = 0x1C022;
	/*  DEV_OPEN_ADDR=0x1C024;*/

	WW(((uw16 *)((Ptr)memBase + DEV_IO_ADDR)), DEVIO_CMD_CODE);
	WW(((uw16 *)((Ptr)memBase + DEV_CLOSE_ADDR)), DEVC_CMD_CODE);
	/*WW(((uw16*)((Ptr)memBase+DEV_OPEN_ADDR)),DEV_OPEN_INSTR);*/

	qlux_table[DEVO_CMD_CODE] = DrvOpen;
	qlux_table[DEVIO_CMD_CODE] = DrvIO;
	qlux_table[DEVC_CMD_CODE] = DrvClose;

	while (p->open != NULL) {
		struct DRV *old_p = p++;
		InitDevDriver(old_p, p - Drivers);
		num_drivers++;
	}
}

char *a0addr(Cond check)
{
	char *f;
	if (*aReg < 131072 || *aReg > RTOP - 130) {
		return nil;
	}
	f = (char *)((Ptr)memBase + ((*aReg) & ADDR_MASK_E));
	if (!check)
		return f;
	if (DGET_ID(f) == DRV_ID)
		return f;
	return nil;
}

void DrvIO(void)
{
	char *f;
	struct DRV *driver;
	int ix;

	if ((long)((Ptr)gPC - (Ptr)memBase) - 2 != DEV_IO_ADDR) {
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
		return;
	}
	f = a0addr(false);

	if (f == nil) {
		*reg = QERR_NO; /* overflow */
		rts();
		return;
	}

#if 0
  if (DGET_ID(f)!=DRV_ID)
    {
      reg[0]= QERR_NO;
      rts();
      return;
    }
#endif

	driver = dget_drv(); /*&Drivers[DGET_DRV(f)];*/
	ix = driver - Drivers;

	if (driver == 0) {
		printf("possible driver problem ??\n");
		return;
	}

	(*(driver->io))(ix, DGET_PRIV(f));

	rts();
}

void DrvOpen(void)
{
	char *f = nil;
	char *name;
	struct DRV *p, *drv = Drivers;
	void *priv;
	int err, found = 0;

	name = (char *)((Ptr)memBase + ((*aReg) & ADDR_MASK_E));

	/* get device */

	p = dget_drv();
	found = (*(p->open_test))(p - Drivers, name);

	if (!found) {
		reg[0] = QERR_NF;
		goto end;
	}
	if (found == -2) {
		reg[0] = qerrno;
		goto end;
	}
	if (found == -1) {
		reg[0] = QERR_BN;
		goto end;
	}

	err = (*(p->open))(p - Drivers, &priv);

	if (err == 0) {
		reg[1] = DRV_SIZE;
		reg[2] = 0;
		QLvector(0xc0, 20000l);
		if ((uw16)reg[0])
			goto end;

		f = a0addr(false);
		if (f) {
			DSET_ID(f, DRV_ID);
			DSET_PRIV(f, priv);
		}
	}
	if (err >= 0)
		reg[0] = 0;

	if (err < 0)
		reg[0] = err;
end:
	rts();
}

void DrvClose(void)
{
	char *f;
	struct DRV *driver;
	int ix;
	w32 saved_regs[16];

	if ((long)((Ptr)gPC - (Ptr)memBase) - 2 != DEV_CLOSE_ADDR) {
		exception = 4;
		extraFlag = true;
		nInst2 = nInst;
		nInst = 0;
		return;
	}

	//save_regs(saved_regs);
	f = a0addr(false);

	if (f == nil) {
		*reg = QERR_NO; /* overflow */
		rts();
		return;
	}

#if 0
  if (DGET_ID(f)!=DRV_ID)
    {
      reg[0]= QERR_NO;
      rts();
      return;
    }
#endif

	driver = dget_drv(); /*&Drivers[DGET_DRV(f)];*/
	ix = driver - Drivers;

	if (driver == 0) {
		printf("possible driver problem ??\n");
		return;
	}

	(*(driver->close))(ix, DGET_PRIV(f));

	QLvector(0xc2, 20000l);
	//restore_regs(saved_regs);

	rts();
}

/* *********** generic helper routines  *********** */

char *strchr_noalpha(char *str, char c)
{
	char *res = (char *)0;
	do {
		if (*str == c) {
			res = str;
			break;
		}
	} while (*str++);

	return res;
}
char *strchr_alpha(char *str, char c)
{
	char *res = (char *)0;

	c = c & 0xdf;
	do {
		if (((*str) & 0xdf) == c) {
			res = str;
			break;
		}
	} while (*str++);

	return res;
}

char *my_strchr(char *str, char c)
{
	if (isalpha(c))
		return strchr_alpha(str, c);
	else
		return strchr_noalpha(str, c);
}

static char rest_name[1025];
static char *ppname;

int parse_separator(char **name, int nlen, char *opts, open_arg *vals,
		    open_arg *res)
{
#ifdef TEST
	printf("parse_separator: \'%c\' : %s\n", *opts, *name);
#endif
	if (tolower(**name) != *opts) {
		res->i = vals->i;
#ifdef TEST
		printf("... use default value %d\n", res->i);
#endif
		return 0;
	}

	(*name)++;
#ifdef TEST
	printf("...found, fetch value\n");
#endif
	return parse_value(name, nlen - 1, NULL, vals, res);
}

int parse_nseparator(char **name, int nnlen, char *opts, open_arg *vals,
		     open_arg *res)
{
	char *nend;
	int nlen;

#ifdef TEST
	printf("parse name: %s rest name %s\n", opts, *name);
#endif
	if (**name != *opts) {
	noval:
		res->s = vals->s;
#ifdef TEST
		printf("... use default value %s\n", res->s);
#endif
		return 0;
	}

	(*name)++;
	nend = strchr(*name, opts[1]); /**/
	if (!nend)
		nend = strchr(*name, 0);
	if (!nend || nend == *name)
		goto noval;

	nlen = nend - *name;
	strncpy(ppname, *name, nlen);
	ppname[nlen] = 0;
	res->s = ppname;

	ppname = &ppname[nlen + 1];
	*name = nend;
	if (*name < strchr(*name, 0))
		(*name)++;
	return 1;
}

#ifdef IPDEV
int parse_mseparator(char **name, int nnlen, char *opts, open_arg *vals,
		     open_arg *res)
{
	char *nend;
	int nlen;

	if (**name != *opts) {
	noval:
		res->s = vals->s;
		return 0;
	}

	(*name)++;
	nend = strchr(*name, opts[1]); /**/
	if (!nend)
		nend = strchr(*name, 0);
	if (!nend || nend == *name)
		goto noval;

	nlen = nend - *name;
	memcpy(ppname, *name, nlen);
	ppname[nlen] = 0;
	res->s = ppname;

	ppname = &ppname[nlen + 1];
	*name = nend;
	return 1;
}
#endif

int parse_value(char **name, int nlen, char *opts, open_arg *vals,
		open_arg *res)
{
	char *p = *name;
	int r;

#ifdef TEST
	printf("parse_value: %s\n", *name);
#endif

	r = strtol(*name, name, 10);
	if (*name == p) {
		res->i = vals->i;
#ifdef TEST
		printf("... take default value %d\n", res->i);
#endif
		return 0;
	} else {
		res->i = r;
		/**name=p;*/
#ifdef TEST
		printf("... return %d\n", res->i);
#endif
		return 1;
	}
}

int parse_option(char **name, int nlen, char *opts, open_arg *vals,
		 open_arg *res)
{
	char *p;

#ifdef TEST
	printf("parse_option: opt_string %s name %s\n", opts, *name);
#endif
	if (**name)
		p = strchr(opts, **name);
	else
		p = NULL;
	if (!p) {
		res->i = vals->i;
		return 0;
	}

	(*name)++;
	res->i = vals[p - opts + 1].i;
#ifdef TEST
	printf("... return optnr %d, value %d\n", p - opts + 1, res->i);
#endif
	return 1;
}

/* returns -1 bad name, 0 not found, >0 success*/
int decode_name(char *name, struct NAME_PARS *ndescr, open_arg *parblk)
{
	int res;
	open_arg rval;
	struct PARENTRY *pars;
	char *nend;
	int j;
	int i = ndescr->pcount;

	ppname = rest_name;
#ifdef TEST
	printf("decode_name: dev_name %s file name %s\n", name + 2,
	       ndescr->name);
#endif

	if (RW(name) < strlen(ndescr->name) ||
	    strncasecmp(name + 2, ndescr->name, strlen(ndescr->name)))
		return 0;
	nend = name;
	if (RW(name) > 1024)
		return -1;
	name = rest_name;

	j = RW(nend) - strlen(ndescr->name);
	strncpy(name, nend + 2 + strlen(ndescr->name), j);
	name[j] = 0;

	nend = &name[j];
	pars = ndescr->pars;
	while (i-- /*&& name<nend*/ && pars->func) {
		res = (*(pars->func))(&name, nend - name, pars->opt,
				      pars->values, &rval);
		switch (res) {
		case -1:
			return -1;
		case 0:
		case 1:
			pars++;
			*parblk++ = rval;
			break;
		default:
#ifdef TEST
			printf("decode_name problem\n");
#endif
			return -1;
		}
	}
	if (name != nend)
		return -1;
	else
		return 1;
}

static char buf[1024];

int ioskip(int (*io_read)(void *, void *, int), void *priv, int len)
{
	int res = 0, ss = 0;

	while (len > 0) {
		res = (*io_read)(priv, buf, min(len, 1024));
		len -= 1024;
		if (res > 0)
			ss += res;
		else
			break;
	}
	if (res < 0)
		return -ss;
	else
		return ss;
}

void ioread(int (*io_read)(void *priv, Ptr from, int cnt), void *priv,
	    uw32 addr, int *count, int lf)
{
	int cnt, ocnt, startpos;
	int c, fn, err, sz, e;
	char *p;
	Ptr i = 0;
	uw32 to, from;

	cnt = *count;
	from = addr;

	to = from + cnt;

	if (from < 131072) {
		err = ioskip(io_read, priv, 131072 - from);
		if (err < 0) {
			cnt = -err;
			e = QERR_NC;
			goto errexit;
		} else
			from += err;
	}
	if (to >= RTOP)
		to = RTOP;
	ocnt = cnt = to - from;

	e = 0;
	if (cnt < 0)
		cnt = 0;

	if (cnt > 0) {
		if (lf) {
			for (i = 0, fn = cnt, p = (Ptr)memBase + from;
			     fn > 0;) {
				e = err = (*io_read)(priv, p, 1);

				if (err < 0) {
					cnt = (long)((Ptr)p - (Ptr)memBase) -
					      from;
					goto errexit;
				}

				if (err == 0 && *(p - 1) != 10) {
					cnt = (long)((Ptr)p - (Ptr)memBase) -
					      from;
					e = QERR_EF; /* QERR_NC ???*/
					goto errexit;
				}

				p += e;
				fn -= e;
				if (*(p - 1) == 10) {
					i = p;
					break;
				}
			}
			if (i) {
				cnt = (long)((Ptr)i - (Ptr)memBase) - from;
				e = 0;
			} else {
				cnt = ocnt;
				e = QERR_BF;
			}
		} else /* non LF part here */
		{
			e = err = (*io_read)(priv, (Ptr)memBase + from, cnt);
			/*printf("ser_read resutl: %d\n",e);*/
			if (e <= 0) {
				if (e == 0 && cnt > 0)
					e = QERR_EF;
				cnt = 0;
				goto errexit;
			} else if (e < cnt)
				e = QERR_NC;
			else
				e = 0;
			cnt = err;
		}
	}

errexit:
	*count = cnt;
	ChangedMemory(from, from + cnt);

	reg[0] = e;
	return;
}

/* io_handle, similar to SERIO vector */

void io_handle(int (*io_read)(void *p, void *buf, int len),
	       int (*io_write)(void *p, void *buf, int len),
	       int (*io_pend)(Ptr priv), void *priv)
{
	void *addr;
	int err, res;
	char c;
	int count, rc_count;
	w32 qaddr;
	int op = (w8)reg[0];

	reg[0] = 0;

#ifdef IOTEST
	printf("call io_handle \t\td0=%d\td1=%x\td2=%x\td3=%x\ta1=%x\n", op,
	       reg[1], reg[2], reg[3], aReg[1]);
#endif

	switch (op) {
	case 0:
		*reg = (*io_pend)(priv);
		break;

	case 1:
		res = (*io_read)(priv, &c, 1);
		if (res == 1)
			*((char *)reg + 4 + RBO) = c;
		else
			*reg = res ? res : QERR_EOF;
		break;

	case 2: /* read line */
		count = max(0, (uw16)reg[2] - (uw16)reg[1]);
		rc_count = (uw16)reg[1];
		qaddr = aReg[1];
		ioread(io_read, priv, qaddr, &count, true);
		//(uw16)reg[1]=count+rc_count;
		SETREG16(reg[1], count + rc_count);
		aReg[1] = qaddr + count;
		break;

	case 3: /* fetch string */
		qaddr = aReg[1];
		count = max(0, (uw16)reg[2] - (uw16)reg[1]);
		rc_count = (uw16)reg[1];
		ioread(io_read, priv, qaddr, &count, false);
		reg[1] = count + rc_count;
		aReg[1] = qaddr + count;
		break;

	case 5:
		res = (*io_write)(priv, (Ptr)reg + 4 + RBO, 1);
		if (res < 0)
			*reg = res;
		break;

	case 7: /* send string */
		count = (uw16)reg[2];
		res = (*io_write)(priv, (Ptr)memBase + aReg[1], count);
		if (res < 0) {
			count = 0;
			*reg = res;
		} else
			count = res;

		reg[1] = count;
		aReg[1] += count;
		break;

	case 0x48: /* read file into memory */
		qaddr = aReg[1];
		count = reg[2];
		rc_count = reg[1];
		ioread(io_read, priv, qaddr, &count, false);
		aReg[1] = qaddr + count;
		break;

	case 0x49:
		count = reg[2];
		res = (*io_write)(priv, (Ptr)memBase + aReg[1], count);
		if (res < 0) {
			count = 0;
			*reg = res;
		} else
			count = res;

		aReg[1] += count;
		break;

	default:
		*reg = QERR_BP;
		break;
	}
#ifdef IOTEST
	printf("ret from io_handle \td0=%d\td1=%x\t\ta1=%x\n", reg[0], reg[1],
	       aReg[1]);
#endif
}

/* returns (where possible) appropriate QDOS error based on errno value*/
int qmaperr(void)
{
	switch (errno) {
	case EPERM:
		return QERR_RO;
	case EBUSY:
		return QERR_IU;
	case ENOENT:
		return QERR_NF;
	case ESRCH:
		return QERR_BJ;
	case EINTR:
		return QERR_NC;
#if 0
    case EWOULDBLOCK : return QERR_NC;
#endif
	case EAGAIN:
		return QERR_NC;
	case EBADF:
		return QERR_RO;
	case ENOMEM:
		return QERR_OM;
	case EACCES:
		return QERR_RO;
	case EEXIST:
		return QERR_EX;
	case EINVAL:
		return QERR_BP;
	case ESPIPE:
		return QERR_BP;
	case EROFS:
		return QERR_RO;

	default:
		perror("warning : unknown error");
		return QERR_NI;
	}
}

/*********************************************************/
/* here comes a simple driver example*/

struct PRT_PRIV {
	FILE *file;
	int tra;
};

int prt_pend(void *priv)
{
	return QERR_EF; /* never anything pending */
}

int prt_read(void *priv, void *buf, int len)
{
	return QERR_EF;
}

int prt_write(void *priv, void *buf, int len)
{
	struct PRT_PRIV *p = (struct PRT_PRIV *)priv;
	int res, i, sig, nlen;
	char conv[4];

	if (len == 0)
		return 0;

	if (p->tra) {
		i = 0;
		sig = 0;

		while (len-- > 0 && !sig) {
			nlen = tra_conv(conv, buf + i, 1);
		restart:
			res = write(fileno(p->file), conv, nlen);
			if (res < nlen) {
				sig = 1;
				if (res > 0)
					nlen -= res;
				if (res > 0 || errno == EAGAIN ||
				    errno == EINTR)
					goto restart;
				else {
					if (!i)
						return qmaperr();
					else
						return i;
				}
			}
			/* successfull write, phew..:*/
			i++;
		}
		return i;
	} else {
		res = write(fileno(p->file), buf, len);
	}

	if (res < 0)
		return qmaperr();
	else
		return res;
}

int prt_init(int idx, void *d)
{
	return 0;
}

open_arg prt_par[4];

int prt_test(int id, char *name)
{
	return decode_name(name, Drivers[id].namep, prt_par);
	/*return RW(name)==strlen("PRT") && !strncasecmp("PRT",name+2,RW(name));*/
}

int prt_open(int id, void **priv)
{
	FILE *f;
	struct PRT_PRIV *p;
	const char *prt_string = emulatorOptionString("print");

	*priv = p = malloc(sizeof(struct PRT_PRIV));
	if (*priv == NULL)
		return -1;

	if (prt_par[2].i || prt_par[3].i) {
		char *p;
		int len;
		if (prt_par[3].i) {
			len = (prt_par[2].s ? strlen(prt_par[2].s) : 0) +
				  strlen(prt_par[3].s) + 2;
			p = (void *)malloc(len);
			strncpy(p, prt_par[3].s, len);
		} else {
			len = strlen(prt_string) + strlen(prt_par[2].s) + 2;
			p = (void *)malloc(len);
			strncpy(p, prt_string, len);
		}
		strncat(p, " ", len);
		if (prt_par[2].s)
			strncat(p, prt_par[2].s, len); /* add options etc */
		printf("executing command �%s�\n", p);
		f = popen(p, "w");
		if (!f)
			return qmaperr();
		printf("executing command �%s�\n", p);
		free(p);
	} else /* simple case */
	{
		f = popen(prt_string, "w");
		if (!f)
			return qmaperr();
	}

	p->file = f;
	p->tra = prt_par[1].i;

	return 0;
}

void prt_close(int id, void *priv)
{
	struct PRT_PRIV *p = priv;

	pclose(p->file);
	free(p);
}

void prt_io(int id, void *priv)
{
	io_handle(prt_read, prt_write, prt_pend, priv);
}

#ifdef TEST
open_arg bg_par[7];

void bg_init(int idx, void *d)
{
	printf("init test driver\n");
}
int bg_open(int ix, void **priv)
{
	printf("test channel opened:\n");
	printf("\t parsed values: %d,%c,%d,%d,%d %s\t%s\n", bg_par[0].i,
	       bg_par[1].i > 31 ? bg_par[1].i : '.', bg_par[2].i, bg_par[3].i,
	       bg_par[4].i, bg_par[5].s, bg_par[6].s);

	return 0;
}
int bg_test(int id, char *name)
{
	return decode_name(name, Drivers[id].namep, &bg_par);
}
void bg_close(int ix, void *prib)
{
}
void bg_io(int ix, void *priv)
{
	*reg = QERR_BP;
}
#endif
