/*
 * (c) UQLX - see COPYRIGHT
 */


/* define memory access fns */

#ifndef _MEMACCESS_H
#define _MEMACCESS_H

#define ReadDispByte(_addr_) (*(uw8 *)((Ptr)theROM+((long)_addr_)))
#define ReadDispWord(__addr__) RW((uw16*)((Ptr)theROM+((long)__addr__)))
#define ReadDispLong(_adr_) (RL((Ptr)theROM+((long)_adr_)))

extern rw8 ReadByte(aw32 addr);
extern rw16 ReadWord(aw32 addr);
extern rw32 ReadLong(aw32 addr);
extern void WriteByte(aw32 addr,aw8 d);
extern void WriteWord(aw32 addr,aw16 d);
extern void WriteLong(aw32 addr,aw32 d);

extern rw8 ModifyAtEA_b(ashort mode,ashort r);
extern rw16 ModifyAtEA_w(ashort mode,ashort r);
extern rw32 ModifyAtEA_l(ashort mode,ashort r);
extern void RewriteEA_b(aw8 d);
extern void rwb_acc(w8 d);
extern void RewriteEA_w(aw16 d);
extern void rww_acc(w16 d);
extern void RewriteEA_l(aw32 d);
extern void rwl_acc(w32 d);

#endif /* _MEMACCESS_H */
