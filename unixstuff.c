/*
 * (c) UQLX - see COPYRIGHT
 */

/************************************************************************************************/
/* Modification history                                                                         */
/* 0712 TF Adapted to linux ARM                                                                 */
/*      Removed all references to EMX                                                           */
/*      Signal handling aligned between Linux arm and Linux x86                                 */
/*      Major code cleanup                                                                      */
/*      NOTE: Not tested on Linux/68k                                                           */
/************************************************************************************************/

#ifdef __WIN32__
#define __USE_MINGW_ALARM
#include <windows.h>
#include <shlobj.h>
#endif

#include "QL68000.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include <inttypes.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "debug.h"
#include "xcodes.h"
#include "QL_config.h"
#include "QInstAddr.h"

#include "unix.h"
#include "boot.h"
#include "dummies.h"
#include "iexl_general.h"
#include "QDisk.h"
#include "QL_cconv.h"
#include "QL_files.h"
#include "QL_sound.h"
#include "uxfile.h"
#include "uqlx_cfg.h"
#include "QL_screen.h"
#include "SDL2screen.h"
#include "version.h"
#include "Xscreen.h"

#define TIME_DIFF 283996800
void GetDateTime(w32 *);
long long qlClock = 0;

#ifdef VTIME
int qlttc = 50;
int qltime = 0;
int qitc = 10;
#endif

int ux_boot = 0;
int ux_bfd = 0;
char *ux_bname = "";
int start_iconic = 0;
char *pwindow = NULL;
int UQLX_optind;
int UQLX_argc;
char **UQLX_argv;

char *homedir = "";

extern uw32 rtop_hard;
extern int screen_drawable;

int do_update = 0; /* initial delay for screen drawing */

#ifndef min
#define min(_a_, _b_) (_a_ < _b_ ? _a_ : _b_)
#endif

#ifndef max
#define max(_a_, _b_) (_a_ > _b_ ? _a_ : _b_)
#endif

int QLdone = 0;

extern void FlushDisplay(void);

extern void DbgInfo(void);

void btrap3(void);

extern void SchedulerCmd(void);
extern void KbdCmd(void);

#ifndef XAW
extern void process_events(void);
#endif

int script = 0;
int redir_std = 0;

int scrcnt = 0;
//volatile int doPoll = 0;

#ifdef VTIME
volatile poll_req = 0; /* debug only */
#endif

int noints = 0;
int schedCount = 0;
extern int min_idle;
int HasDialog;

long pagesize = 4096;
int pageshift = 12;

void cleanup_dialog()
{
}

int rtc_emu_on = 0;
void prep_rtc_emu()
{
}

void set_rtc_emu()
{
}

/* Read a system variable word sized from QDOS memory    */
/* NOTE(TF): Apparently not used anywhere in the code!   */
uw16 sysvar_w(uw32 a)
{
	return RW((Ptr)memBase + 0x28000 + a);
}

/* Read a system variable (long) from QDOS memory        */
uw32 sysvar_l(uw32 a)
{
	return RL((Ptr)memBase + 0x28000 + a);
}

#ifdef SHOWINTS
static long alrm_count = 0;
static long a_ticks = 0;
static long aa_cnt = 0;
#endif

static int flptest = 0;

void dosignal()
{
	SDL_AtomicSet(&doPoll, 0);

#ifndef XAW
	if (!script && !QLdone)
		//process_events ();
		QLSDLProcessEvents();
#endif

#ifdef SHOWINTS
	aa_cnt += alrm_count;
	alrm_count = 0;
	if (++a_ticks > 49) {
		printf("received %d alarm signals, processed %d\n", aa_cnt,
		       a_ticks);
		a_ticks = aa_cnt = 0;
	}
#endif

	if (--scrcnt < 0) {
		set_rtc_emu();
	}

	if (flptest++ > 25) {
		flptest = 0;
		TestCloseDevs();
	}

#ifndef xx_VTIME
	FrameInt();
#endif
}

extern int xbreak;
void cleanup(int err)
{
	CleanRAMDev("RAM");
	QLSDLExit();
	exit(err);
}

void oncc(int sig)
{
	printf("exiting\n");
	QLdone = 1;
}

void signalTimer()
{
	//doPoll = 1;
#ifdef VTIME
	poll_req++;
#endif
	schedCount = 0;
#ifdef SHOWINTS
	alrm_count++;
#endif
}

