/*
 * (c) UQLX - see COPYRIGHT
 */



/* QVFS is (c) by HPR    */
/* this file implements something remotely similar */
/* .. hoping to become compatible sometimes */

/* currently this IS NOT HPR's QVFS*/

#ifdef QVFS

/*#include "QLtypes.h"*/
#include "QL68000.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MDV_ID                       0x28b07ae4

#define min(_a_,_b_)  (_a_<_b_ ? _a_ :_b_)


#include "QL.h"
#include "xcodes.h"

#include "driver.h"
#include "QDOS.h"

#include "QInstAddr.h"
#include "unix.h"

#include "QLfiles.h"
#include "QFilesPriv.h"
#include "QL_driver.h"
#include "QVFS.h"
#include "util.h"


static char qvf_mount[4096];
static char qvf_mname[4096];
static char qvf_buff[4096];


int qvf_init(int id,void *p){return 0;}

int qvf_test(int id, char *name)
{
  int strict,creat;
  int res,key;
  char *cname,*pname,*cmount;
    unsigned int i;

  strict=0;
  creat=0;

  /* no delete or directory yet */
  key=reg[3];

  i=RW(name);
  if (i>4000) i=4000;
  strncpy(qvf_buff,name+2,i);
  qvf_buff[i]=0;

  cname=qvf_buff;

  if(RW(name)>3)
    if (!strncasecmp(cname,"XVFS_",5))
      {
	cname+=5;     /* skip optional name heading */
	strict=1;
      }

  if (!strict && cname[0]!='/' ) return 0;

  if (key==-1 || key==4)
    {
      qerrno=QERR_NI;
      return -2;
    }

#if 0
  if (strlen(cname)>4000)
    return strict ? -1 :0;
#endif

  cmount=qvf_mount;
  *cmount++='/';
  cmount[0]=0;
  pname=cname;
  qvf_mname[0]=0;


  if (*pname=='/') pname++;


  if (key==4)
    res=match(qvf_mount, qvf_mname, pname, 1,0,4000,0);
  else
    {
      creat=0;
      if (key==2 &&
	  (res=match(qvf_mount, qvf_mname, pname, 0,creat,4000,0)))
	{
	  qerrno=QERR_EX;
	  return -2;
	}


      cmount=qvf_mount;
      *cmount++='/';
      cmount[0]=0;
      pname=cname;
      qvf_mname[0]=0;

      if (*pname=='/') pname++;

      if (key >=2 ) creat=1;
      res=match(qvf_mount, qvf_mname, pname, 0,creat,4000,0);
    }

  /*printf("qvf_mount: %s\nqvf_mname: %s\nres=%d\n",qvf_mount,qvf_mname,res);*/


  return res;
}

int qvf_open(int id, void **priv)
{
  qvf_priv *p;
  int perm,fd;
  struct HF_FCB *fcb;
  struct mdvFile *f;
  char *cp;
  struct stat sbuf;

  if (reg[3]==4)
    /*fd = qopendir(qvf_mount,qvf_mname,4000);*/
    return -1;
  else
    {
      perm = O_RDWR | O_CREAT ;
      fd = qopenfile(qvf_mount,qvf_mname,perm,0666,4000);
      if (fd<0)
	{
	  fd = qopenfile(qvf_mount,qvf_mname,O_RDWR,0666,4000);
	  if (fd<0)
	    fd = qopenfile(qvf_mount,qvf_mname,O_RDONLY ,0666,4000);
	}
    }

  if (fd<0)
    return -1;

  fstat(fd,&sbuf);

  p=*priv=malloc(sizeof(qvf_priv));

  f=&(p->f);

#ifndef EMX
#define DEVMODE (S_IFREG | S_IFDIR | S_IFCHR | S_IFBLK)
#else
#define DEVMODE (S_IFREG | S_IFDIR | S_IFCHR)
#endif

  if (sbuf.st_mode & DEVMODE)
    {
      p->isdev=0;
      SET_FCB(f,&(p->fcb));
      strcpy(GET_FCB(f)->uxname,"/");   /* HACK !!! hide mount */
      strncat(GET_FCB(f)->uxname,qvf_mname,4094);

      SET_HFILE(f,fd);
      /*SET_FNUMBER(f,-1);*/
      SET_OPEN(f,true);
      SET_FILESYS(f,-1);
      SET_DRIVE(f,-1);
      SET_ISDISK(f,0);
      SET_KEY(f,reg[3]);
      SET_ISDIR(f,4==reg[3]);
      SET_EOF(f,HDfLen(f,0));
      SET_NEXT(f,nil);
      SET_ID(f,MDV_ID);

      /*cpy name into header!!!*/
      cp=strrchr(qvf_mname,'/');
      if (cp) cp++;
      else cp=qvf_mname;

      strncpy(NAME_REF(f)+2,cp,36);
      WW(NAME_REF(f),min(36,strlen(cp)));
    }
  else p->isdev=1;

  p->fd=fd;

  return 0;
}

void qvf_close(int id, void *priv)
{
  qvf_priv *p=priv;

  close(GET_HFILE(&(p->f)));

}

int qvf_pend(qvf_priv *p)
{
  if (check_pend(p->fd,SLC_READ)) return 0;
  else return QERR_NC;
}

int qvf_read(qvf_priv *p, void *buf, int pno)
{
  int res;

  res=read(p->fd,buf,pno);

  if (res<0) res=qmaperr();
  return res;
}

static int qvf_write(qvf_priv *p, void *buf, int pno)
{
  int res;

  res=write(p->fd,buf,pno);

  if (res<0) res=qmaperr();
  return res;
}

void qvf_io(int id, void *priv)
{
  qvf_priv *p=priv;

  if (p->isdev)
    io_handle(qvf_read,qvf_write,qvf_pend, p);
  else
    QHostIO(&(p->f),reg[0],0);

}

#endif /* QVFS */
