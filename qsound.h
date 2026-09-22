/*
 * qsound.h - QSound (AY-3-8910 / YM2149) Emulation for Sinclair QL (sQLux)
 *
 * Provides MC6821 PIA and direct I/O bus decoding, logarithmic DAC,
 * and 16-bit sound synthesis.
 */

#ifndef QSOUND_H
#define QSOUND_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Stereo panning modes
#define QSOUND_MODE_MONO       0
#define QSOUND_MODE_STEREO_ABC 1
#define QSOUND_MODE_STEREO_ACB 2

// Core lifecycle
void    qsound_init(int clock_hz, int sample_rate, int stereo_mode);
void    qsound_reset(void);
void    qsound_set_enabled(int enabled);
int     qsound_is_enabled(void);
int     qsound_is_read_active(uint32_t addr);

// Hardware bus access (I/O & PIA 6821)
void    qsound_write_byte(uint32_t addr, uint8_t val);
uint8_t qsound_read_byte(uint32_t addr);

// 16-bit signed audio rendering and mixing
void    qsound_render_mix_s16(int16_t *stream, int len);

#ifdef __cplusplus
}
#endif

#endif // QSOUND_H