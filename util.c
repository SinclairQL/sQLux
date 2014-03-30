/*
 * (c) UQLX - see COPYRIGHT
 */



/* some miscelaneous utility fucntions */

/*#include "QLtypes.h"*/
#include "QL68000.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#ifdef IPDEV
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#include <sys/ioctl.h>


#include "QSerial.h"
#include "QDOS.h"
#include "driver.h"
#include "uqlx_cfg.h"
#include "util.h"


int check_pend(int fd,int mode) 
{
  struct timeval tv;
  fd_set wfd,errfd,rfd,xfd,*xx;
  int res;

  /*printf("calling check_pend\n");*/

  switch(mode)
    {
    case SLC_READ: xx=&rfd;;
      break;
    case SLC_WRITE: xx=&wfd;
      break;
    case SLC_ERR: xx=&errfd;
      break;
    default : printf("wrong mode for check_pend: %d\n",mode);
      return 0;     
    }
  
  tv.tv_sec=0;
  tv.tv_usec=0;
  
  FD_ZERO(&wfd);
  FD_ZERO(&errfd);
  FD_ZERO(&rfd);
  /*FD_ZERO(&xfd);*/

  FD_SET(fd,xx);
  

  res=select(fd+1,&rfd,&wfd,&errfd,&tv);
  /*printf("select returns %d, rfds %d wfds %d, errfds %d\n",res,rfd,wfd,errfd);*/
  
  if (res<0) return 0;
  /*return FD_ISSET(fd,xx);*/
  return (res>0);
}
