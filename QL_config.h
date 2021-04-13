/*
 * (c) UQLX - see COPYRIGHT
 */

#ifndef _QL_CONFIG_H
#define _QL_CONFIG_H

void InitROM(void);
void EmulatorTable(Ptr);
void save_regs(void *p);
void restore_regs(void *p);
Cond LookFor(uw32 *a, uw32 w, long nMax);
short LoadMainRom(void);

#endif /*_QL_CONFIG_H*/
