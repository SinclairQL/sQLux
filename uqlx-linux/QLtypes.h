/*
 * (c) UQLX - see COPYRIGHT
 */


#define true 1
#define false 0

typedef signed char w8;
typedef signed short w16;
typedef signed int w32;
typedef unsigned char uw8;
typedef unsigned short uw16;
typedef unsigned int uw32;
typedef unsigned char Cond;
#ifndef BOOLEAN_ALREADY_DEFINED
typedef Cond Boolean;
#endif
typedef void* Ptr;     /* non ANSI, but convenient... */


struct qFloat{
        uw16    exp;
        uw32    mant;
};

#include "misdefs.h"
