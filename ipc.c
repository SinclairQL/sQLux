/*
 * ipc.c - Low level emulation of the QL IPC (Intel 8049)
 *
 * Wiring (Sinclair QL motherboard, as in the MAME QL driver):
 *   P1        keyboard matrix column outputs (KBO0-7)
 *   BUS       keyboard matrix row inputs (KBI0-7): OR of the selected columns
 *   P2.0      serial data input (SER2 RxD), idle high
 *   P2.6      serial handshake input, copied into bit 6 of the IPC status;
 *             reads low with nothing connected
 *   P2.1      speaker
 *   P2.2/2.3  interrupt request lines to the 68008 (IPL0/2 and IPL1)
 *   P2.7      COMDATA, the serial link with the ZX8302 (wired AND)
 *   MOVX wr   COMCTL pulse to the ZX8302
 *   T1        BAUDx4 from the ZX8302 (serial reception, not emulated)
 *
 * ZX8302 side of the link (zx8302.v of the MiSTer QL core):
 *   A write to $18003 loads four bits (----XEDS: start, data, stop, extra
 *   stop) and sets the two busy bits. On every falling edge of COMCTL the
 *   ZX8302 latches COMDATA for the CPU, shifts the four bits towards the IPC
 *   and clears one busy bit. $18020 bit 6 is busy and bit 7 is COMDATA.
 *
 * Timing: the 8049 runs at 11 MHz / 15 = 733333 machine cycles per second,
 * from its own crystal. Its time is kept in units of 1/165 MHz (one 8049
 * machine cycle is 225 units) and is real time: each frame lasts 1/hz s for
 * the 8049 whatever the SPEED of the 68008, and within the frame it advances
 * in proportion to the 68008 cycles executed (at SPEED = 1, 22 units per
 * 68008 cycle, as on a QL).
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>                         /* SDL_GetTicks */

#include "ipc.h"
#include "mcs48.h"

#define UNITS_PER_IPC_CYCLE   225
#define UNITS_PER_SECOND      165000000u
#define UNITS_PER_MS          (UNITS_PER_SECOND / 1000u)
/* Longest catch-up in one call (one second): avoids stalls after long pauses */
#define MAX_CATCHUP_UNITS     ((uint64_t)UNITS_PER_SECOND)

extern uint64_t ql_cycles;                       /* iexl_general.c */
extern void ql_set_ipc_ipl(int lines);         /* general.c */

static int      lle_active = 0;
static uint8_t  rom[2048];
static mcs48_t  ipc;
static uint64_t ipc_units;               /* time reached by the 8049 */

/* IPC clock: the current frame in 68008 cycles and in IPC time */
static uint64_t frame_start_cycles = 0;  /* ql_cycles at the last sync */
static uint64_t frame_start_units = 0;   /* IPC time at the last sync */
static uint64_t frame_len_cycles = 150000;  /* 0: unlimited speed */
static uint64_t frame_units = UNITS_PER_SECOND / 50;
static uint64_t frame_host_start = 0;   /* host counter at the last sync */

/* IPC time that corresponds to the current 68008 cycle. At unlimited speed
 * the 68008 cycles say nothing about time, so the 8049 follows the host
 * clock instead, never beyond the end of the frame. */
static uint64_t ipc_target(void)
{
	uint64_t dt;

	if (frame_len_cycles) {
		dt = (ql_cycles - frame_start_cycles) * frame_units /
		     frame_len_cycles;
	} else {
		uint64_t freq = SDL_GetPerformanceFrequency();
		dt = (SDL_GetPerformanceCounter() - frame_host_start) *
		     (UNITS_PER_SECOND / 1000u) / (freq / 1000u ? freq / 1000u : 1);
		if (dt > frame_units)
			dt = frame_units;
	}
	return frame_start_units + dt;
}

/* ZX8302 side of the link */
static uint8_t  comdata_reg = 0x0f;      /* line released: idle high */
static uint8_t  ipc_busy = 0;
static uint8_t  comdata_to_cpu = 1;

