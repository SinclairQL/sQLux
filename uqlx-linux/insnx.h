/*
 * (c) UQLX - see COPYRIGHT
 */


#define SZ_B 0
#define SZ_W 1
#define SZ_L 2

#define A_READ 0
#define A_WRITE 1

struct  RX_A 
{
  int xp;  /* 0: exception unxpected, 1 op, 2 op */
/*  int (*undo_test)(); /* for many insn's additional decoding*/
/* is needed to localize the exception, eg move.l (a0)+,(a1)- */
/* ... move to extra table*/
  
  int pcchange1;  /* oldpc-xpc if 1 op error*/
  int pcchange2;  /* oldpc-xpc .. 2 ... */
  
  int regnum1;
  int chng1;     /* 0+anyreg == nochange! */
  int regnum2;
  int chng2;     /* oldstate-exceptionstate */
};

extern struct RX_A *rxa_table;

extern int faultaddr; /* vm.c may use it to actually do something usefull */
