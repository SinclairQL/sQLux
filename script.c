/*
 * (c) UQLX - see COPYRIGHT
 */

/* script redirection IO */

#include <fcntl.h>
#include <unistd.h>

#include "QL68000.h"
#include "util.h"
#include "script.h"
#include "QDOS.h"
#include "qx_proto.h"

int script_read_enable=0;

int script_read(void *p, void *buf, int len)
{
  int res;

  /*printf("script enable %d\n",script_read_enable);*/
  if (!script_read_enable)
    return 0;

  res=read(0,buf,len);
  /*printf("**** scrpit_read : %s\n",buf);*/
  if (res<0) res=qmaperr();
  return res;
}
int script_write(void *p, void * buf, int len)
{
  int res;

  res=write(1,buf,len);

  if (res<0) res=qmaperr();
  return res;
}
int script_pend(void *p)
{
  if (!script_read_enable)
    return QERR_NC;

  if (check_pend(0,SLC_READ)) return 0;
  else return QERR_NC;
}

