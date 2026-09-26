/*
 * zx8301.h - ZX8301 beam position and memory contention in clock cycles
 *
 * The frame interrupt always follows the real video signal: 50 Hz for PAL and
 * 60 Hz for NTSC, paced by the host clock. What happens within a frame (beam
 * position, RAM contention, line by line screen capture) is placed by
 * counting emulated clock cycles from the vertical sync.
 */

#ifndef ZX8301_H
#define ZX8301_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZX_CPU_HZ            7500000
#define ZX_VISIBLE_LINES     256
#define ZX_RAM_BASE          0x20000
#define ZX_RAM_SIZE          0x20000

/* Contention active in the current frame (check before calling the hooks) */
extern int zx_contention;

/* ntsc: 0 = PAL (50 Hz, 312 lines), 1 = NTSC (60 Hz, 262 lines)
 * vsync_lines: lines from the frame interrupt to the first visible line
 *              (< 0 = default: 36 for PAL, 4 for NTSC) */
void     zx8301_init(int contention, int ntsc, int vsync_lines);

unsigned zx8301_hz(void);                  /* 50 or 60 */
unsigned zx8301_lines(void);               /* lines per frame */
unsigned zx8301_first_visible(void);       /* lines from VSYNC to line 0 */
/* Cycles per speed unit: speed = SPEED * 20 units per frame, so SPEED = 1
 * gives ZX_CPU_HZ / hz cycles per frame (7.5 MHz). */
unsigned zx8301_speed_unit(void);

/* Cycle at which visible line vline (0..255) starts, in a frame that began
 * with the vertical sync at frame_start and lasts frame_len cycles (0 means
 * unlimited speed). Lines last 64 us (PAL) or 63.2 us (NTSC) of that clock. */
uint64_t zx8301_line_time(uint64_t frame_start, uint64_t frame_len,
			  unsigned vline);

/* Vertical sync: a frame starts at cycle 'now'. speed as in unixstuff.c */
void     zx8301_frame(uint64_t now, int speed);

/* Start of an instruction: bus time and program fetch from RAM */
void     zx8301_insn(uint64_t t0, uint32_t pc_addr, unsigned prog_words);
/* Data access of 'bytes' bytes to the internal RAM */
void     zx8301_ram(unsigned bytes, int is_write);

static inline int zx8301_is_ram(uint32_t addr)
{
	return (uint32_t)(addr - ZX_RAM_BASE) < ZX_RAM_SIZE;
}

#ifdef __cplusplus
}
#endif

#endif /* ZX8301_H */
