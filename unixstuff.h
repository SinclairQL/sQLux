#pragma once
#include <stdint.h>

extern int QLdone;
extern char *homedir;
extern char *ux_bname;
extern int ux_boot;
extern int speed;

void cleanup(int err);
void set_rtc_emu(void);
void SetHome(void);
void uqlxInit(void);
int QLRun(void *data);
long ql2uxtime(long t);
long ux2qltime(long t);
int qm_fork(void (*cleanup)(), unsigned long id);
void prep_rtc_emu(void);
void GetDateTime(int32_t *t);
void DbgInfo(void);
void dosignal(void);
int32_t ReadQlClock(void);
uint32_t sysvar_l(uint32_t a);
void init_uqlx_tz(void);
