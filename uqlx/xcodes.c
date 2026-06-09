/*
 * (c) UQLX - see COPYRIGHT
 */


/*#include "QLtypes.h"*/
#include "QL68000.h"
#include "QL.h"
#include "xcodes.h"

/* variable addresses */

uw32 IPC_CMD_ADDR;         /* JS IPC patch*/
uw32 IPCR_CMD_ADDR;
uw32 IPCW_CMD_ADDR;
uw32 FSTART_CMD_ADDR;      /* JS Fast start patch */
uw32 KEYTRANS_CMD_ADDR;    /* JS Keytranslation patch */
uw16 KEYTRANS_OCODE;
uw32 ROMINIT_CMD_ADDR;     /* patch to link in drivers */
uw16 ROMINIT_OCODE;
uw16 DEVPEFIO_OCODE;
uw16 DEVPEFO_OCODE;

