#pragma once

#include <stdio.h>

#if defined(DEBUG)
    #define DEBUG_PRINT(fmt, args...) fprintf(stderr, "DEBUG: %d:%s(): " \
    fmt, __LINE__, __func__, ##args)
#else
    #define DEBUG_PRINT(fmt, args...)
#endif
