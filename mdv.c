/*
 * mdv.c - Sinclair QL Microdrive emulation
 * Translation of mdv.v and zx8302.v from the MiSTer QL core.
 *
 * Timing model:
 * - The tape advances with the emulated 68008 clock cycles (ql_cycles, see
 *   cycles68k.c). ExecuteLoop calls mdv_sync() every 16 instructions and
 *   QLRun also calls it while the CPU is stopped, so the tape never stalls.
 * - Only the selected drive (motor on) moves, as on a real QL. The tape
 *   position is kept while the motor is off.
 *
 * Note on rx_ready:
 * In mdv.v rx_ready is a single tick pulse of the 200 kHz clock (37 CPU
 * cycles) per byte, and the data is read according to bit_cnt at the time
 * of the read. sQLux charges the real cost of every instruction
 * (cycles68k.c) but not the exact time of every bus access within it, and a
 * polling loop iteration may take slightly longer than 37 cycles. The
 * receiver therefore latches the byte when rx_ready is raised (rx_byte) and
 * keeps rx_ready until the CPU reads it or the next byte arrives. After two
 * byte times without valid data (gap or preamble) rx_ready is dropped, so
 * that a stale byte of a block the ROM chose to ignore is never read.
 */

#include <SDL.h>
#include "mdv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Set to 1 to print one line per tape revolution on the console.
#define MDV_DEBUG_REVOLUTIONS 0


// 7500000 / 200000 - 1, as mdv_clk_scaler in mdv.v
#define MDV_CLK_SCALER 36

// Byte times without valid data after which rx_ready is dropped
#define MDV_RX_IDLE_DROP 2

// Gap lengths in 16 bit words (35 in mdv.v), kept separate so that each
// one can be adjusted independently.
#define MDV_GAP_BEFORE_HEADER 35
#define MDV_GAP_BEFORE_DATA   35

// Maximum cycles per mdv_sync call (a safety limit only; it is never
// reached with synchronisation every 16 instructions).
#define MDV_SYNC_CAP 750000

typedef struct {
    uint16_t *data;
    uint32_t mdv_end;
    uint32_t mem_addr;
    bool     present;
    bool     reverse;

    // Registers equivalent to mdv.v
    uint8_t  mdv_bit_cnt;
    uint16_t mdv_data;
    bool     mdv_data_valid;
    bool     mdv_gap;
    uint16_t mdv_gap_cnt;
    bool     mdv_gap_state;
    bool     mdv_gap_active;
    uint8_t  mdv_clk_cnt;

    // Receiver (see the note at the top)
    bool     rx_ready;          // an unread byte is available
    uint8_t  rx_byte;           // byte latched with rx_ready
    uint8_t  rx_last;           // last byte delivered to the CPU
    uint8_t  rx_idle;           // consecutive byte times without valid data
    uint16_t rx_word;           // block word of rx_byte (diagnostics)

    // Per block diagnostics (header or data)
    bool     blk_read;      // the CPU has read part of this block
    uint16_t blk_bytes;     // bytes read from this block
    uint16_t blk_first;     // word of the first byte read
} MDVDrive;

static MDVDrive drives[MDV_MAX_DRIVES];

// ZX8302 registers
static uint8_t mdv_sel = 0;
static uint8_t mctrl = 0;
static uint8_t irq_mask = 0;
static bool    gap_irq = false;
extern void    ql_trigger_gap_interrupt(void);

// Emulated 68008 clock cycles (iexl_general.c)
extern uint64_t ql_cycles;
static uint64_t last_cycles = 0;

#if MDV_DEBUG_REVOLUTIONS
static uint32_t cpu_bytes_read = 0;
// Per revolution statistics: [0] = headers, [1] = data blocks
typedef struct {
    uint32_t blocks;    // blocks with at least one byte read by the CPU
    uint32_t bytes;     // bytes read
    uint32_t full;      // data blocks read almost completely (>= 500 bytes)
    uint32_t skipped;   // unread bytes overwritten by the next one while reading a block
    uint16_t first_min; // word of the first byte read (min/max)
    uint16_t first_max;
    uint16_t skip_min;  // position (bytes already read from the block) of the losses
    uint16_t skip_max;
} MDVStats;
static MDVStats dbg_st[2];

static void dbg_stats_reset(void) {
    memset(dbg_st, 0, sizeof(dbg_st));
    dbg_st[0].first_min = dbg_st[1].first_min = 0xFFFF;
    dbg_st[0].skip_min = dbg_st[1].skip_min = 0xFFFF;
}
static uint64_t dbg_lost_cap = 0;
#endif

