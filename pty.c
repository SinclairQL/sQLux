/*
 * This code was shamelessly stolen from the Emu project, whose notice is
 * reproduced below.
 */

/*
 * This file is part of the Emu system.
 *
 * Copyright 1990 by PCS Computer Systeme, GmbH. Munich, West Germany.
 *
 * Copyright 1994 by Jordan K. Hubbard and Michael W. Elbel
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL PCS, THE AUTHORS, OR THEIR HOUSEPETS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. SO DON'T SUE US.
 * THANK YOU.
 */

#ifdef SERIAL

/*#include "QLtypes.h"*/
#include "QL68000.h"

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <fcntl.h>
#include <termios.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
//#include <sys/termios.h>

#ifdef HPUX
#include <bsdtty.h>
#endif

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#ifdef __linux__
#include <sys/cdefs.h>
#endif
#include <sys/wait.h>
#include <sys/time.h>	/* for timeval */
#include <stdarg.h>

#ifdef BSD44
#define BSD
#endif

#ifdef NEWPTY

#include "QL.h"
#include "driver.h"

#include "QSerial.h"
#endif

#include "QLserio.h"
#include "unixstuff.h"

#if !defined(NFILES)
#define NFILES	64	/* take a wild guess */
#endif

#ifdef _NO_PID_T
typedef int		pid_t;
#endif

#if defined(__GNUC__)
# ifdef _USE_STDARGS
#   define _VARARGS(start) start, ...
#   define _VA_DCL(fname, arg) fname(arg, ...)
# else
#   define _VARARGS(start) ...
#   define _VA_DCL(fname, arg) fname(va_alist) va_dcl
# endif
#else
#   define _VARARGS(start)
#   define _VA_DCL(fname, arg) fname(va_alist) va_dcl
#endif

/* predefined subprocess exit codes */
#define PROC_EXEC_FAILED	-4
#define PROC_TTYOPEN_FAILED	-5

#if !defined (SIGCHLD) && defined (SIGCLD)
#define SIGCHLD SIGCLD
#endif /* SIGCLD */

#if !defined(PTYDEV)
#define	PTYDEV		"/dev/pty%c%c"
#endif	/* !PTYDEV */

#if !defined(TTYDEV)
#define	TTYDEV		"/dev/tty%c%c"
#endif	/* !TTYDEV */

#if !defined(PTYCHAR1)
#define	PTYCHAR1	"pqrstuvwxyz"
#endif	/* !PTYCHAR1 */

#if !defined(PTYCHAR2)
#define	PTYCHAR2	"0123456789abcdef"
#endif	/* !PTYCHAR2 */


#define DEFAULTCOMMAND	"/bin/sh"
#define DEFAULTTERMTYPE	"vt100"

/* moved to QSerial.h */
#if 0
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
} FakeRec;
#endif
/*static FakeTerm w;*/

void fake_tty_close(FakeTerm);
FakeTerm fake_tty_open(char *);
void pty_cleanup(pid_t pid, int id);

#ifdef NEWPTY

open_arg pty_par[4];

int pty_init(int idx){return 0;}

int pty_test(int id, char *name)
{
    return decode_name(name, Drivers[id].namep, &pty_par[0]);
}

FakeTerm pty_list=NULL;

int pty_open(int id, void **priv)
{
	serdev_t *p;
	char *cmd;

	*priv=p=malloc(sizeof(serdev_t));

	p->parity=0;
	p->hshake=-1;
	p->xlate=pty_par[1].i;
	p->baud=115200;
#ifdef NO_FIONREAD
	p->bfc_valid=0;
#endif
	p->killed=0;
	p->w=NULL;
	p->teof=0;

	cmd=pty_par[2].s;
	if (!strcmp(cmd,""))
		cmd=NULL;

	p->w = fake_tty_open(cmd);
	if (p->w && (p->fd = p->w->master)>0)
	{
		/*link into list*/
		if (pty_list) {p->w->next=pty_list;pty_list=p->w;}
		else pty_list=p->w;
		p->w->sd=p;
		p->w->job_control=pty_par[0].i;
		return 0;
	}


	/* failure */
	free(p);
	return -1;
}

