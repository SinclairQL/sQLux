/*
 * (c) UQLX - see COPYRIGHT
 */

/* trap#2,d0=1 open codes */

#define Q_IO_OLD 0
#define Q_IO_SHARE 1
#define Q_RDONLY Q_IO_SHARE
#define Q_IO_NEW 2
#define Q_IO_OVERW 3
#define Q_DIR 4

/* QDOS error codes */

#define QERR_NC   -1       /*  not complete */
#define QERR_BJ   -2       /* bad job */
#define QERR_OM   -3       /* out of memory */
#define QERR_OR   -4       /* out of range */
#define QERR_BF   -5       /* buffer full */
#define QERR_NO   -6       /* no open */
#define QERR_NF   -7
#define QERR_EX   -8
#define QERR_IU   -9
#define QERR_EOF  -10
#define QERR_EF QERR_EOF
#define QERR_DF   -11
#define QERR_BN   -12
#define QERR_TE   -13
#define QERR_FP   -14
#define QERR_BP   -15
#define QERR_BM   -16      /* bad or changed medium */
#define QERR_XP   -17
#define QERR_OV   -18
#define QERR_NI   -19
#define QERR_RO   -20
#define QERR_BL   -21

extern int qerrno;
extern long long qlClock;

extern w32  EMUL_IPC_LOC;
extern w32  EMUL_IPCR_LOC;
extern w32  EMUL_IPCW_LOC;
extern w32  EMUL_KEYTRANS_LOC;
extern uw16 EMUL_KT_CODE;


void SetDisplayMode(void);
void SetInitilaDisplay(void);
void SetDisplay(w8,Cond);
void ZeroKeyboardBuffer(void);
