/*
 * (c) UQLX - see COPYRIGHT
 */


/*#include "QLtypes.h"*/
#include "QL68000.h"

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

#include "QL_config.h" 
#include "QInstAddr.h"

#include "unix.h"
#include "boot.h"
#include "qx_proto.h"

extern char **argv;

int main(int ac, char **av)
{
  argv=av;

  SetParams(ac, av);
  
  uqlxInit();
  
  QLRun();

  return 0;
}