void pty_close(int id,void *priv)
{
  FakeTerm tp;
  char *fb;

  serdev_t *p=priv;

  /*printf("pty_close\n");*/

  ser_write(p,&fb,0);   /* attempt to generate a EOF/EOT */

  fake_tty_close(p->w);

  if (p->w->job_control==2)
    kill(p->w->pid, 2);

  if (pty_list==p->w)
    {
      pty_list=pty_list->next;
      return;
    }

  tp=pty_list;
  while(tp)
    {
      if (tp->next==p->w)
	{
	  tp->next=tp->next->next;
	  free(p->w);

	  return;
	}
      tp=tp->next;
    }

  /*printf("WARNING: closed pty not in list\n");*/
}

#endif


static void fork_process(FakeTerm w)
{
     int i, res;
     sigset_t mask,omask;

     /* Make sure we don't get the signal just yet - do it the BSD way,
	linux doesn't support sighold */
#if 0
     omask = sigblock(sigmask(SIGCHLD));
#else
     sigemptyset(&mask);
     sigaddset(&mask,SIGCHLD);
     sigprocmask(SIG_BLOCK,&mask,&omask);
#endif

     if (!(w->pid = qm_fork(pty_cleanup,0))) {
	  /* Now in child process */
	  char **argp;
	  int fd;

	  /* We're not particularly interested */
	  signal(SIGCHLD, SIG_DFL);

      	  if (setsid() < 0)
               puts ("failed to set process group");
#if 0
	  printf("execing prog: %x %s\n",w->command,w->command);
	  printf("args: %x %s\n",w->command_args);
	  for (i=0;w->command_args[i];i++)
	    printf("\t%s\n",w->command_args[i]);
#endif


#if 1
	  /* Close everything in sight */
	  for (i = 0; i < NFILES; i++)
	       close(i);
#else
	  close(0);
	  close(1);    /* leave stderr */
	  for (i = 3; i < NFILES; i++)
	    close(i);
#endif

	  /*
	   * Open the slave side for stdin and dup twice for stdout and stderr.
	   * We need to open all fd's read/write since some weird applications
	   * (like "more") READ from stdout/stderr or WRITE to stdin!
	   * Ack, bletch, barf.
	   *
	   * This should also take care of the controlling TTY. I hope.
	   */
          fd = open(w->tname, O_RDWR, 0);		/* 0 */
          res = dup(fd);				/* 1 */
          res = dup(fd);				/* 2 */
#if 0
	  fprintf(stderr,"opened tty: %s, result %d\n",w->tname,fd);
#endif

#if 0
	  /* set various signals to non annoying (for SYSV) values */
	  if (getpgrp() == getpid()) {
	       signal(SIGINT, exit);
	       signal(SIGQUIT, exit);
	       signal(SIGTERM, exit);
	  }
	  else {
	       signal(SIGINT, SIG_IGN);
	       signal(SIGQUIT, SIG_IGN);
	       signal(SIGTERM, SIG_IGN);
	  }
	  signal(SIGPIPE, exit);
#endif

	  /* since we may be setuid root (for utmp), be safe */
	  res = setuid(w->uid);
	  res = setgid(w->gid);

	  argp = w->command_args ;

	  if (w->login_shell)
	       argp[0] = "-";
	  else
	       argp[0] = w->command;
	  /*printf("*******\n");*/
	  execvp(w->command, argp);
	  /*
	   * This will show up in the client window (hopefully), so
	   * sleep for some time before exiting to let the user read
	   * the message
	   */
	  printf ("### pty error message: Couldn't exec %s\t ###\n", argp[0]);
	  sleep(5);
	  exit(PROC_EXEC_FAILED);
     }

     /*
      * Now that we have stored the widget ID, we can enable SIGCHLD
      * and deal with a possible early failure.
      */
     sigprocmask(SIG_SETMASK,&omask,NULL);
}