static void mdv_reset_tape_state(MDVDrive *d) {
    d->mem_addr = 0;
    d->mdv_gap_cnt = 0;       // words until the gap
    d->mdv_gap_state = 1;     // alternates header / data gap
    d->mdv_gap_active = 1;    // start inside a gap
    d->mdv_gap = 1;
    d->rx_ready = false;
    d->rx_idle = 0;
    d->blk_read = false;
    d->blk_bytes = 0;
}

// Close the statistics of the block that has just ended.
// is_data: 0 = header, 1 = data block
static void mdv_block_end(MDVDrive *d, int is_data) {
#if MDV_DEBUG_REVOLUTIONS
    if (d->blk_read) {
        MDVStats *s = &dbg_st[is_data];
        s->blocks++;
        s->bytes += d->blk_bytes;
        if (is_data && d->blk_bytes >= 500) s->full++;
        if (d->blk_first < s->first_min) s->first_min = d->blk_first;
        if (d->blk_first > s->first_max) s->first_max = d->blk_first;
    }
#else
    (void)is_data;
#endif
    d->blk_read = false;
    d->blk_bytes = 0;
}

void mdv_init(void) {
    memset(drives, 0, sizeof(drives));
    for (int i = 0; i < MDV_MAX_DRIVES; i++) {
        drives[i].data = (uint16_t *)malloc(MDV_MAX_WORDS * sizeof(uint16_t));
    }
    mdv_reset();
}

void mdv_reset(void) {
    last_cycles = ql_cycles;
#if MDV_DEBUG_REVOLUTIONS
    dbg_stats_reset();
#endif
    mdv_sel = 0;
    mctrl = 0;
    irq_mask = 0;
    gap_irq = false;

    for (int i = 0; i < MDV_MAX_DRIVES; i++) {
        mdv_reset_tape_state(&drives[i]);
        drives[i].mdv_data_valid = 0;
        drives[i].mdv_bit_cnt = 0;
        drives[i].mdv_data = 0;
        drives[i].mdv_clk_cnt = 0;
    }
}

int mdv_load(int drive, const char *path) {
    if (drive < 0 || drive >= MDV_MAX_DRIVES || !path) return -1;

    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0 || size > (long)(MDV_MAX_WORDS * 2)) {
        fclose(f);
        return -1;
    }

    uint8_t *raw_buf = (uint8_t *)malloc(size);
    if (!raw_buf) {
        fclose(f);
        return -1;
    }

    if (fread(raw_buf, 1, size, f) != (size_t)size) {
        free(raw_buf);
        fclose(f);
        return -1;
    }
    fclose(f);

    uint32_t words = (uint32_t)(size / 2);
    for (uint32_t i = 0; i < words; i++) {
        drives[drive].data[i] = ((uint16_t)raw_buf[i * 2] << 8) | raw_buf[i * 2 + 1];
    }
    free(raw_buf);

    drives[drive].mdv_end = words - 1;
    drives[drive].present = true;
    mdv_reset_tape_state(&drives[drive]);

    printf("[MDV] Drive %d loaded: '%s' (%u words / %ld bytes)\n",
           drive + 1, path, words, size);
    return 0;
}

void mdv_unload(int drive) {
    if (drive >= 0 && drive < MDV_MAX_DRIVES) {
        drives[drive].present = false;
        drives[drive].mdv_end = 0;
        drives[drive].rx_ready = false;
    }
}

void mdv_set_reverse(int drive, bool reverse) {
    if (drive >= 0 && drive < MDV_MAX_DRIVES) {
        drives[drive].reverse = reverse;
    }
}

int mdv_is_selected(void) {
    return (mdv_sel != 0);
}

int mdv_any_present(void) {
    for (int i = 0; i < MDV_MAX_DRIVES; i++) {
        if (drives[i].present) return 1;
    }
    return 0;
}

static MDVDrive *mdv_selected_drive(void) {
    if (mdv_sel & 0x01) return &drives[0];
    if (mdv_sel & 0x02) return &drives[1];
    return NULL;
}

// -----------------------------------------------------------------------------
//        STATE MACHINE OF mdv.v (always @(posedge clk) if (ce))
//        One call = one 7.5 MHz CPU cycle
//
//        'selected' is 'sel' in mdv.v: it does not affect the tape
//        movement, only the outputs to the ZX8302 (gap IRQ and rx_ready).
// -----------------------------------------------------------------------------

