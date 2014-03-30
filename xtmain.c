/*
 * (c) UQLX - see COPYRIGHT
 */


#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/*#include "QLtypes.h"*/
#include "QL68000.h"
#include "QL_config.h" 
#include "QInstAddr.h"

#include "unix.h"
#include "boot.h"

void x_reset_state()
{}

int screen_drawable=1;

extern char **argv;

int QLInit(int ac, char **av)
{
  char *rf;
  int  rl=0;
  void *tbuff;
  int j,c;
  int mem=-1, col=-1, hog=-1;

  argv=av;
  
  SetParams(ac, av);
  
  uqlxInit();
}

void destroy_image(){}

  