static void process_init(FakeTerm w)
{
     char envterm[1024], *tmp;

     w->uid = getuid();
     w->gid = getgid();

     snprintf(envterm, 1024, "TERM=%s", w->term_type);
     tmp = strdup (envterm);
     putenv(tmp);

     fork_process(w);
}

static void process_cleanup(FakeTerm w)
{
  /*printf("process cleanup\n");*/
}

#define WAIT_STATUS_TYPE int

static char * tty_find_pty(FakeTerm w)
{
	int err;
    char *pty;
    if((w->master = open("/dev/ptmx",O_RDWR,0)) >= 0)
    {
        err=grantpt(w->master);
        if (err < 0) {
        	perror("Granting PTY\n");
        	return NULL;
        }
        err = unlockpt(w->master);
        if (err < 0) {
        	perror("Unlocking PTY\n");
        	return NULL;
        }
        pty = ptsname(w->master);
        fprintf(stderr, "Using XOPEN pty %s\n", pty);
        if((w->slave = open(pty, O_RDWR,0)) >= 0)
        {
            return strdup(pty);
        }
        else
        {
            close(w->master);
        }
    }
    return NULL;
}

typedef struct termios TtyStuff;

static void tty_set_size(FakeTerm w, int rows, int cols, int width, int height)
{
     struct winsize wz;
     int pgrp;

     wz.ws_row = rows;
     wz.ws_col = cols;
     wz.ws_xpixel = width;
     wz.ws_ypixel = height;
     ioctl(w->slave, TIOCSWINSZ, &wz);
#ifdef TIOCGPGRP
     if (ioctl(w->slave, TIOCGPGRP, &pgrp) != -1)
	  killpg(pgrp, SIGWINCH);
#endif
}

void pty_cleanup(pid_t pid, int id)
{
     FakeTerm tp;

     tp=pty_list;
     while(tp)
       {
	 if (pid==tp->pid)
	   {
	     if (tp->job_control != 1)
	       tp->sd->killed=1;
	     return;
	   }
	 tp=tp->next;
       }
}

void fake_process_create(FakeTerm w)
{
     /*signal(SIGCHLD, process_reaper);*/
     process_init(w);
}

static int doargs(char *p, char **cmdp, char ***argp)
{
    int n,ff,qq,q,issp;
    char *p1,*p3,*pp;
    char **x,**xav;

#if 1
    p3=malloc(strlen(p)*2);
    issp=q=qq=0;

    for(p1=p,pp=p3;*p1;)
      {
	if (*p1=='\'') q=(!q),qq=1;
	else qq=0;
	if (q || qq)
	  {
	    *pp++=*p1++;
	    continue;
	  }


	if ((*p1!='<') && (*p1!='>') && !(isdigit(*p1) && (*(p1+1)=='<' || *(p1+1)=='>')))
	  {
	    if (!issp || *p1!=' ')
	      *pp++=*p1++;
	    else p1++;

	    if (*(p1-1)==' ') issp=1;
	    else issp=0;

	  }

	else  /* insert spaces around redirection form */
	  {
	    *pp++=' ';
	    ff=0;
	    if (!isdigit(*p1)) ff++;
	    *pp++=*p1++;

	    while(ff<2 && (*p1=='<' || *p1=='>'))
	      {
		*pp++=*p1++;
		ff++;
	      }

	    *pp++=' ';
	  }
      }
    *pp=0;
    pp=p3;    /* free !!!! */

    /*printf("pty args are : %s\n",pp);*/

#endif
#if 1
    p1=pp;
    n=1;

    while (p3)
      {
	p3=strpbrk(p3," '\t");
	if (p3)
	  {
	    if (*p3=='\'')
	      {
		p3=strpbrk(p3+1,"'");
		if (p3)p3+=1;

		continue;
	      }

	    *p3++=0;
	  }
	n++;
      }
    xav = x = malloc((n+1) * sizeof(char *));
    ff = 0;
    p3=p1;
    while(--n>0)
      {
	/*printf("setting arg %d to %s\n",x-xav,p3);*/

	*x=p3;
	if(ff == 0)
	{
	  *cmdp = *x;ff=1;
	}
	x++;
	p3=p3+strlen(p3)+1;
      }

    *x = NULL;
    *argp = xav;
    return n;

#else

    p3 = strdup(pp);
    for(n = 0, p1 = p3; (p2 = strtok(p1, " \t")); p1 = NULL, n++)
        ;
    free(p3);

    xav = x = malloc((n+1) * sizeof(char *));
    n = 0;
    for(p1 = p; (p2 = strtok(p1, " \t")); n++, p1 = NULL)
    {
        *x = strdup(p2);
	if(n == 0)
	{
	    *cmdp = *x;
	}
	x++;
    }
    *x = NULL;
    *argp = xav;
    return n;

#endif
}

