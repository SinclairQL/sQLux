/*
 * qsound.c - QSound Emulation for Sinclair QL (sQLux)
 * Accurate Bus Decoding (Byte & Word writes) & JT49 Noise Engine
 */

#include "qsound.h"
#include <string.h>
#include <stdio.h>

typedef struct {
    int enabled;
    double chip_freq;
    double sound_freq;
    int stereo_mode;

    // Registros AY-3-8910 / AY-3-8912 (14 registros)
    uint8_t regs[14];
    uint8_t ay_addr;

    // Puertos PIA 6821
    uint8_t port_a;
    uint8_t port_b;

    // Contadores de coma fija
    double tacts_per_sample;
    double tact_accum;

    // Generadores de Tono A, B, C
    int cnt_a, cnt_b, cnt_c;
    int bit_a, bit_b, bit_c;

    // Generador de Ruido JT49 (17-bit LFSR estricto)
    int cnt_n;
    uint32_t seed;
    int bit_n;

    // Generador de Envolvente
    int cnt_e;
    int env_step;
    int env_vol;
    int env_holding;

    // Filtro analógico paso-bajo suave
    double lpf_state;
} QSoundState;

static QSoundState qs;

// Tabla DAC logarítmica real (3dB por paso)
static const int16_t ayemu_dac_table[32] = {
    0,   0,   0,   0,   0,   1,   1,   1,
    1,   1,   1,   2,   2,   2,   3,   4,
    5,   6,   7,   8,  10,  12,  14,  17,
   20,  24,  28,  33,  37,  40,  40,  40
};

extern char *emulatorOptionString(const char *name);

// Buffer 8 KB para la ROM
static uint8_t qsound_rom[8192];
static int     qsound_rom_loaded = 0;
static size_t  qsound_rom_size   = 0;


void qsound_init(int clock_hz, int sample_rate, int stereo_mode) {
    memset(&qs, 0, sizeof(qs));
    qs.enabled = 1;
    // 750 kHz real del Sinclair QL por defecto
    qs.chip_freq = (clock_hz > 0) ? (double)clock_hz : 750000.0;
    qs.sound_freq = (sample_rate > 0) ? (double)sample_rate : 44100.0;
    qs.stereo_mode = stereo_mode;
    qs.tacts_per_sample = qs.chip_freq / (8.0 * qs.sound_freq);
    qsound_reset();
}

void qsound_reset(void) {
    memset(qs.regs, 0, sizeof(qs.regs));
    qs.regs[7] = 0xFF; // Silencio
    qs.ay_addr = 0;
    qs.port_a = 0;
    qs.port_b = 0;
    qs.tact_accum = 0;
    qs.lpf_state = 0.0;

    qs.cnt_a = qs.cnt_b = qs.cnt_c = 0;
    qs.bit_a = qs.bit_b = qs.bit_c = 0;

    qs.cnt_n = 0;
    qs.seed = 0x10000;
    qs.bit_n = 0;

    qs.cnt_e = 0;
    qs.env_step = 0;
    qs.env_vol = 0;
    qs.env_holding = 0;
}

void qsound_set_enabled(int enabled) { qs.enabled = enabled; }
int  qsound_is_enabled(void)         { return qs.enabled; }

int qsound_is_read_active(uint32_t addr) {
    if (!qs.enabled) return 0;
    uint32_t a = addr & 0xFFFF;
    if ((a & 0xF000) == 0x3000 && a != 0x3000) {
        return 1;
    }
    if ((a & 0xF000) == 0x2000 && (a == 0x2000 || a == 0x2001)) {
        return (qs.port_b & 0x01);
    }
    return 0;
}

static void ayemu_step_envelope(void) {
    if (qs.env_holding) return;

    uint8_t shape = qs.regs[13] & 0x0F;
    uint8_t attack = (shape & 0x04) ? 1 : 0;
    uint8_t alt    = (shape & 0x02) ? 1 : 0;
    uint8_t hold   = (shape & 0x01) ? 1 : 0;

    if (qs.env_step < 16) {
        qs.env_vol = attack ? qs.env_step : (15 - qs.env_step);
    } else if (qs.env_step < 32) {
        if (!(shape & 0x08)) {
            qs.env_vol = 0;
            qs.env_holding = 1;
        } else {
            if (hold) {
                qs.env_vol = alt ? (attack ? 0 : 15) : (attack ? 15 : 0);
                qs.env_holding = 1;
            } else {
                int step16 = qs.env_step - 16;
                qs.env_vol = alt ? (attack ? (15 - step16) : step16) : (attack ? step16 : (15 - step16));
            }
        }
    } else {
        if ((shape & 0x08) && !hold) {
            qs.env_step = 0;
            qs.env_vol = attack ? 0 : 15;
        } else {
            qs.env_holding = 1;
        }
    }

    if (!qs.env_holding) {
        qs.env_step++;
    }
}

static void ayemu_write_reg(uint8_t reg, uint8_t val) {
    if (reg > 13) return;
    qs.regs[reg] = val;
    if (reg == 13) {
        qs.cnt_e = 0;
        qs.env_step = 0;
        qs.env_holding = 0;
        uint8_t attack = (val & 0x04) ? 1 : 0;
        qs.env_vol = attack ? 0 : 15;
    }
}

