/*
 * (c) UQLX - see COPYRIGHT
 */

/* definitions for remote control and message passing */


struct paste_buf
{
  struct paste_buf *next;  /* linked list */
  char *p;                 /* next char */
  int size;                /* # of chars */
  char text[0];
};


/* all data in network byte order: */

typedef struct
{
  int len;  /* len of additional data */
  int type;
  union {
    int spar;
#if 0
    struct DEVINFO dev;
#endif
  } body;
} message;

#define MSG_MINSIZE  12

/* message types */
#define MX_EXIT  1
#define MX_CLONE 2
#define MX_BREAK 3     /* int par */
#define MX_DEVINFO 4
#define MX_PASTE 5
#define MX_HOG 6       /* int par */
#define MX_KEYSW 7     /* int par */
#define MX_REDRAW 8

#define MX_DEBUG_ON     9
#define MX_DEBUG_OFF   10
/* DEBUG subtype in message.spar: */
#define DBG_FXIO          1     /* file io, unix subtype */
#define DBG_FQIO          2     /*          qdos-fs */
#define DBG_SCRIO         3
#define DBG_SERIO         4
#define DBG_SOCKIO        5
#define DBG_SYSTRACE1     6     /* QDOS traps 1  */
#define DBG_SYSTRACE2     7     /* QDOS traps 2  */
#define DBG_SYSTRACE3     8     /* QDOS traps 3  */


#define MX_SYSI   11
/* SYS subtype in message.par */
#define SYS_WPROT          1     /* protocol window output, job name substr
				   in additional data */
#define SYS_JOBS           2     /* list QDOS jobs */