void ontsignal(int sig)
{
	/*set_rtc_emu();*/ /* .. not yet working */
	//signalTimer ();
}

/* rather crude but reliable */
static int fat_int_called = 0;

void on_fat_int(int x)
{
	if (fat_int_called == 1)
		exit(45);
	if (fat_int_called > 1)
		raise(9);
	fat_int_called++;

	alarm(0);
	printf("Terminate on signal %d\n", x);
	printf("This may be due to an internal error,\n"
	       "a feature not emulated or an 68000 exception\n"
	       "that typically occurs only when QDOS is unrecoverably\n"
	       "out of control\n");
	dbginfo("FATAL error, PC may not be displayed correctly\n");
	cleanup(44);
}

void InitDialogErr(int x)
{
	HasDialog = -1;
	/* init_timers (); */
}

#ifdef UX_WAIT
#include <sys/wait.h>
struct cleanup_entry {
	void (*cleanup)();
	unsigned long int id;
	pid_t pid;
	struct cleanup_entry *next;
};

static struct cleanup_entry *cleanup_list = NULL;
static int run_reaper;

static int qm_wait(fc) int *fc;
{
	int pid;

	pid = wait3(fc, WNOHANG, (struct rusage *)NULL);
	return pid;
}

/* exactly like fork but registers cleanup handler */
int qm_fork(void (*cleanup)(), unsigned long id)
{
	struct cleanup_entry *ce;
	int pid;

	pid = fork();
	if (pid > 0) {
		ce = (void *)malloc(sizeof(struct cleanup_entry));
		ce->pid = pid;
		ce->id = id;
		ce->cleanup = cleanup;
		ce->next = cleanup_list;
		cleanup_list = ce;
	}
	return pid;
}

static void qm_reaper()
{
	struct cleanup_entry *ce, **last;
	int pid, found;
	int failcode;

	run_reaper = 0;
	while ((pid = qm_wait(&failcode)) > 0) {
		ce = cleanup_list;
		last = &cleanup_list;
		found = 0;
		while (ce) {
			if (pid == ce->pid) {
				*last = ce->next;
				(*(ce->cleanup))(ce->pid, ce->id, failcode);
				free(ce);
				found = 1;
				break;
			}
			last = &(ce->next);
			ce = ce->next;
		}
		if (!found)
			printf("hm, pid %d not found in cleanup list?\n", pid);
	}
}
#endif

int load_rom(char *, w32);

void ChangedMemory(int from, int to)
{
	int i;
	uw32 dto, dfrom;

	/* QL screen memory involved? */
	if ((from >= qlscreen.qm_lo && from <= qlscreen.qm_hi) ||
	    (to >= qlscreen.qm_lo && to <= qlscreen.qm_hi)) {
		    QLSDLUpdatePixelBuffer();
	}
}

char **argv;

void DbgInfo(void)
{
	int i;

	/* "ssp" is ssp *before* sv-mode was entered (if active now) */
	/* USP is saved value of a7 or meaningless if not in sv-mode */
	printf("DebugInfo: PC=%" PRIXPTR
	       ", code=%x, SupervisorMode: %s USP=%x SSp=%x A7=%x\n",
	       (Ptr)pc - (Ptr)memBase, code, (supervisor ? "yes" : "no"), usp,
	       ssp, *m68k_sp);
	printf("Register Dump:\t Dn\t\tAn\n");
	for (i = 0; i < 8; i++)
		printf("%d\t\t%8x\t%8x\n", i, reg[i], aReg[i]);
}

long uqlx_tz;

long ux2qltime(long t)
{
	return t + TIME_DIFF + uqlx_tz;
}

long ql2uxtime(long t)
{
	return t - TIME_DIFF - uqlx_tz;
}

void GetDateTime(w32 *t)
{
	struct timeval tp;

#ifndef VTIME

	gettimeofday(&tp, (void *)0);
	*t = ux2qltime(tp.tv_sec) + qlClock;
	;
#else
	*t = qltime;
#endif
}

int rombreak = 0;

int allow_rom_break(int flag)
{
	if (flag < 0)
		return rombreak;

	if (flag) {
		rombreak = 1;
	} else {
		rombreak = 0;
	}
	return rombreak;
}

void init_uqlx_tz()
{
	struct tm ltime;
	struct tm gtime;
	time_t ut;

	ut = time(NULL);
	ltime = *localtime(&ut);
	gtime = *gmtime(&ut);

	gtime.tm_isdst = ltime.tm_isdst;
	uqlx_tz = mktime(&ltime) - mktime(&gtime);
}