static int      ipl_lines = 3;           /* P2.2, P2.3: 1 = inactive */

/* HW_TRACE: link bits and notes started since the last report */
static unsigned trace_bits = 0;
static unsigned trace_notes = 0;
static uint64_t speaker_last_edge = 0;   /* 8049 time of the last toggle */

static uint64_t trace_units0 = 0;
static uint32_t trace_host0 = 0;

/* HW_TRACE >= 2: the traffic on the link is decoded as the IPC protocol,
 * whatever the time between bits (programs may send an order a few bits per
 * frame): 4 bits of order, then its parameters (bits sent by the 68008) and
 * its answer (bits returned by the 8049), with the length of each order.
 * Sound orders (10) are printed with their 8 parameter bytes. */
static int      trace_level = 0;
static int      pr_state = 0;            /* 0 order, 1 parameters, 2 answer */
static int      pr_cmd = 0, pr_stage = 0;
static int      pr_need = 4, pr_have = 0;
static uint32_t pr_acc = 0;
static uint8_t  pr_param[8];
static int      pr_prev_sent = -1;       /* bit of the previous transaction */

void ipc_lle_set_trace(int level)
{
	trace_level = level;
}

static void pr_expect(int state, int bits)
{
	pr_state = state;
	pr_need = bits;
	pr_have = 0;
	pr_acc = 0;
	if (!bits) {                    /* nothing more: next order */
		pr_state = 0;
		pr_need = 4;
	}
}

static void pr_order(void)
{
	pr_cmd = (int)pr_acc;
	pr_stage = 0;
	switch (pr_cmd) {
	case 1:  pr_expect(2, 8);  break;       /* status */
	case 6:
	case 7:  pr_expect(2, 8);  break;       /* serial data: count first */
	case 8:  pr_expect(2, 4);  break;       /* keyboard: count first */
	case 9:  pr_expect(1, 4);  break;       /* KEYROW: row, then answer */
	case 10: pr_expect(1, 64); break;       /* sound */
	case 11:                                /* kill sound */
		printf("IPC KILL SOUND at %.3f s\n",
		       ipc_units / (double)UNITS_PER_SECOND);
		fflush(stdout);
		pr_expect(0, 0);
		break;
	case 12:
	case 13: pr_expect(1, 4);  break;       /* one nibble parameter */
	case 14: pr_expect(2, 16); break;       /* random number */
	case 15: pr_expect(1, 8);  break;       /* test: byte, then answer */
	default: pr_expect(0, 0);  break;
	}
}

static void pr_done(void)
{
	int i;

	if (pr_state == 1) {                    /* parameters complete */
		if (pr_cmd == 10) {
			printf("IPC BEEP at %.3f s: %02X %02X %02X %02X %02X %02X "
			       "%02X %02X (pitch %u, pitch 2 %u, interval %u, "
			       "duration %u, step/wrap %02X, fuzzy/random %02X)\n",
			       ipc_units / (double)UNITS_PER_SECOND,
			       pr_param[0], pr_param[1], pr_param[2], pr_param[3],
			       pr_param[4], pr_param[5], pr_param[6], pr_param[7],
			       pr_param[0], pr_param[1],
			       pr_param[2] | (pr_param[3] << 8),
			       pr_param[4] | (pr_param[5] << 8),
			       pr_param[6], pr_param[7]);
			fflush(stdout);
			pr_expect(0, 0);
		} else if (pr_cmd == 9 || pr_cmd == 15) {
			pr_expect(2, 8);
		} else {
			pr_expect(0, 0);
		}
		return;
	}
	/* answer complete */
	if (pr_cmd == 8 && pr_stage == 0) {     /* keys: 4 + 8 bits each */
		pr_stage = 1;
		pr_expect(2, (int)(pr_acc & 7) * 12);
	} else if ((pr_cmd == 6 || pr_cmd == 7) && pr_stage == 0) {
		pr_stage = 1;
		pr_expect(2, (int)(pr_acc & 0xff) * 8);
	} else {
		pr_expect(0, 0);
	}
	(void)i;
}

