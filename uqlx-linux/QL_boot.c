/*
 * (c) UQLX - see COPYRIGHT
 */



/*#include "QLtypes.h"*/
#include "QL68000.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

#include "QL.h"
#include "QLfiles.h"
#include "QFilesPriv.h"
#include "QInstAddr.h"
#include "QDisk.h"
#include "unix.h"

#include "driver.h"
#include "QDOS.h"
#include "qx_proto.h"

/* emulate BOOT device */
/* take input from string, file or stdin */
/*      determined by ux_boot & ux_bname */
/*           0: no BOOT device    */
/*           1: ux_bfd is file handle */
/*           2: ux_bname is command string */

int boot_init(int);
int boot_open(int, void**);
int boot_test(int, char*); 
void boot_close(int, void *); 
void boot_io(int, void *);

extern int ux_boot,ux_bfd;
extern char *ux_bname;

#define min(_xx_,_yy_) (_xx_<_yy_ ? _xx_ : _yy_)

struct BOOT_PRIV
{
  int fd;
  char * str_p;
  int (*read)();
  int (*write)();
  int (*pend)();
  
  int bfc_valid;
  char bfc;
};



int boot_write()
{
  return QERR_RO;
}

int u_pend(struct BOOT_PRIV *p)
{
  int res;
  
  if (p->bfc_valid)
    return 0;

  res=read(p->fd, &(p->bfc), 1);
  if (res==1)
    {
      p->bfc_valid=1;
      return 0;
    }
  return QERR_EF;
}

int str_pend(struct BOOT_PRIV *p)
{
  if (*(p->str_p)) return 0;
  else return QERR_EF;
}

int str_read(struct BOOT_PRIV *p, void *buf, int pno)
{
  long count;
  
  if (/*pno>0 &&*/ *(p->str_p)==0 || pno==0) 
    return 0;

  count=min(pno,strlen(p->str_p));
  
  strncpy(buf,p->str_p,count);
  p->str_p+=count; 
  
  return count;
}

int u_read(struct BOOT_PRIV *p, void *buf, int pno)
{
  long count,ci,res;
  char *c=buf;

  count=pno;
  
  if (p->bfc_valid)
    {
      *c++=p->bfc;
      p->bfc_valid=0;
      count--;ci=1;
    }

  res=read(p->fd, c, count);
 
  if (res>=0 ) return res+ci;
  else return qmaperr();
}



int boot_init(int idx)
{
  return 0;
}

open_arg boot_par; /* dummy */
int boot_test(int id, char *name)
{
  if (ux_boot==1 || ux_boot==2) 
    return decode_name(name,Drivers[id].namep,&boot_par);
  else return 0;
}


int boot_open(int id, void **priv)
{
  struct BOOT_PRIV *p;
  
  *priv=p=malloc(sizeof(struct BOOT_PRIV));
  if (ux_boot==1)
    {
      p->fd=ux_bfd;
      p->pend=u_pend;
      p->read=u_read;
      p->write=boot_write;
    }
  else
    {
      p->str_p=ux_bname;
      p->pend=str_pend; 
      p->read=str_read;
      p->write=boot_write;
    }

  return 0;
}


void boot_close(int id,void *priv)
{
  if(priv)
    free(priv);
}

void boot_io(int id, void *priv)
{
  struct BOOT_PRIV *p=priv;
  
  io_handle(p->read,p->write,p->pend,p);
}



