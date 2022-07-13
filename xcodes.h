/*
 * (c) UQLX - see COPYRIGHT
 */



/* variable addresses */
extern uint32_t MIPC_CMD_ADDR;        /* Minerva sx_ipcom */
extern uint32_t IPC_CMD_ADDR;         /* JS IPC patch*/
extern uint32_t IPCR_CMD_ADDR;
extern uint32_t IPCW_CMD_ADDR;
extern uint32_t FSTART_CMD_ADDR;      /* JS Fast start patch */
extern uint32_t KEYTRANS_CMD_ADDR;    /* JS Keytranslation patch */
extern uint16_t KEYTRANS_OCODE;
extern uint32_t ROMINIT_CMD_ADDR;     /* patch to link in drivers */
extern uint16_t ROMINIT_OCODE;
extern uint16_t DEVPEFIO_OCODE;
extern uint16_t DEVPEFO_OCODE;


 /* hardwired addresses in "QEML ROM" */
#define MDVIO_CMD_ADDR      0x14000  
#define MDVO_CMD_ADDR       0x14002
#define MDVC_CMD_ADDR       0x14004
#define MDVSL_CMD_ADDR      0x14006
#define MDVFO_CMD_ADDR      0x14008

#define DEVIO_CMD_ADDR      0x14020
#define DEVC_CMD_ADDR       0x14022   /* DEVO_CMD_ADDR not hardwired */

#define SCHEDULER_CMD_ADDR  0x14030   /* scheduler loop routine */
#define MIPC_CMD_ADDR       0x14032
#define POLL_CMD_ADDR       0x14034   /* gap needed before next address !!!! */
#define KBENC_CMD_ADDR      0x14044   /* ... cause they call each other */

#if 1
#define DEVPEF_IO_ADDR      0x14050   /* extended screen fool driver */
#define DEVPEF_OPEN_ADDR    0x14054   /* for ptr_gen */
#define DEVPEF_CLOSE_ADDR   0x14058
#endif

#define UQLX_STR_SCRATCH    0x14400


/* instruction opcodes */
#define IPC_CMD_CODE        0xadc0
#define IPCR_CMD_CODE       0xadc1
#define IPCW_CMD_CODE       0xadc2
#define MDVR_CMD_CODE       0xadc3   /* patch vectored utilities */
#define MDVW_CMD_CODE       0xadc4
#define MDVV_CMD_CODE       0xadc5
#define MDVH_CMD_CODE       0xadc6
#define FSTART_CMD_CODE     0xadc7
#define KEYTRANS_CMD_CODE   0xadc8

#define ROMINIT_CMD_CODE    0xaaa0
#define MDVIO_CMD_CODE      0xaaa1
#define MDVO_CMD_CODE       0xaaa2
#define MDVC_CMD_CODE       0xaaa3
#define MDVSL_CMD_CODE      0xaaa4
#define MDVFO_CMD_CODE      0xaaa5

#define OSERIO_CMD_CODE     0xaaa6  /* old style serial driver*/
#define OSERO_CMD_CODE      0xaaa7  /*  *** OBSOLETE *** */
#define OSERC_CMD_CODE      0xaaa8

#define SCHEDULER_CMD_CODE  0xaaa9

#define DEVIO_CMD_CODE      0xaaaa
#define DEVO_CMD_CODE       0xaaab
#define DEVC_CMD_CODE       0xaaac

#define MIPC_CMD_CODE       0xaaad 
#define POLL_CMD_CODE       0xaaae
#define KBENC_CMD_CODE      0xaaaf

#define BASEXT_CMD_CODE     0xaab0

#define DEVPEF_CMD_CODE     0xaab1
#define DEVPEFO_CMD_CODE    0xaab2

#define REGEMU_CMD_CODE     0xaab3

extern void PollCmd(void);
extern void KBencCmd(void);
extern void BASEXTCmd(void);
extern void RegEmuCmd(void);