/* One transaction of the link: the bit sent and the bit received */
static void pr_feed(int sent, int received)
{
	int bit = pr_state == 2 ? received : sent;

	if (pr_state == 1 && pr_cmd == 10 && pr_have < 64) {
		int k = pr_have >> 3;
		pr_param[k] = (uint8_t)((pr_have & 7) ? (pr_param[k] << 1) | bit : bit);
	}
	pr_acc = (pr_acc << 1) | (uint32_t)bit;
	if (++pr_have < pr_need)
		return;
	if (pr_state == 0)
		pr_order();
	else
		pr_done();
}

void ipc_lle_trace(unsigned *bits, unsigned *notes, unsigned *ipc_ms,
		   unsigned *host_ms)
{
	uint32_t now = SDL_GetTicks();

	*bits = trace_bits;
	*notes = trace_notes;
	*ipc_ms = (unsigned)((ipc_units - trace_units0) / UNITS_PER_MS);
	*host_ms = (unsigned)(now - trace_host0);
	trace_bits = trace_notes = 0;
	trace_units0 = ipc_units;
	trace_host0 = now;
}

/* BAUDx4: square wave from the ZX8302 at four times the serial baud rate,
 * read by the 8049 on T1. Baud rate selected by $18002 bits 0-2:
 * 19200 >> n (n = 7: 75 baud). */
static unsigned baud = 9600;

/* ------------------------------------------------------------------------ */
/* Speaker output: the pin is averaged over each output sample in emulated  */
/* time and queued for the audio callback.                                  */
/* ------------------------------------------------------------------------ */

#define AUDIO_RING      16384            /* power of two */
#define AUDIO_MAX_FILL  8192             /* samples dropped above this */
/* Fill level kept by the drift control: above the samples produced at once
 * per 50 Hz frame (882) plus one audio callback (512). */
#define AUDIO_TARGET    1600
#ifndef AUDIO_MAX_DRIFT
#define AUDIO_MAX_DRIFT 0.002            /* +-0.2 % read rate correction */
#endif
#ifndef AUDIO_GAIN
#define AUDIO_GAIN 60.0                  /* rate = 1 + dev / (target * gain) */
#endif
/* The fill level rises and falls at 50 Hz because each frame is emulated
 * at once; the drift control follows its average over about 0.5 s, so the
 * read rate does not wobble within a frame. */
#ifndef AUDIO_AVG_SAMPLES
#define AUDIO_AVG_SAMPLES 44100.0
#endif
#define AUDIO_AMPLITUDE 6000

static int      audio_rate = 0;
static uint64_t units_per_sample = 0;
static uint64_t sample_units = 0;        /* units accumulated in the sample */
static double   sample_acc = 0.0;        /* integral of the filtered level */

/* The speaker and its drive filter the square wave: two low-pass stages at
 * AUDIO_CUTOFF_HZ, applied at the resolution of the 8049 before sampling,
 * so that its harmonics above half the sample rate do not fold back
 * (aliasing, heard as a rough, "airy" tone). */
#define AUDIO_CUTOFF_HZ 7000.0
static double   lp1 = 0.0, lp2 = 0.0;
static double   lp_tau_units = 1.0;      /* time constant in 1/165 MHz units */
static double   lp_k1 = 0.0, lp_k2 = 0.0; /* exp(-t/tau) for 1 and 2 cycles */
static uint8_t  speaker = 0;
static int16_t  ring[AUDIO_RING];
static volatile unsigned ring_wr = 0, ring_rd = 0;
static double   ring_frac = 0.0;         /* fractional read position */
static double   fill_avg = AUDIO_TARGET; /* averaged fill level */
static int      playing = 0;
static int16_t  last_out = 0;
static double   hp_x = 0.0, hp_y = 0.0;  /* DC blocking filter state */

