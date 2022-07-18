/*
 * (c) UQLX - see COPYRIGHT
 */


/* define memory access fns */

#pragma once

#include <stddef.h>

int8_t ReadByte(int32_t addr);
int16_t ReadWord(int32_t addr);
int32_t ReadLong(int32_t addr);
void WriteByte(int32_t addr,int8_t d);
void WriteWord(int32_t addr,int16_t d);
void WriteLong(int32_t addr,int32_t d);

int8_t ModifyAtEA_b(int16_t mode, int16_t r);
int16_t ModifyAtEA_w(int16_t mode, int16_t r);
int32_t ModifyAtEA_l(int16_t mode, int16_t r);
void RewriteEA_b(int8_t d);
void rwb_acc(int8_t d);
void RewriteEA_w(int16_t d);
void rww_acc(int16_t d);
void RewriteEA_l(int32_t d);
void rwl_acc(int32_t d);

void ChangedMemory(uint32_t from, uint32_t to);

#define QL_ROM_BASE             0x0000
#define QL_ROM_SIZE             0xC000
#define QL_ROM_PORT_BASE        0xC000
#define QL_ROM_PORT_SIZE        0x4000
#define QL_ROM2_BASE            0x10000
#define QL_ROM2_SIZE            0x4000
#define QL_ROM3_BASE            0x14000
#define QL_ROM3_SIZE            0x4000
#define QL_INTERNAL_IO_BASE     0x18000
#define QL_INTERNAL_IO_SIZE     0x4000
#define QL_EXTERNAL_IO_BASE     0x1C000
#define QL_EXTERNAL_IO_SIZE     0x4000
#define QL_SCREEN_BASE          0x20000