/* convert UQLX-> unix filenames and prepare redirection a'la 'sh' */
void conv_fnames(char ***cmd, int *redir)
{
}



FakeTerm fake_tty_open(char *cmd)
{
	FakeTerm w;
	int redir[20];


	if((w = calloc(1,sizeof(FakeRec))))
	{
		if(cmd)
		{
			doargs(cmd, &w->command, &w->command_args);
			conv_fnames(&w->command_args,redir);
		}
		else
		{
			w->command = DEFAULTCOMMAND;
			w->command_args = NULL;
		}
		w->term_type =  DEFAULTTERMTYPE;

		if (!(w->tname = tty_find_pty(w)))
		{
			free(w);
			return NULL;
		}
		else
		{
			tty_set_size (w, 24, 80, 480, 240);
			fake_process_create(w);
			/*printf("pty child process %d\n",w->pid);*/
			return w;
		}
	}
	return NULL;
}

void fake_tty_close(FakeTerm w)
{
  /*printf("fake_tty_close\n");*/

    if(w)
    {
	char **x;

	if (w->master >= 0)
	    close(w->master);
	if (w->slave >= 0)
	    close(w->slave);
	if(w->tname)
	    free(w->tname);
	x = w->command_args;
	if (x && *x) free(x);

	/* for(x = w->command_args; x && *x; x++)
	   {
	        if(*x) free(*x);
	   }*/
	/*if(w->command_args) free (w->command_args);*/

	/*free(w);*/
	/*w = NULL;*/
	process_cleanup(w);
    }
}

#ifdef TEST
int check(int f1, int f2, fd_set *rfds)
{
    if(FD_ISSET(f1, rfds))
    {
	int nb;
	int nn;
	char buf[80];

	if(ioctl(f1, FIONREAD, &nn) == 0)
	{
	    if(nn > 80) nn = 80;
	    if((nb = read(f1, buf, nn)) > 0)
	    {
		write(f2, buf, nb);
	    }
	}
    }
}

void fake_handle_process()
{
    int n, nc,nr ;
    short done = 0;
    fd_set rfds;

    FD_ZERO(&rfds);
    nr = 0;

    while ( !done )
    {
	FD_SET(0, &rfds);
	FD_SET(w->master, &rfds);
	n = select ( FD_SETSIZE, &rfds, 0, 0, NULL);
  	switch(n)
	{
	  case 0:
	    nr = 0;
	    done = 1;
	    break;
	  case -1:
	    nr = -1;
	    done = 1;
	    break;
	  default:
	    check(0, w->master, &rfds);
	    check(w->master, 1, &rfds);
	    break;
	}
   }
}

int main(int ac, char *av)
{
    if(fake_tty_open () > 0)
    {
	printf("fds are %d %d\n", w->master, w->slave);
	fake_handle_process();
	fake_tty_close();
    }
}
#endif
#endif /* SERIAL */