static void audio_advance(uint64_t units)
{
	double x, k;

	if (!audio_rate)
		return;

	/* the pin does not change during one instruction of the 8049 */
	x = speaker ? 1.0 : 0.0;
	k = units == UNITS_PER_IPC_CYCLE ? lp_k1 :
	    units == 2 * UNITS_PER_IPC_CYCLE ? lp_k2 :
	    exp(-(double)units / lp_tau_units);
	lp1 = x + (lp1 - x) * k;
	lp2 = lp1 + (lp2 - lp1) * k;

	while (units) {
		uint64_t room = units_per_sample - sample_units;
		uint64_t take = units < room ? units : room;

		sample_units += take;
		sample_acc += lp2 * (double)take;
		units -= take;

		if (sample_units == units_per_sample) {
			unsigned wr = ring_wr, rd = ring_rd;
			if (((wr - rd) & (AUDIO_RING - 1)) < AUDIO_MAX_FILL) {
				ring[wr & (AUDIO_RING - 1)] = (int16_t)(
					AUDIO_AMPLITUDE * sample_acc /
					(double)units_per_sample);
				ring_wr = (wr + 1) & (AUDIO_RING - 1);
			}
			sample_units = 0;
			sample_acc = 0.0;
		}
	}
}

void ipc_lle_audio_init(int sample_rate)
{
	if (sample_rate <= 0)
		return;
	audio_rate = sample_rate;
	units_per_sample = UNITS_PER_SECOND / (uint64_t)sample_rate;
	sample_units = 0;
	sample_acc = 0.0;
	lp1 = lp2 = 0.0;
	lp_tau_units = (double)UNITS_PER_SECOND / (2.0 * M_PI * AUDIO_CUTOFF_HZ);
	lp_k1 = exp(-(double)UNITS_PER_IPC_CYCLE / lp_tau_units);
	lp_k2 = exp(-2.0 * UNITS_PER_IPC_CYCLE / lp_tau_units);
	ring_wr = ring_rd = 0;
	ring_frac = 0.0;
	fill_avg = AUDIO_TARGET;
	playing = 0;
}

void ipc_lle_audio_mix(int16_t *stream, int frames)
{
	int i;

	if (!lle_active || !audio_rate)
		return;

	for (i = 0; i < frames; i++) {
		unsigned wr = ring_wr, rd = ring_rd;
		unsigned fill = (wr - rd) & (AUDIO_RING - 1);
		double x, y;
		int32_t l, r;

		/* The samples are produced with the emulated clock and consumed
		 * with the clock of the sound card. Start once AUDIO_TARGET
		 * samples are queued, and keep the fill level around it by
		 * reading slightly faster or slower (linear interpolation). */
		if (!playing && fill >= AUDIO_TARGET) {
			playing = 1;
			fill_avg = (double)fill;
		}
		if (playing && fill < 2)
			playing = 0;            /* underrun: wait for the target */

		fill_avg += ((double)fill - fill_avg) / AUDIO_AVG_SAMPLES;

		if (playing) {
			double rate = 1.0 + (fill_avg - AUDIO_TARGET) /
					    (AUDIO_TARGET * AUDIO_GAIN);
			int16_t s0 = ring[rd];
			int16_t s1 = ring[(rd + 1) & (AUDIO_RING - 1)];
			unsigned step;

			if (rate > 1.0 + AUDIO_MAX_DRIFT)
				rate = 1.0 + AUDIO_MAX_DRIFT;
			if (rate < 1.0 - AUDIO_MAX_DRIFT)
				rate = 1.0 - AUDIO_MAX_DRIFT;

			last_out = (int16_t)(s0 + (s1 - s0) * ring_frac);
			ring_frac += rate;
			step = (unsigned)ring_frac;
			ring_frac -= step;
			ring_rd = (rd + step) & (AUDIO_RING - 1);
		}

		/* remove the DC level of a square wave between 0 and +amplitude */
		x = (double)last_out;
		y = x - hp_x + 0.995 * hp_y;
		hp_x = x;
		hp_y = y;

		l = stream[2 * i] + (int32_t)y;
		r = stream[2 * i + 1] + (int32_t)y;
		stream[2 * i]     = (int16_t)(l > 32767 ? 32767 : l < -32768 ? -32768 : l);
		stream[2 * i + 1] = (int16_t)(r > 32767 ? 32767 : r < -32768 ? -32768 : r);
	}
}