// Decodificación de bus exacta para escrituras Byte y Word
void qsound_write_byte(uint32_t addr, uint8_t val) {
    if (!qs.enabled) return;

    uint32_t a = addr & 0xFFFF;

    // --- ACCESO DIRECTO (0xC3000 = Dirección, 0xC3001/0xC3002/0xC3003 = Dato) ---
    if ((a & 0xF000) == 0x3000) {
        if (a == 0x3000) {
            qs.ay_addr = val & 0x0F;
        } else {
            ayemu_write_reg(qs.ay_addr, val);
        }
        return;
    }

    // --- ACCESO PIA 6821 (0xC2000/1 = Datos, 0xC2002/3 = Control BDIR/BC1) ---
    if ((a & 0xF000) == 0x2000) {
        if (a == 0x2000 || a == 0x2001) {
            qs.port_a = val;
        } else {
            qs.port_b = val;
            if ((val & 0x05) == 0x05) {
                // BDIR=1, BC1=1 -> Latch Address
                qs.ay_addr = qs.port_a & 0x0F;
            } else if ((val & 0x05) == 0x04) {
                // BDIR=1, BC1=0 -> Write Data
                ayemu_write_reg(qs.ay_addr, qs.port_a);
            }
        }
        return;
    }
}

uint8_t qsound_read_byte(uint32_t addr)
{
    if (!qs.enabled) return 0xFF;

    uint32_t full_addr = addr & 0x00FFFFFF;

    // 1. LECTURA SEGURA DE LA ROM ($0C0000 - $0C1FFF o hasta 16 KB)
    if (qsound_rom_loaded && (full_addr >= 0x000C0000) && (full_addr < (0x000C0000 + qsound_rom_size))) {
        return qsound_rom[full_addr - 0x000C0000];
    }

    // 2. PUERTOS DE HARDWARE ($C2000 y $C3000)
    uint32_t a = addr & 0xFFFF;
    if ((a & 0xF000) == 0x3000) {
        if (a != 0x3000) {
            return (qs.ay_addr < 14) ? qs.regs[qs.ay_addr] : 0xFF;
        }
    } else if ((a & 0xF000) == 0x2000) {
        if (a == 0x2000 || a == 0x2001) {
            if (qs.port_b & 0x01) {
                return (qs.ay_addr < 14) ? qs.regs[qs.ay_addr] : 0xFF;
            }
            return qs.port_a;
        }
    }
    return 0xFF;
}

// -----------------------------------------------------------------------------
//              RENDERIZADOR     EN     16 BITS 
// -----------------------------------------------------------------------------
void qsound_render_mix_s16(int16_t *stream, int len) {
    if (!qs.enabled) return;

    for (int i = 0; i < len; i++) {
        qs.tact_accum += qs.tacts_per_sample;
        int tacts = (int)qs.tact_accum;
        qs.tact_accum -= (double)tacts;

        while (tacts > 0) {
            tacts--;

            // 1. Tono Canal A
            int tone_a = ((qs.regs[1] & 0x0F) << 8) | qs.regs[0];
            if (tone_a == 0) tone_a = 1;
            if (++qs.cnt_a >= tone_a) {
                qs.cnt_a = 0;
                qs.bit_a = !qs.bit_a;
            }

            // 2. Tono Canal B
            int tone_b = ((qs.regs[3] & 0x0F) << 8) | qs.regs[2];
            if (tone_b == 0) tone_b = 1;
            if (++qs.cnt_b >= tone_b) {
                qs.cnt_b = 0;
                qs.bit_b = !qs.bit_b;
            }

            // 3. Tono Canal C
            int tone_c = ((qs.regs[5] & 0x0F) << 8) | qs.regs[4];
            if (tone_c == 0) tone_c = 1;
            if (++qs.cnt_c >= tone_c) {
                qs.cnt_c = 0;
                qs.bit_c = !qs.bit_c;
            }

            // 4. Ruido JT49 (LFSR 17-bit)
            int noise_period = (qs.regs[6] & 0x1F) * 2;
            if (noise_period == 0) noise_period = 2;
            if (++qs.cnt_n >= noise_period) {
                qs.cnt_n = 0;
                uint32_t bit = ((qs.seed ^ (qs.seed >> 3)) & 1);
                qs.seed = ((qs.seed >> 1) | (bit << 16)) & 0x1FFFF;
                qs.bit_n = qs.seed & 1;
            }

            // 5. Envolvente
            int env_period = ((qs.regs[12] << 8) | qs.regs[11]) * 2;
            if (env_period == 0) env_period = 2;
            if (++qs.cnt_e >= env_period) {
                qs.cnt_e = 0;
                ayemu_step_envelope();
            }
        }

        uint8_t r7 = qs.regs[7];
        int out_a = 0, out_b = 0, out_c = 0;

        // Canal A
        if ((qs.bit_a || (r7 & 0x01)) && (qs.bit_n || (r7 & 0x08))) {
            int v = (qs.regs[8] & 0x10) ? qs.env_vol : (qs.regs[8] & 0x0F);
            if (v > 0) out_a = ayemu_dac_table[v * 2 + 1];
        }

        // Canal B
        if ((qs.bit_b || (r7 & 0x02)) && (qs.bit_n || (r7 & 0x10))) {
            int v = (qs.regs[9] & 0x10) ? qs.env_vol : (qs.regs[9] & 0x0F);
            if (v > 0) out_b = ayemu_dac_table[v * 2 + 1];
        }

        // Canal C
        if ((qs.bit_c || (r7 & 0x04)) && (qs.bit_n || (r7 & 0x20))) {
            int v = (qs.regs[10] & 0x10) ? qs.env_vol : (qs.regs[10] & 0x0F);
            if (v > 0) out_c = ayemu_dac_table[v * 2 + 1];
        }

        // Señal escalada a 16 bits
        double raw_signal = (double)(((out_a + out_b + out_c) / 2) * 256);

        // Filtro analógico suave (0.90)
        qs.lpf_state += 0.90 * (raw_signal - qs.lpf_state);

        int32_t mix = (int32_t)stream[i] + (int32_t)qs.lpf_state;
        if (mix > 32767)  mix = 32767;
        if (mix < -32768) mix = -32768;

        stream[i] = (int16_t)mix;
    }
}