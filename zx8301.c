/*
 * zx8301.c - ZX8301 beam position and memory contention in clock cycles
 *
 * VERTICAL SYNC
 *   The frame interrupt always follows the real video signal (50 Hz PAL,
 *   60 Hz NTSC), paced by the host clock (Pulse50Thread). On every vertical
 *   sync FrameInt calls zx8301_frame(); from then on the beam position is
 *   obtained by counting emulated cycles. At SPEED = 1 a frame lasts
 *   ZX_CPU_HZ / hz cycles (150000 for PAL).
 *
 * MEMORY CONTENTION (only at SPEED = 1, as on the MiSTer QL core)
 *   Follows the ql_timing module of the MiSTer QL core (Marcel Kilgus and
 *   Daniele Terdina), restricted to the internal RAM:
 *   - A line is made of 40 chunks of 12 cycles = 480 cycles (64 us).
 *   - During visible lines the ZX8301 uses 32 chunks (384 cycles) to fetch
 *     the screen; during the other lines it uses 8 chunks to refresh the
 *     DRAM.
 *   - Within a busy chunk the CPU can only start a RAM access on cycle 0 of
 *     the chunk: at most one byte every 12 cycles.
 *   - The ZX8301 decides one cycle after the CPU asserts DS. On a read DS is
 *     asserted in S2 and the decision is in time for S4 (no wait state if
 *     the RAM is free); on a write DS is asserted one cycle later, so there
 *     is always at least one wait state.
 *   - ROM, I/O and expansion RAM are not affected.
 */

#include "zx8301.h"

/* ---- Contention parameters (ql_timing) ---- */
#define ZX_CHUNK_CYCLES   12    /* cycles per chunk */
#define ZX_CHUNKS_VIDEO   32    /* busy chunks in a visible line */
#define ZX_CHUNKS_REFRESH 8     /* busy chunks in a non visible line */
#define ZX_CPU_ACCESS     4     /* byte access without wait states */
#define ZX_READ_DECIDE    2     /* cycle of the access at which the ZX8301 decides (read) */
#define ZX_WRITE_DECIDE   3     /* ... and on a write (DS one cycle later) */

/* ---- Frame geometry ---- */
/* PAL: 256 + 15 + 6 + 35 = 312 lines; NTSC: 256 + 2 + 2 + 2 = 262 lines.
 * Lines last 64 us (PAL) and 63.2 us (NTSC). */
#define ZX_PAL_LINES      312
/* Lines from the frame interrupt to the first visible line. Calibrated
 * against raster timed demos that run correctly on a real QL: all of them
 * work between 35 and 37, so the default is the centre of that range. */
#define ZX_PAL_FIRST      36
#define ZX_PAL_LINE_CYC   480
#define ZX_NTSC_LINES     262
#define ZX_NTSC_FIRST     4
#define ZX_NTSC_LINE_CYC  474
/* Line and frame length in 10.5 MHz pixel clocks: a line lasts 64 us (PAL)
 * or 63.2 us (NTSC); a frame lasts the 20 ms or 16.67 ms of the host clock.
 * At SPEED = 1 a line is therefore exactly 480 cycles, not 1/312 of 20 ms
 * (480.77). */
#define ZX_PAL_LINE_PX    672
#define ZX_PAL_FRAME_PX   210000
#define ZX_NTSC_LINE_PX   664
#define ZX_NTSC_FRAME_PX  175000

#define ZX_SPEED_NATIVE   20    /* speed = SPEED * 20 -> SPEED = 1 */

extern uint64_t ql_cycles;          /* iexl_general.c */

int zx_contention = 0;

static int      zx_contention_cfg = 0;
static unsigned zx_hz = 50;
static unsigned zx_nlines = ZX_PAL_LINES;
static unsigned zx_first = ZX_PAL_FIRST;
static unsigned zx_line_cyc = ZX_PAL_LINE_CYC;
static unsigned zx_line_px = ZX_PAL_LINE_PX;
static unsigned zx_frame_px = ZX_PAL_FRAME_PX;
static uint64_t zx_frame = 0;       /* cycle of the last vertical sync */
static uint64_t zx_bus_t = 0;       /* estimated time of the next bus access */

/* Current line, cached to avoid a division on every access: accesses come
 * in almost always increasing order, so it rarely needs recomputing. */
static uint64_t zx_line_t0 = 0;         /* first cycle of the line */
static uint64_t zx_line_t1 = 0;         /* first cycle after the line */
static unsigned zx_line_busy = 0;       /* busy chunks in this line */