static void mdv_step_ce(MDVDrive *d, int drive_index, bool selected) {
    if (d->mdv_clk_cnt == MDV_CLK_SCALER) {
        d->mdv_clk_cnt = 0;
    } else {
        d->mdv_clk_cnt++;
    }

    if (d->mdv_clk_cnt) return;

    // --- One tick of the 200 kHz clock ---
    d->mdv_bit_cnt = (d->mdv_bit_cnt + 1) & 0x0F;

    // rx_ready: in mdv.v it is sel && (bit_cnt[2:0] == 2) && data_valid for
    // this tick only. Here the byte is latched and rx_ready is held until
    // the CPU reads it or the next byte arrives.
    uint8_t phase = d->mdv_bit_cnt & 0x07;
    if (phase == 2) {
        if (d->mdv_data_valid && selected) {
            d->rx_idle = 0;
#if MDV_DEBUG_REVOLUTIONS
            if (d->rx_ready && d->blk_read) {
                // the previous byte was not read in time
                MDVStats *s = &dbg_st[d->mdv_gap_state];
                s->skipped++;
                if (d->blk_bytes < s->skip_min) s->skip_min = d->blk_bytes;
                if (d->blk_bytes > s->skip_max) s->skip_max = d->blk_bytes;
            }
#endif
            // dout = bit_cnt[3] ? data[7:0] : data[15:8], as in mdv.v
            d->rx_byte = (d->mdv_bit_cnt & 0x08) ? (uint8_t)(d->mdv_data & 0xFF)
                                                 : (uint8_t)(d->mdv_data >> 8);
            d->rx_word = d->mdv_gap_cnt;
            d->rx_ready = true;
        } else if (d->rx_ready && ++d->rx_idle >= MDV_RX_IDLE_DROP) {
            // Gap or preamble: drop rx_ready for a stale byte
            d->rx_ready = false;
            d->rx_idle = 0;
        }
    }

    if (d->mdv_bit_cnt != 15) return;

    // --- End of word: load the next one ---
    uint16_t mdv_din = (d->mem_addr <= d->mdv_end) ? d->data[d->mem_addr] : 0;
    bool next_data_valid = !d->mdv_gap_active && (d->mdv_gap_cnt > 5) &&
                           !(d->mdv_gap_state && (d->mdv_gap_cnt > 7) && (d->mdv_gap_cnt < 12));

    if (d->mem_addr > d->mdv_end) {
        // End of the tape loop: back to the start
        mdv_reset_tape_state(d);

#if MDV_DEBUG_REVOLUTIONS
        static uint32_t last_rev_time[MDV_MAX_DRIVES] = {0};
        static int rev_count[MDV_MAX_DRIVES] = {0};
        uint32_t now = SDL_GetTicks();
        uint32_t rev_ms = (last_rev_time[drive_index] == 0) ? 0 : (now - last_rev_time[drive_index]);
        last_rev_time[drive_index] = now;
        rev_count[drive_index]++;
        printf("[MDV%d REV %d] %u ms | bytes %u | "
               "HDR n=%u bytes=%u skip=%u@%u..%u | "
               "DAT n=%u bytes=%u full=%u skip=%u@%u..%u | cap %llu M\n",
               drive_index + 1, rev_count[drive_index], rev_ms, cpu_bytes_read,
               dbg_st[0].blocks, dbg_st[0].bytes, dbg_st[0].skipped,
               dbg_st[0].skipped ? dbg_st[0].skip_min : 0, dbg_st[0].skip_max,
               dbg_st[1].blocks, dbg_st[1].bytes, dbg_st[1].full, dbg_st[1].skipped,
               dbg_st[1].skipped ? dbg_st[1].skip_min : 0, dbg_st[1].skip_max,
               (unsigned long long)(dbg_lost_cap / 1000000));
        fflush(stdout);
        cpu_bytes_read = 0;
        dbg_stats_reset();
        dbg_lost_cap = 0;
#endif
    } else {
        uint16_t old_gap_cnt = d->mdv_gap_cnt;
        d->mdv_gap_cnt = (old_gap_cnt + 1) & 0x3FF;

        if (d->mdv_gap_active) {
            // End of the gap. mdv_gap_state tells what follows:
            // 1 -> header, 0 -> data block (toggled on exit).
            uint16_t gap_len = d->mdv_gap_state ? MDV_GAP_BEFORE_HEADER
                                                : MDV_GAP_BEFORE_DATA;
            if (old_gap_cnt == gap_len - 1) {
                d->mdv_gap_cnt = 0;
                d->mdv_gap_active = 0;
                d->mdv_gap_state = !d->mdv_gap_state;
                d->mdv_gap = 0;
            }
        } else {
            d->mem_addr++;

            if (!d->mdv_gap_state && old_gap_cnt == 13) {
                // End of the 14 header words: a gap starts
                d->mdv_gap_cnt = 0;
                d->mdv_gap_active = 1;
                d->mdv_gap = 1;
                mdv_block_end(d, 0);   // end of header

                if (selected && (irq_mask & 0x01)) {
                    gap_irq = true;
                    ql_trigger_gap_interrupt();
                }
            } else if (d->mdv_gap_state && old_gap_cnt == 328) {
                // End of the data block words: a gap starts
                d->mdv_gap_cnt = 0;
                d->mdv_gap_active = 1;
                d->mdv_gap = 1;
                mdv_block_end(d, 1);   // end of data block

                if (selected && (irq_mask & 0x01)) {
                    gap_irq = true;
                    ql_trigger_gap_interrupt();
                }

                if (d->reverse) {
                    if (d->mem_addr == 343)
                        d->mem_addr = d->mdv_end - 343 + 1;
                    else
                        d->mem_addr = d->mem_addr - 686 + 1;
                }
            }
        }
    }

    d->mdv_data = mdv_din;
    d->mdv_data_valid = next_data_valid;
}

