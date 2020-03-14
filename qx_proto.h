/*
 * (c) UQLX - see COPYRIGHT
 */

/*
 * all sorts of prototype declarations to avoid the ugliest compiler warnings
 */

#include "QSerial.h"
#include "QFilesPriv.h"

extern void InitialSetup(void);
extern void init_bas_exts(void);
void init_iso(void);
void QMParams (void);
extern void SchedInit(void);
extern int init_xscreen(void);
extern void init_poll(void);
int qm_fork(void (*cleanup)(), unsigned long id);


extern void GetDateTime(w32 *);


extern void XPatchPTRENV(void);
int c68err(int err);

extern void vm_init(void);
extern void vm_off(void);
extern void vm_on(void);
void vm_setscreen(void);
extern void vmMarkScreen(uw32 );
extern void set_rtc_emu(void);
extern void prep_rtc_emu(void);

uw16 find_ocode(uw32 addr);
void QLtrap(int t,int id,int nMax);
void QLvector(int which, int nMax);
void QLsubr(uw32 ea, int nMax);
int testMinerva(void);
void MReadKbd(void);
extern short LoadMainRom(void);

void InitDrivers(void);
void InitFileDrivers (void);
int eretry(void);
extern void on_fat_int(int x);

void add_patch_data(uw32 addr);
void add_patch(uw32 addr,uw16 ocode);
void restart_emulator(void);
void instrumentCode(void);

extern void uqlx_protect(unsigned long  start, unsigned long len, int type);
void uqlx_prestore(unsigned long start, unsigned long len);

void queueKey(short m,short code,uw16 asciiChar);
void insert_keyQ(int len,char *text);

void ResetMousePtr(void);
void InvisibleMousePtr(void);
void save_regs(void *p);
void restore_regs(void *p);

void process_events(void);

void ExceptionProcessing(void);
long ux2qltime(long t);
int qmaperr(void);
long ql2uxtime(long t);
size_t x_read(int fildes, void *buf, size_t byt);

void xso_q2x(int level,int optname, void* optval,int len);
void xso_x2q(int level,int optname, void* optval,int len);

int ser_read(serdev_t *, void *, int );
int ser_write(serdev_t *, void *, int );
void BlockMoveData(void *, void *,long );
int FSClose(int );
void RewriteHeader(void);
OSErr KillFileTail(FileNum , int );	
void TestCloseDevs(void);
void ChangedMemory(int , int );
void CleanRAMDev(char *dev);

int tra_conv(char *dest, char *src,int len);
int iso2ql_mem(unsigned char * buf,int len);
int ql2iso(int c);
int ql2iso_mem(unsigned char * buf,int len);

void CustomErrorAlert(char *);
void ErrorAlert(int);
void debug(char *);
void debug2(char*,long);
w32 ReadQlClock(void);

void rts(void);
void cleanup(int err);

void destroy_image (void);
void x_screen_open (int frk);
void x_screen_close (void);
void x_reset_state(void);
void redraw_screen(int ,int ,int ,int );
void conv_chunk(w32 from, w32 to);
void scrconv(void);
void parse_screen(char *x);
int Xsim(int val);

void init_keymap(void);
void vmtest(void);
void SetParams(int ac, char **av);
void uqlxInit(void);

void QLRun(void);
void ExecuteChunk(long n);

void init_timers(void);
void fork_files(void);

void debugIPC(char *msg, long n);
void cleanup_ipc(void);
void process_ipc(void);
int init_ipc(void);

void SetTable(void (**itable)(void),
	      char *s,void (*f)(void));
void SetInvalEntries(void (**itable)(void),void *code);

void dosignal(void);
int do_fork(void);
int InitDialog();
void oncc(int sig);
int allow_rom_break(int flag);
int toggle_hog(int val);

extern char *qm_findx(char *name);

extern int display_mode;

/* function prototypes missing in various OS's */

#ifdef NEED_STRNCASECMP
int strncasecmp(char *s1,char *s2,int len);
#endif