w32 ReadQlClock(void)
{
	w32 t;

	GetDateTime(&t);
	return t;
}

int impopen(char *name, int flg, int mode)
{
	char buff[PATH_MAX], *p;
	int r, md;

	md = mode;

	if ((r = open(name, flg, md)) != -1)
		return r;

	if (*name == '~') {
		char *p = buff;
		strcpy(p, homedir);
		strcat(p, name + 1);
		name = p;
	}

	return open(name, flg, md);

	//strcpy(buff,IMPL);
	p = buff + strlen(buff);
	if (*(p - 1) != '/')
		strcat(buff, "/");
	strncat(buff, name, PATH_MAX);

	return open(buff, flg, md);
}

int load_rom(char *name, w32 addr)
{
	struct stat b;
	int r;
	int fd;

	fd = impopen(name, O_RDONLY | O_BINARY, 0);
	if (fd < 0) {
		perror("Warning: could not find ROM image ");
		printf(" - rom name %s\n", name);
		return 0;
	}

	fstat(fd, &b);
	if (b.st_size != 16384 && addr != 0)
		printf("Warning: ROM size of 16K expected, %s is %d\n", name,
		       (int)b.st_size);
	if (addr & 16383)
		printf("Warning: addr %x for ROM %s not multiple of 16K\n",
		       addr, name);

	r = read(fd, (Ptr)memBase + addr, b.st_size);
	if (r < 0) {
		perror("Warning, could not load ROM \n");
		printf("name %s, addr %x, QDOS origin %p\n", name, addr,
		       memBase);
		return 0;
	}
	if (V3)
		printf("loaded %s \t\tat %x\n", name, addr);
	close(fd);

	return r;
}

int scr_planes = 2;
int scr_width, scr_height;

int verbose = 2;

#ifndef XAW
extern int shmflag;
#endif

int sct_size;
//char *scrModTable,
char *oldscr;

static char obuf[BUFSIZ];

void CoreDump()
{
	int fd, r;

	fd = open("qlcore", O_RDWR | O_CREAT, 0644);
	if (fd < 0)
		perror("coredump failed: read: :");
	if (fd > -1) {
		r = write(fd, memBase, 1024 * 1024);
		if (!r)
			perror("coredump failed: write: ");
		close(fd);
		if (r)
			printf("memory dump saved as qlcore\n");
	}
}

#include "uqlx_cfg.h"

char *qm_findx(char *name)
{
	char *loc;
	static char buf[PATH_MAX + 40];

	strncpy(buf, homedir, PATH_MAX);
	qaddpath(buf, "lib/uqlx", PATH_MAX);
	if (!access(buf, R_OK | X_OK))
		loc = buf;
	else
		loc = NULL;

	//if (!loc && !access(IMPL, R_OK | X_OK))
	//   loc = IMPL;
	if (!loc && !access("/usr/local/lib/uqlx", R_OK | X_OK))
		loc = "/usr/local/lib/uqlx/";
	if (!loc && !access("/usr/lib/uqlx", R_OK | X_OK))
		loc = "/usr/lib/uqlx/";
	if (!loc && !access("/usr/local/uqlx/lib/", R_OK | X_OK))
		loc = "/usr/local/uqlx/lib/";

	return loc;
}

void browse_manuals()
{
	int ret;
	char buf[PATH_MAX + 25];
	char *loc;

	loc = qm_findx("browse_manual");

	strncpy(buf, loc, PATH_MAX);
	qaddpath(buf, "browse_manual ", PATH_MAX);
	//strncat(buf,IMPL,PATH_MAX);
	//printf("executing %s\n",buf);
	ret = system(buf);
	if (ret == -1)
		printf("Failed to execute browser\n");

	exit(0);
}

void vmtest()
{
#if defined(LINUX) && defined(m68k)
	int res;

	char buf[PATH_MAX + 1];
	char *loc;

	loc = qm_findx("vmtest");

	strncpy(buf, loc, PATH_MAX);
	qaddpath(buf, "vmtest", PATH_MAX);

	res = system(buf);
	if (res == 1) {
		printf("vmtest failed, for m68k get latest kernel patch from"
		       "rz@linux-m68k.org");
		exit(44);
	}
	if (res) {
		printf("problem executing %s\n", buf);
		exit(1);
	}
#endif
}

