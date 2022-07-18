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
#include "SDL2main.hpp"

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
	SDL_Event event;

	CleanRAMDev("RAM");

	event.user.type = SDL_USEREVENT;
	event.user.code = USER_CODE_EMUEXIT;
	event.user.data1 = NULL;
	event.user.data2 = NULL;

	event.type = SDL_USEREVENT;

	SDL_PushEvent(&event);

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

int verbose = 2;
static char obuf[BUFSIZ];

int toggle_hog(int val)
{
	if (val < 0)
		return QMD.cpu_hog;
	QMD.cpu_hog = val;

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

int QLRun(void *data)
{
	int scrchange, i;
	int loop = 0;

	int speed = (int)(QMD.speed * 20);
	speed = (speed >= 0) && (sem50Hz != NULL) ? speed : 0;

exec:
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

	if (!QLdone)
		goto exec;

	cleanup(0);

	return 0;
}