/* ------------------------------------------------------------------------ */
/* 8049 port wiring                                                         */
/* ------------------------------------------------------------------------ */

static uint8_t p2_in(void *ctx)
{
	(void)ctx;
	/* P2.7 = COMDATA from the ZX8302 and P2.0 = SER2 RxD (idle high), as in
	 * the MAME QL driver. The other pins read low: P2.6 in particular is
	 * copied into bit 6 of the IPC status (order 1), which reads 0 on a QL
	 * with nothing connected to the serial ports. */
	return (uint8_t)(0x01 | ((comdata_reg & 1) << 7));
}

static void p2_out(void *ctx, uint8_t v)
{
	int lines;
	(void)ctx;

	if (((v >> 1) & 1) != speaker) {
		/* a note starts when the speaker moves after 5 ms of silence */
		if (ipc_units - speaker_last_edge > UNITS_PER_SECOND / 200)
			trace_notes++;
		speaker_last_edge = ipc_units;
	}
	speaker = (v >> 1) & 1;

	/* P2.2 drives IPL0/2 and P2.3 drives IPL1 (active low) */
	lines = ((v >> 2) & 1) | (((v >> 3) & 1) << 1);
	if (lines != ipl_lines) {
		ipl_lines = lines;
		ql_set_ipc_ipl(lines);
	}
}

/* ------------------------------------------------------------------------ */
/* Keyboard matrix (see IPC_KEY_SPACING_CYCLES in ipc.h)                    */
/* ------------------------------------------------------------------------ */

#define KEYQ_LEN 128                     /* power of two */

static struct {
	uint8_t  code;
	uint8_t  pressed;
	uint32_t host_ms;                /* host time of the change */
} keyq[KEYQ_LEN];
static volatile unsigned keyq_wr = 0, keyq_rd = 0;
static uint8_t  matrix[8];               /* rows as read by the 8049 */
static uint64_t key_next = 0;            /* earliest cycle for the next change */

/* Host time <-> IPC time, set on every vertical sync */
static uint32_t ref_host_ms = 0;
static uint64_t ref_units = 0;
static int      ref_valid = 0;

void ipc_lle_key(int code, int pressed)
{
	unsigned wr = keyq_wr;

	if (((wr + 1) & (KEYQ_LEN - 1)) == keyq_rd)
		return;                         /* queue full: drop */
	keyq[wr].code = (uint8_t)(code & 0x3f);
	keyq[wr].pressed = (uint8_t)(pressed ? 1 : 0);
	keyq[wr].host_ms = SDL_GetTicks();
	keyq_wr = (wr + 1) & (KEYQ_LEN - 1);
}

/* Apply the queued changes whose time (IPC clock) has come */
static void key_apply(uint64_t now)
{
	while (keyq_rd != keyq_wr) {
		unsigned rd = keyq_rd;
		int code = keyq[rd].code;
		int row = 7 - code / 8;
		uint8_t bit = (uint8_t)(1 << (code % 8));
		uint64_t t = 0;

		if (ref_valid) {
			/* one frame late: a change made while the emulator waited
			 * for this frame falls within it, at the same offset */
			int64_t dt = (int64_t)(int32_t)(keyq[rd].host_ms - ref_host_ms) *
				     (int64_t)UNITS_PER_MS + (int64_t)frame_units;
			if (dt < 0)
				dt = 0;
			t = ref_units + (uint64_t)dt;
		}
		if (t < key_next)
			t = key_next;
		if (now < t)
			break;

		/* The original firmware loses keys when a modifier (SHIFT, CTRL,
		 * ALT: codes 0-2) changes while another key is held, which on a
		 * host keyboard happens all the time when typing fast. Release
		 * the held keys first (they have already been accepted); the
		 * modifier changes on the next step. */
		if (code <= 2 && ((matrix[7] & 0xf8) || matrix[0] || matrix[1] ||
				  matrix[2] || matrix[3] || matrix[4] ||
				  matrix[5] || matrix[6])) {
			matrix[7] &= 0x07;
			memset(matrix, 0, 7);
			key_next = t + (uint64_t)IPC_KEY_MIN_SPACING_MS * UNITS_PER_MS;
			continue;
		}

		if (keyq[rd].pressed)
			matrix[row] |= bit;
		else
			matrix[row] &= (uint8_t)~bit;
		keyq_rd = (rd + 1) & (KEYQ_LEN - 1);
		key_next = t + (uint64_t)IPC_KEY_MIN_SPACING_MS * UNITS_PER_MS;
	}
}