void zx8301_init(int contention, int ntsc, int vsync_lines)
{
	zx_contention_cfg = contention;
	zx_contention = 0;
	zx_hz = ntsc ? 60 : 50;
	zx_nlines = ntsc ? ZX_NTSC_LINES : ZX_PAL_LINES;
	zx_first = ntsc ? ZX_NTSC_FIRST : ZX_PAL_FIRST;
	zx_line_cyc = ntsc ? ZX_NTSC_LINE_CYC : ZX_PAL_LINE_CYC;
	zx_line_px = ntsc ? ZX_NTSC_LINE_PX : ZX_PAL_LINE_PX;
	zx_frame_px = ntsc ? ZX_NTSC_FRAME_PX : ZX_PAL_FRAME_PX;
	/* Vertical phase adjustment (ZX8301_VSYNC_LINES) */
	if (vsync_lines >= 0 &&
	    (unsigned)vsync_lines + ZX_VISIBLE_LINES <= zx_nlines)
		zx_first = (unsigned)vsync_lines;
	zx_frame = ql_cycles;
}

unsigned zx8301_hz(void)            { return zx_hz; }
unsigned zx8301_lines(void)         { return zx_nlines; }
unsigned zx8301_first_visible(void) { return zx_first; }
unsigned zx8301_speed_unit(void)    { return ZX_CPU_HZ / zx_hz / 20; }

uint64_t zx8301_line_time(uint64_t frame_start, uint64_t frame_len,
			  unsigned vline)
{
	uint64_t pclk = (uint64_t)(zx_first + vline) * zx_line_px;

	return frame_start + frame_len * pclk / zx_frame_px;
}

void zx8301_frame(uint64_t now, int speed)
{
	zx_frame = now;
	zx_line_t0 = zx_line_t1 = 0;    /* force the line to be recomputed */
	zx_contention = zx_contention_cfg && speed == ZX_SPEED_NATIVE;
}

static void zx_locate_line(uint64_t c)
{
	uint64_t f = c - zx_frame;
	uint64_t line = f / zx_line_cyc;

	zx_line_t0 = zx_frame + line * zx_line_cyc;
	zx_line_t1 = zx_line_t0 + zx_line_cyc;
	/* Visible lines: screen fetch; any other line (including the cycles
	 * left at the end of the frame): DRAM refresh. */
	zx_line_busy = (line >= zx_first && line < zx_first + ZX_VISIBLE_LINES)
			       ? ZX_CHUNKS_VIDEO
			       : ZX_CHUNKS_REFRESH;
}

/* Can the CPU start a RAM access at cycle c? (could_start) */
static int zx_could_start(uint64_t c)
{
	uint32_t x;

	if (c < zx_line_t0 || c >= zx_line_t1 || c < zx_frame)
		zx_locate_line(c);

	x = (uint32_t)(c - zx_line_t0);
	/* Outside the busy chunks, or at the start of one */
	return x >= zx_line_busy * ZX_CHUNK_CYCLES || (x % ZX_CHUNK_CYCLES) == 0;
}

/* Wait states of a byte access starting at cycle t */
static unsigned zx_wait(uint64_t t, int is_write)
{
	uint64_t c = t + (is_write ? ZX_WRITE_DECIDE : ZX_READ_DECIDE);
	unsigned wait = is_write ? 1 : 0;       /* late DS on writes */

	while (!zx_could_start(c)) {            /* at most 11 iterations */
		c++;
		wait++;
	}
	return wait;
}

void zx8301_insn(uint64_t t0, uint32_t pc_addr, unsigned prog_words)
{
	zx_bus_t = t0;

	/* Program fetch from the internal RAM: 2 bytes per word */
	if (prog_words && zx8301_is_ram(pc_addr)) {
		unsigned i, total = 0;
		for (i = 0; i < 2 * prog_words; i++) {
			unsigned w = zx_wait(zx_bus_t, 0);
			total += w;
			zx_bus_t += ZX_CPU_ACCESS + w;
		}
		ql_cycles += total;
	}
}

void zx8301_ram(unsigned bytes, int is_write)
{
	unsigned i, total = 0;

	/* Access outside an instruction (e.g. exception stacking) or a very
	 * long instruction: resynchronise with the clock. */
	if (zx_bus_t + 1024 < ql_cycles)
		zx_bus_t = ql_cycles;

	for (i = 0; i < bytes; i++) {
		unsigned w = zx_wait(zx_bus_t, is_write);
		total += w;
		zx_bus_t += ZX_CPU_ACCESS + w;
	}
	ql_cycles += total;
}