// Only the selected drive (motor on) moves, as on a real QL.
void mdv_tick(int cpu_cycles) {
    MDVDrive *d = mdv_selected_drive();
    if (!d || !d->present) return;

    int n = (int)(d - drives);
    for (int i = 0; i < cpu_cycles; i++) {
        mdv_step_ce(d, n, true);
    }
}

// Bring the microdrive up to date with the cycles emulated since the last
// call. Called from ExecuteLoop (every 16 instructions), from QLRun (while
// the CPU is stopped) and before every access to $18020-$18023.
void mdv_sync(void) {
    uint64_t current = ql_cycles;
    uint64_t cycles = current - last_cycles;
    last_cycles = current;
    if (cycles == 0) return;

    if (cycles > MDV_SYNC_CAP) {
#if MDV_DEBUG_REVOLUTIONS
        dbg_lost_cap += cycles - MDV_SYNC_CAP;
#endif
        cycles = MDV_SYNC_CAP;
    }
    mdv_tick((int)cycles);
}

// -----------------------------------------------------------------------------
//                  ZX8302 REGISTER INTERFACE
// -----------------------------------------------------------------------------

void mdv_write_control(uint8_t val) {
    uint8_t old_clk = (mctrl >> 1) & 0x01;
    uint8_t new_clk = (val >> 1) & 0x01;
    uint8_t data_bit = val & 0x01;
    mctrl = val;

    if (old_clk && !new_clk) {
        mdv_sel = ((mdv_sel << 1) | data_bit) & 0xFF;
    }
}

void mdv_write_int(uint8_t val) {
    irq_mask = (val >> 5) & 0x07;
    if (val & 0x01) {
        gap_irq = false;
    }
}

// $18020: {comdata_to_cpu, ipc_busy, 0, 0, gap, rx_ready, tx_empty, 0}
uint8_t mdv_read_status(void) {
    MDVDrive *d = mdv_selected_drive();

    bool mdv_present = d && d->present && (d->mdv_end != 0);
    bool gap = !mdv_present || d->mdv_gap;
    bool rx_ready = mdv_present && d->rx_ready;

    uint8_t status = 0x80;                    // comdata_to_cpu idle
    if (gap)      status |= (1 << 3);
    if (rx_ready) status |= (1 << 2);

    return status;
}

uint8_t mdv_read_int(void) {
    return gap_irq ? 0x01 : 0x00;
}

// $18022/$18023: both return the same byte ({mdv_byte, mdv_byte} in zx8302.v)
uint8_t mdv_read_data(void) {
    MDVDrive *d = mdv_selected_drive();
    if (!d || !d->present) return 0xFF;

#if MDV_DEBUG_REVOLUTIONS
    cpu_bytes_read++;
#endif

    if (!d->rx_ready) {
        // Read without rx_ready: repeat the last byte delivered
        return d->rx_last;
    }

#if MDV_DEBUG_REVOLUTIONS
    if (!d->mdv_gap_active) {
        if (!d->blk_read) {
            d->blk_read = true;
            d->blk_first = d->rx_word;
        }
        d->blk_bytes++;
    }
#endif

    d->rx_ready = false;
    d->rx_last = d->rx_byte;
    return d->rx_last;
}