static uint8_t bus_in(void *ctx)
{
	uint8_t v = 0;
	int i;
	(void)ctx;

	key_apply(ipc_units);

	/* row 7 holds SHIFT (bit 0), CTRL (bit 1) and ALT (bit 2) */
	for (i = 0; i < 8; i++)
		if (ipc.p1 & (1 << i))
			v |= matrix[i];
	return v;
}

static void movx_wr(void *ctx, uint8_t addr, uint8_t v)
{
	(void)ctx; (void)addr; (void)v;

	/* COMCTL pulse: latch COMDATA (wired AND of both sides), shift the bits
	 * towards the IPC and clear one busy bit */
	comdata_to_cpu = (uint8_t)((comdata_reg & 1) & ((ipc.p2 >> 7) & 1));
	comdata_reg = (uint8_t)((comdata_reg >> 1) | 0x08);
	ipc_busy = (uint8_t)(ipc_busy >> 1);
}

static int t1_in(void *ctx)
{
	/* half period of BAUDx4 in 1/165 MHz units: 165e6 / (8 * baud) */
	uint64_t half = UNITS_PER_SECOND / (8u * baud);
	(void)ctx;
	return (int)((ipc_units / half) & 1);
}

void ipc_lle_set_baud(uint8_t ctrl)
{
	static const unsigned rates[8] = { 19200, 9600, 4800, 2400, 1200, 600, 300, 75 };
	baud = rates[ctrl & 7];
}

static int int_in(void *ctx)
{
	(void)ctx;
	return 1;                       /* INT not asserted */
}

/* ------------------------------------------------------------------------ */

/* Firmware: an Intel HEX file, or a raw 2K binary dump */
static int load_firmware(const char *path)
{
	FILE *f = fopen(path, "rb");
	static uint8_t file[65536];
	size_t len, i;
	int bytes = 0;

	if (!f)
		return -1;
	len = fread(file, 1, sizeof(file), f);
	fclose(f);
	memset(rom, 0, sizeof(rom));

	for (i = 0; i < len && (file[i] == ' ' || file[i] == '\r' ||
				file[i] == '\n' || file[i] == '\t'); i++)
		;
	if (i < len && file[i] == ':') {
		/* Intel HEX: data records only */
		char *p = (char *)file;
		file[len < sizeof(file) ? len : sizeof(file) - 1] = 0;
		while ((p = strchr(p, ':')) != NULL) {
			unsigned rl, addr, type, k, v;

			if (sscanf(p + 1, "%2x%4x%2x", &rl, &addr, &type) == 3 &&
			    type == 0) {
				for (k = 0; k < rl; k++) {
					if (sscanf(p + 9 + 2 * k, "%2x", &v) != 1)
						break;
					if (addr + k < sizeof(rom)) {
						rom[addr + k] = (uint8_t)v;
						bytes++;
					}
				}
			}
			p++;
		}
		return bytes;
	}

	/* raw binary: the 2K of the 8049 (a larger dump keeps its first 2K) */
	if (len < sizeof(rom))
		return -1;
	memcpy(rom, file, sizeof(rom));
	return (int)sizeof(rom);
}

