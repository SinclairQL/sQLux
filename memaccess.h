/*
 * (c) UQLX - see COPYRIGHT
 */


/* define memory access fns */

#ifndef MEMACCESS_H
#define MEMACCESS_H

#include "QL68000.h"

rw8 ReadByte(aw32 addr);
rw16 ReadWord(aw32 addr);
rw32 ReadLong(aw32 addr);
void WriteByte(aw32 addr,aw8 d);
void WriteWord(aw32 addr,aw16 d);
void WriteLong(aw32 addr,aw32 d);

rw8 ModifyAtEA_b(ashort mode,ashort r);
rw16 ModifyAtEA_w(ashort mode,ashort r);
rw32 ModifyAtEA_l(ashort mode,ashort r);
void RewriteEA_b(aw8 d);
void rwb_acc(w8 d);
void RewriteEA_w(aw16 d);
void rww_acc(w16 d);
void RewriteEA_l(aw32 d);
void rwl_acc(w32 d);

#define QL_ROM_BASE             0x0000
#define QL_ROM_SIZE             0x10000
#define QL_INTERNAL_IO_BASE     0x18000
#define QL_INTERNAL_IO_SIZE     0x4000
#define QL_EXTERNAL_IO_BASE     0x1C000
#define QL_EXTERNAL_IO_SIZE     0x4000
#define QL_SCREEN_BASE          0x20000

#endif /* _MEMACCESS_H */
