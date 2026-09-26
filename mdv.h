/*
 * mdv.h - Sinclair QL Microdrive Hardware Emulation
 */

#ifndef MDV_H
#define MDV_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MDV_MAX_DRIVES     2
#define MDV_WORDS_PER_SEC  343
#define MDV_TOTAL_SECTORS  255
#define MDV_MAX_WORDS      88000

// Core lifecycle
void    mdv_init(void);
void    mdv_reset(void);
int     mdv_load(int drive, const char *path);
void    mdv_unload(int drive);
void    mdv_set_reverse(int drive, bool reverse);
int     mdv_is_selected(void);

// Clock & hardware registers
void    mdv_tick(int cpu_cycles);
void    mdv_sync(void);
void    mdv_write_control(uint8_t val); // $18020 write
void    mdv_write_int(uint8_t val);     // $18021 write
uint8_t mdv_read_status(void);          // $18020 read
uint8_t mdv_read_int(void);             // $18021 read
uint8_t mdv_read_data(void);            // $18022 / $18023 read
int     mdv_any_present(void);

#ifdef __cplusplus
}
#endif

#endif // MDV_H
