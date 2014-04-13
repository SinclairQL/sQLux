/*
 * (c) UQLX - see COPYRIGHT
 */


#define true 1
#define false 0

typedef int8_t w8;
typedef int16_t w16;
typedef int32_t w32;
typedef uint8_t uw8;
typedef uint16_t uw16;
typedef uint32_t uw32;
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
