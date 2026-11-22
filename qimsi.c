/*
 * qimsi.c - QIMSI PS/2 mouse interface for sQLux
 *
 * The QIMSI registers sit in the ROM port, which has no write strobe, so
 * every register is a read. Only the mouse is emulated; its PS/2 side
 * reports an IntelliMouse in stream mode, as QIMSI firmware leaves it after
 * power-up. Based on qimsi.js from QLRun.
 */

#include <string.h>

#include "qimsi.h"
#include "zx8301.h"

extern uint64_t ql_cycles; /* iexl_general.c */

#define MOUSE_CODE   0xFEDD
#define MOUSE_UNLOCK 0xFEDE
#define MOUSE_STATUS 0xFEDF

#define STATUS_RECEIVED 0x01
#define STATUS_WHEEL    0x10
#define PACKET_ALWAYS   0x08
#define PACKET_XSIGN    0x10
#define PACKET_YSIGN    0x20
#define PACKET_LEN      4
#define BUTTON_MASK     0x07
#define PS2_CLOCK_HZ    12000
#define PS2_FRAME_BITS  11
#define SAMPLE_HZ       100
#define MAX_PACKET_MOVE 127
#define MAX_PENDING     255
#define MIN_WHEEL       (-8)
#define MAX_WHEEL       7
#define MAX_QUEUED      4

int qimsi_mapped = 0;

static struct {
	uint64_t byte_cycles, sample_cycles;
	uint8_t packet[PACKET_LEN];
	int pos, len;
	uint8_t code;
	uint64_t ready_at, sample_at;
	int dx, dy, dz; /* host motion, +y down */
	int buttons, reported;
	int queued[MAX_QUEUED];
	int nqueued;
} q;

static int clampi(int v, int lo, int hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

void qimsi_init(void)
{
	memset(&q, 0, sizeof(q));
	q.byte_cycles = (uint64_t)ZX_CPU_HZ * PS2_FRAME_BITS / PS2_CLOCK_HZ;
	q.sample_cycles = ZX_CPU_HZ / SAMPLE_HZ;
	qimsi_mapped = 1;
}

/* Latch the next stream packet when there is motion or a button change */
static void sample_packet(uint64_t now)
{
	int mx, my, mz, buttons, head;

	if (q.dx == 0 && q.dy == 0 && q.dz == 0 && q.nqueued == 0)
		return;

	buttons = q.reported;
	if (q.nqueued > 0) {
		buttons = q.queued[0];
		memmove(q.queued, q.queued + 1, (--q.nqueued) * sizeof(int));
	}
	mx = clampi(q.dx, -MAX_PACKET_MOVE, MAX_PACKET_MOVE);
	my = clampi(-q.dy, -MAX_PACKET_MOVE, MAX_PACKET_MOVE); /* up is + */
	mz = q.dz;
	q.dx -= mx;
	q.dy += my;
	q.dz = 0;

	head = PACKET_ALWAYS | buttons;
	if (mx < 0)
		head |= PACKET_XSIGN;
	if (my < 0)
		head |= PACKET_YSIGN;
	q.packet[0] = head;
	q.packet[1] = mx & 0xFF;
	q.packet[2] = my & 0xFF;
	q.packet[3] = mz & 0xFF;
	q.reported = buttons;
	q.pos = 0;
	q.len = PACKET_LEN;
	q.sample_at = now + q.sample_cycles;
}

uint8_t qimsi_read_byte(uint32_t addr)
{
	uint64_t now = ql_cycles;
	int ready;

	addr &= 0xFFFF;
	if (q.pos >= q.len && now >= q.sample_at)
		sample_packet(now);
	ready = q.pos < q.len && now >= q.ready_at;

	switch (addr) {
	case MOUSE_CODE:
		if (ready)
			q.code = q.packet[q.pos];
		return q.code;
	case MOUSE_UNLOCK:
		if (ready) {
			q.code = q.packet[q.pos];
			q.pos++;
			q.ready_at = now + q.byte_cycles;
		}
		return 0;
	case MOUSE_STATUS:
		return STATUS_WHEEL | (ready ? STATUS_RECEIVED : 0);
	default:
		return 0;
	}
}

void qimsi_move(int dx, int dy)
{
	q.dx = clampi(q.dx + dx, -MAX_PENDING, MAX_PENDING);
	q.dy = clampi(q.dy + dy, -MAX_PENDING, MAX_PENDING);
}

void qimsi_scroll(int dz)
{
	q.dz = clampi(q.dz + dz, MIN_WHEEL, MAX_WHEEL);
}

void qimsi_buttons(int mask)
{
	int last;

	mask &= BUTTON_MASK;
	q.buttons = mask;
	last = q.nqueued ? q.queued[q.nqueued - 1] : q.reported;
	if (mask == last)
		return;
	if (q.nqueued == MAX_QUEUED)
		q.queued[q.nqueued - 1] = mask;
	else
		q.queued[q.nqueued++] = mask;
}
