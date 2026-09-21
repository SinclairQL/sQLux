#ifndef QSOUND_H
#define QSOUND_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Modos de sonido
#define QSOUND_MODE_MONO       0
#define QSOUND_MODE_STEREO_ABC 1
#define QSOUND_MODE_STEREO_ACB 2

void    qsound_init(int clock_hz, int sample_rate, int stereo_mode);
void    qsound_reset(void);
void    qsound_set_enabled(int enabled);
int     qsound_is_enabled(void);
int     qsound_is_read_active(uint32_t addr);

void    qsound_write_byte(uint32_t addr, uint8_t val);
uint8_t qsound_read_byte(uint32_t addr);

// Función original de 8 bits
void    qsound_render_mix_s8(int8_t *stream, int len);

// Nueva función de 16 bits (manteniendo tu algoritmo intacto)
void    qsound_render_mix_s16(int16_t *stream, int len);

#ifdef __cplusplus
}
#endif

#endif // QSOUND_H