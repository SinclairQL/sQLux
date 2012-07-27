/* UQLX */

#ifndef QSERIAL_H
#define QSERIAL_H

#ifdef SERIAL
#include <sys/types.h>

Cond SetBaudRate(short);
void InstallSerial(void);
void InitSerial(void);
void CloseSerial(void);

typedef struct _FakeRec *FakeTerm;
typedef struct _FakeRec 
{
     char *command;			/* command name			*/
     char **command_args;		/* command arguments		*/
     char *term_type;			/* terminal type		*/
     char utmp_inhibit;         	/* don't add utmp entries	*/
     char login_shell;		        /* invoke shell as login shell	*/
     char *tname;			/* name of allocated PTY	*/
     pid_t pid;				/* process ID of child process	*/
     uid_t uid;				/* uid of invoking user		*/
     gid_t gid;				/* gid of invoking user		*/
     int master;			/* pty master file descriptor	*/
     int slave;				/* pty slave file descriptor	*/
  struct SERDEV *sd;
  FakeTerm next;
  int job_control;
} FakeRec;

typedef struct SERDEV
{
  long unit;
  long parity;
  long hshake;
  long xlate;
  long baud;
  int fd;
#ifdef NO_FIONREAD
  int bfc_valid;
  char bfc;
#endif
  /* some pty special values */
  FakeTerm w;
  int killed;
  int teof;    /* chr$(26) occured and should cause 1 EOF*/
} serdev_t;


void tty_baud(serdev_t *);
int tty_open(char *, serdev_t *);
void tty_close(int );

int pty_open(int, void **);
void pty_close(int, void *);

int pendingio (int f);
int writeio (serdev_t * sd, char *buf, long *pno);
int readio (serdev_t * sd, char *buf, long *pno, short tc);

#define MAXSERIAL 4

#endif
#endif
