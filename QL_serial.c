/*
 *		Copyright (c) 1996 jonathan hudson  
 */

/* most of the code has moved to QLserio.c */

#ifdef SERIAL
#include "QL68000.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <fcntl.h>


#include "QL.h"
#include "QInstAddr.h"
#include "unix.h"

#include "QSerial.h"
#include "uqlx_cfg.h"

static short defbaud[1] = {9600};

serdev_t *sparams[MAXSERIAL+1];

Cond SetBaudRate(short rate)
{
    short i;
    
    for(i = 1; i < MAXSERIAL+1; i++)
    {
	defbaud[0] = rate;
	if (sparams[i])
	  {
	    (sparams[i])->baud = rate;
	    if((sparams[i])->fd > 0)
	      {
		tty_baud(sparams[i]);
	      }
	  }
    }
    return 1;
}

#endif /* SERIAL */
