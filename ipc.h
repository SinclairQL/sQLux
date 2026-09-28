/*
 * ipc.h - Low level emulation of the QL IPC (Intel 8049)
 *
 * The 8049 runs its original firmware and is connected as on the QL
 * motherboard: keyboard matrix, speaker, interrupt lines and the serial link
 * with the ZX8302 ($18003 write, $18020 bits 6 and 7). It is enabled with the
 * IPC_ROM option; otherwise the high level IPC emulation is used.
 */

#ifndef IPC_H
#define IPC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Load the firmware (Intel HEX or a raw 2K binary) and start the 8049.
 * Returns 1 on success, 0 if the low level IPC is not used. */
int     ipc_lle_init(const char *hex_path);
int     ipc_lle_active(void);

/* Run the 8049 up to the current emulated time (ql_cycles) */
void    ipc_lle_sync(void);

/* ZX8302 side of the link */
void    ipc_lle_write_comdata(uint8_t d);       /* write to $18003 */
uint8_t ipc_lle_status(void);                   /* $18020 bits 7 and 6 */
void    ipc_lle_set_baud(uint8_t ctrl);         /* write to $18002 */

/* Keyboard matrix. The host keyboard handling queues every change of a key
 * (QL key code 0-63; SHIFT, CTRL and ALT are codes 0, 1 and 2 of row 7)
 * together with the host time at which it happened. The emulator runs each
 * frame in a single burst and then waits for the next one, so the host time
 * of every change is mapped onto the next frame of the IPC clock (one frame
 * of latency): the 8049 sees each key held for as long as it really was, and
 * never two changes in the same scan, which the original firmware ignores.
 * Consecutive changes are kept at least IPC_KEY_MIN_SPACING_MS apart (enough
 * for the firmware to tell a modifier from the key that follows it). */
#define IPC_KEY_MIN_SPACING_MS  2
void    ipc_lle_key(int code, int pressed);     /* host thread */

/* Vertical sync. The IPC has its own clock (11 MHz crystal): every frame
 * lasts its real time for the 8049 (native_cycles at 7.5 MHz) whatever the
 * SPEED of the 68008, and within the frame its time advances with the 68008
 * cycles executed. frame_cycles is the length of the frame in 68008 cycles
 * at the current speed (0 at unlimited speed: the 8049 then follows the
 * host clock within the frame). */
void    ipc_lle_frame(uint64_t frame_cycles, unsigned native_cycles);

/* HW_TRACE: link bits, notes started, IPC time (ms) and host time (ms)
 * since the last call */
void    ipc_lle_trace(unsigned *bits, unsigned *notes, unsigned *ipc_ms,
		      unsigned *host_ms);
/* HW_TRACE >= 2: also print the parameters of every sound order */
void    ipc_lle_set_trace(int level);

/* Speaker output */
void    ipc_lle_audio_init(int sample_rate);
void    ipc_lle_audio_mix(int16_t *stream, int frames);   /* stereo s16 */

#ifdef __cplusplus
}
#endif

#endif /* IPC_H */