int toggle_hog(int val)
{
	/*printf("toggle_hog, setting to %d\n",val);*/
	if (val < 0)
		return QMD.cpu_hog;
	QMD.cpu_hog = val;
	//if(QMD.cpu_hog)
	//   min_idle = 20000;
	//else
	//   min_idle = 5;
	return QMD.cpu_hog;
}

void usage(char **argv)
{
	printf("UQLX release %s:\n\tusage: %s [options] arguments\n", release,
	       argv[0]);
	printf("\t options:\n"
	       "\t\t -?                  : list options \n"
	       "\t\t -f config_file      : read alternative config file\n"
	       "\t\t -r MEM              : set RAMTOP to this many Kb \n"
	       "\t\t -o romname          : use this ROM \n"
	       "\t\t -b 'boot_cmd'       : define BOOT device with boot_cmd \n"
	       "\t\t -d 'boot_dev'       : device where BOOT should be read eg. 'mdv1'\n"
	       "\t\t -g NxM              : define screen size to NxM \n"
	       "\t\t -v num              : set verbosity\n"
	       "\t\t -w 1x,2x,3x,max,full: window size 1x, 2x, 3x, maximize, fullscreen \n"
	       "\t\t -n                  : dont patch anything (xuse only)\n\n"
	       "\t arguments: available through Superbasic commands\n\n");
	exit(0);
}

void SetParams(int ac, char **av)
{
	char sysrom[200];
	int c;
	int mem = -1, hog = -1, no_patch = -1;
	int gg = 0;
	char *home;
	int res;

#ifdef __WIN32__
	struct stat stat_res;
	char my_documents[MAX_PATH + 1];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL,
					 SHGFP_TYPE_CURRENT, my_documents);

	if (result == S_OK) {
		strncat(my_documents, "\\sQLux", MAX_PATH);
		if (stat(my_documents, &stat_res) < 0) {
			res = mkdir(my_documents);
			if (res < 0) {
				perror("Error");
				printf("Creating Home %s", my_documents);
			} else {
				homedir = strdup(my_documents);
			}
		} else {
			homedir = strdup(my_documents);
		}
	}
#else
	home = getenv("HOME");
	if (home)
		homedir = strdup(home);
#endif

	setvbuf(stdout, obuf, _IOLBF, BUFSIZ);

	*sysrom = 0;

	while((c = getopt(ac, av, "c:f:r:o:b:d:g:v:w:n?")) != EOF) {
		switch (c) {
		case 'f':
			strncpy(QMD.config_file, optarg, PATH_MAX);
			QMD.config_file_opt = 1;
			break;
		}
	}

	QMParams();

	if (strcmp(QMD.resolution, "512x256")) {
		parse_screen(QMD.resolution);
	}

	/* reset the option parsing for second pass */
#ifdef __APPLE__
	optreset = 1;
#endif
	optind = 1;

#ifndef NO_GETOPT
	while ((c = getopt(ac, av, "c:f:r:o:b:d:g:v:w:n?")) != EOF) {
		switch (c) {
		case 'g':
			gg = 0;
			parse_screen(optarg);
			break;
		case 'f':
			break;
		case 'o':
			strcpy(sysrom, optarg);
			break;
		case 'r':
			mem = atoi(optarg);
			break;
		case 'h':
			hog = 1;
			break;
		case 'n':
			no_patch = 1;
			break;
		case 'v':
			verbose = atoi(optarg);
			break;
		case 'b': {
			int len;

			if (optarg && strcmp("", optarg)) {
				ux_boot = 2;
				len = strlen(optarg);
				ux_bname = malloc(len + 2);
				strncpy(ux_bname, optarg, len);
				ux_bname[len] = 10;
				ux_bname[len + 1] = 0;
			}
		} break;
		case 'w':
			if (optarg && (strlen(optarg) < 5)) {
				int valid = 1;
				valid &= strcmp("1x", optarg) ? 1 : 0;
				valid &= strcmp("2x", optarg) ? 1 : 0;
				valid &= strcmp("3x", optarg) ? 1 : 0;
				valid &= strcmp("max", optarg) ? 1 : 0;
				valid &= strcmp("full", optarg) ? 1 : 0;

				if (!valid) {
					strcpy(QMD.winsize, optarg);
				} else {
					usage(av);
				}
			} else {
				usage(av);
			}
			break;
		case '?':
			usage(av);
			break;
		case 'd':
			if (optarg && strcmp("", optarg)) {
				if (strlen(optarg) != 4) {
					printf("-d BOOT_DEV must be 4 chars\n");
				} else {
					strncpy(QMD.bootdev, optarg, 4);
				}
			}
			break;
		case 'c':
			res = QMParseParam(optarg);
			if (res < 0)
				printf("Invalid arg %s\n", optarg);
			break;
		default:
			usage(av);
		}
	}

	UQLX_argc = ac;
	UQLX_argv = av;
	UQLX_optind = optind;

