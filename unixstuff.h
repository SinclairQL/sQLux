#ifndef UNIXSTUFF_H
#define UNIXSTUFF_H

extern int QLdone;
extern char *homedir;

void cleanup(int err);
void set_rtc_emu(void);
void SetParams(int ac, char **av);
void uqlxInit(void);
void QLRun(void);
long ql2uxtime(long t);
long ux2qltime(long t);
void ChangedMemory(int from, int to);
int qm_fork(void (*cleanup)(), unsigned long id);
void prep_rtc_emu(void);
void GetDateTime(w32 *t);
void DbgInfo(void);
void dosignal(void);
w32 ReadQlClock(void);
uw32 sysvar_l(uw32 a);

#endif