static uint32_t crc32_rom(void)
{
	uint32_t crc = 0xffffffffu;
	unsigned i, k;

	for (i = 0; i < sizeof(rom); i++) {
		crc ^= rom[i];
		for (k = 0; k < 8; k++)
			crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1)));
	}
	return ~crc;
}

int ipc_lle_init(const char *hex_path)
{
	mcs48_io_t io;
	int bytes;

	lle_active = 0;
	if (!hex_path || !*hex_path)
		return 0;

	bytes = load_firmware(hex_path);
	if (bytes <= 0) {
		fprintf(stderr, "IPC_ROM: cannot read %s, using the high level IPC\n",
			hex_path);
		return 0;
	}

	memset(&io, 0, sizeof(io));
	io.p2_in = p2_in;
	io.p2_out = p2_out;
	io.bus_in = bus_in;
	io.movx_wr = movx_wr;
	io.int_in = int_in;
	io.t1_in = t1_in;

	comdata_reg = 0x0f;
	ipc_busy = 0;
	comdata_to_cpu = 1;
	ipl_lines = 3;
	memset(matrix, 0, sizeof(matrix));
	keyq_rd = keyq_wr;
	key_next = 0;
	mcs48_init(&ipc, rom, sizeof(rom), &io);
	ipc_units = 0;
	frame_start_cycles = ql_cycles;
	frame_start_units = 0;
	frame_len_cycles = 150000;
	frame_units = UNITS_PER_SECOND / 50;
	ref_valid = 0;

	lle_active = 1;
	printf("IPC: low level emulation, firmware %s (%d bytes, CRC32 %08X)\n",
	       hex_path, bytes, crc32_rom());
	return 1;
}

int ipc_lle_active(void)
{
	return lle_active;
}

static void run_until(uint64_t target)
{
	if (target > ipc_units + MAX_CATCHUP_UNITS)
		ipc_units = target - MAX_CATCHUP_UNITS;

	while (ipc_units < target) {
		uint64_t u = (uint64_t)mcs48_step(&ipc) * UNITS_PER_IPC_CYCLE;
		audio_advance(u);
		ipc_units += u;
	}
}

void ipc_lle_sync(void)
{
	if (lle_active)
		run_until(ipc_target());
}

void ipc_lle_frame(uint64_t frame_cycles, unsigned native_cycles)
{
	uint64_t end;

	if (!lle_active)
		return;

	/* The 8049 completes the frame that has just ended... */
	end = frame_start_units + frame_units;
	run_until(end);

	/* ...and the new one starts exactly 1/hz s after the previous one. If
	 * the 68008 ran past the end of the frame, the 8049 is ahead and waits
	 * for it at the start of the new frame; if it fell short, the 8049 has
	 * just caught up. Either way its time stays real time. */
	frame_start_cycles = ql_cycles;
	frame_start_units = end;
	/* 22 units per cycle of a 68008 at 7.5 MHz */
	frame_units = (uint64_t)(native_cycles ? native_cycles : 149760) * 22u;
	frame_len_cycles = frame_cycles;
	frame_host_start = SDL_GetPerformanceCounter();

	ref_host_ms = SDL_GetTicks();
	ref_units = frame_start_units;
	ref_valid = 1;
}

void ipc_lle_write_comdata(uint8_t d)
{
	ipc_lle_sync();
	trace_bits++;
	if (trace_level >= 2) {
		/* the previous transaction is complete: its answer is latched */
		if (pr_prev_sent >= 0)
			pr_feed(pr_prev_sent, comdata_to_cpu);
		pr_prev_sent = (d >> 1) & 1;
	}
	comdata_reg = d & 0x0f;
	ipc_busy = 3;                   /* busy until COMCTL pulses twice */
}

uint8_t ipc_lle_status(void)
{
	ipc_lle_sync();
	return (uint8_t)((comdata_to_cpu << 7) | ((ipc_busy & 1) << 6));
}