#else
	UQLX_argc = ac;
	UQLX_argv = av;
	UQLX_optind = 1;
#endif

	if (mem > 0 && mem < 17)
		mem = mem * 1024;

	if (mem != -1)
		QMD.ramtop = mem;
	if (hog != -1)
		QMD.cpu_hog = 1;
	if (no_patch != -1)
		QMD.no_patch = 1;

	if (QMD.no_patch)
		do_update = 1;
	//printf("do_update: %d\n",do_update);

	toggle_hog(QMD.cpu_hog);

	RTOP = QMD.ramtop * 1024;
}

#ifdef G_reg
w32 _reg[16];
#endif

#define MAX_DISKS 2
typedef struct BlockDriverState BlockDriverState;
BlockDriverState *bs_table[MAX_DISKS];
#ifdef DARWIN
const char *hd_filename[MAX_DISKS] = {};
#else
const char *hd_filename[MAX_DISKS] = { "/home/rz/.qldir/IMG1",
				       "/home/rz/.qldir/IMG2" };
#endif

void init_xhw()
{
}

#if 0
void uqlxInit()
{
	char *rf;
	int rl = 0;
	void *tbuff;

	if (V1)
		printf("*** sQLux release %s\n\n", release);
	tzset();

	memBase = malloc(RTOP);
	if (memBase == NULL) {
		printf("sorry, not enough memory for a %dK QL\n", RTOP / 1024);
		exit(1);
	}

	if (EmulatorTable()) {
		printf("Failed to allocate instruction table\n");
		free(memBase);
		exit(1);
	}

	{
		char roms[PATH_MAX + 1];
		char *p = NULL;
		int romd_len;

		if ((rf = getenv("QL_ROM")))
			rl = load_rom(rf, 0);
		if (!rl) {
			strncpy(roms, QMD.romdir, PATH_MAX);
			romd_len = strlen(roms);
			p = (char *)roms + romd_len;

			if (*(p - 1) != '/') {
				*p++ = '/';
			}
			strncpy(p, QMD.sysrom, PATH_MAX - romd_len);

			rl = load_rom(roms, (w32)0);
			if (!rl) {
				fprintf(stderr,
					"Could not find qdos ROM image, exiting\n");
				exit(2);
			}
		}
		if (strlen(QMD.romim)) {
			strncpy(roms, QMD.romdir, PATH_MAX);
			romd_len = strlen(roms);
			p = (char *)roms + romd_len;

			if (*(p - 1) != '/') {
				*p++ = '/';
			}
			strncpy(p, QMD.romim, PATH_MAX - romd_len);

			rl = load_rom(roms, 0xC000);
			if (!rl) {
				fprintf(stderr,
					"Could not find expansion rom, exiting\n");
				exit(2);
			}
		}
	}

	init_uqlx_tz();

	init_signals();

	init_iso();

	init_xhw(); /* init extra HW */
	LoadMainRom(); /* patch QDOS ROM*/

	/* Minerva cannot handle more than 16M of memory... */
	if (isMinerva && RTOP > 16384 * 1024)
		RTOP = 16384 * 1024;
	/* ...everything else not more than 4M */
	if (!isMinerva && RTOP > 4096 * 1024)
		RTOP = 4096 * 1024;

	rtop_hard = RTOP;

	if (isMinerva) {
		qlscreen.xres = qlscreen.xres & (~(7));
		qlscreen.linel = qlscreen.xres / 4;
		qlscreen.qm_len = qlscreen.linel * qlscreen.yres;

		qlscreen.qm_lo = 128 * 1024;
		qlscreen.qm_hi = 128 * 1024 + qlscreen.qm_len;
		if (qlscreen.qm_len > 0x8000) {
			if (((long)RTOP - qlscreen.qm_len) <
			    256 * 1024 + 8192) {
				/*RTOP+=qlscreen.qm_len;*/
				printf("sorry, not enough RAM for such a big screen\n");
				goto bsfb;
			}
			qlscreen.qm_lo = ((RTOP - qlscreen.qm_len) >> 15)
					 << 15; /* RTOP MUST BE 32K aligned.. */
			qlscreen.qm_hi = qlscreen.qm_lo + qlscreen.qm_len;
			RTOP = qlscreen.qm_lo;
		}
	} else /* JS doesn't handle big screen */
	{
	bsfb:
		qlscreen.linel = 128;
		qlscreen.yres = 256;
		qlscreen.xres = 512;

		qlscreen.qm_lo = 128 * 1024;
		qlscreen.qm_hi = 128 * 1024 + 32 * 1024;
		qlscreen.qm_len = 0x8000;
	}

#ifndef XAW
	//if (!script)
	//   x_screen_open(0);
#endif
	scr_width = qlscreen.xres;
	scr_height = qlscreen.yres;

	if (!script)
		QLSDLScreen();

	if (V1 && (QMD.speed > 0)) printf("Speed %.1f\n",QMD.speed);

	sound_enabled = (QMD.sound > 0);
	if (V1 && (QMD.sound > 0))
		printf("sound enabled, volume %i.\n",QMD.sound);

#ifdef SOUND
	if ((!script) && sound_enabled)
		sound_enabled = initSound(QMD.sound);
#endif

#ifdef TRACE
	TraceInit();
#endif
#ifdef G_reg
	reg = _reg;
#endif
	if (!isMinerva) {
		qlux_table[IPC_CMD_CODE] = UseIPC; /* install pseudoops */
		qlux_table[IPCR_CMD_CODE] = ReadIPC;
		qlux_table[IPCW_CMD_CODE] = WriteIPC;
		qlux_table[KEYTRANS_CMD_CODE] = QL_KeyTrans;

		qlux_table[FSTART_CMD_CODE] = FastStartup;
	}
	qlux_table[ROMINIT_CMD_CODE] = InitROM;
	qlux_table[MDVIO_CMD_CODE] = MdvIO;
	qlux_table[MDVO_CMD_CODE] = MdvOpen;
	qlux_table[MDVC_CMD_CODE] = MdvClose;
	qlux_table[MDVSL_CMD_CODE] = MdvSlaving;
	qlux_table[MDVFO_CMD_CODE] = MdvFormat;
	qlux_table[POLL_CMD_CODE] = PollCmd;

#ifdef SERIAL
#ifndef NEWSERIAL
	qlux_table[OSERIO_CMD_CODE] = SerIO;
	qlux_table[OSERO_CMD_CODE] = SerOpen;
	qlux_table[OSERC_CMD_CODE] = SerClose;
#endif
#endif

	qlux_table[SCHEDULER_CMD_CODE] = SchedulerCmd;
	if (isMinerva) {
		qlux_table[MIPC_CMD_CODE] = KbdCmd;
		qlux_table[KBENC_CMD_CODE] = KBencCmd;
	}
	qlux_table[BASEXT_CMD_CODE] = BASEXTCmd;

	if (QMD.skip_boot)
		qlux_table[0x4e43] = btrap3;

	g_reg = reg;

	InitialSetup();

	if (isMinerva) {
		reg[1] = (RTOP & ~16383) | 1;
		SetPC(0x186);
	}

	QLdone = 0;
}

#endif

#ifndef XAW
void QLRun(void)
#else
Cond CPUWork(void)
#endif
{
	int scrchange, i;
	int loop = 0;

	int speed = (int)(QMD.speed * 20);
	speed = (speed >= 0) && (sem50Hz != NULL) ? speed : 0;

#ifndef XAW
exec:
#endif
	if (!speed) {
		ExecuteChunk(3000);
	}
	else {
		ExecuteChunk(300);
		++loop;

		if (loop >= speed) {
			SDL_SemWait(sem50Hz);
			loop = 0;
		}
	}

#ifdef UX_WAIT
	if (run_reaper)
		qm_reaper();
#endif

#ifdef VTIME
	if ((qlttc--) <= 0) {
		qlttc = 3750;
		qltime++;
	}
	if ((qitc--) < 0) {
		qitc = 3;
		doPoll = 1;
		/*FrameInt();*/
		poll_req++;
		/*dosignal();*/
	}
#endif

#ifndef XAW
	if (!QLdone)
		goto exec;

	cleanup(0);
#else
	return 0;
#endif
}
