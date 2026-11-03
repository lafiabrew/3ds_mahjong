#ifndef MAHJONG_H
#define MAHJONG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAHJ_MAX_TILES 36

typedef struct {
    uint8_t kind;    /* 0..17 tile faces (Bamboo 1-9, Dots 1-4, Winds E/S/W/N, Red Dragon) */
    uint8_t gx;      /* grid col 0..7 */
    uint8_t gy;      /* grid row 0..3 */
    uint8_t layer;   /* 0=base, 1=upper pyramid */
    bool present;
} mahj_tile_t;

typedef struct {
    mahj_tile_t tiles[MAHJ_MAX_TILES];
    int tile_count;
    int remaining;
    int selected_idx;
    int hint_a;
    int hint_b;
    uint32_t matches;
    uint32_t score;
    bool cleared;
} mahj_state_t;

void mahj_init(mahj_state_t *st, uint32_t seed);
bool mahj_is_tile_free(const mahj_state_t *st, int idx);
bool mahj_try_match(mahj_state_t *st, int idx_a, int idx_b);
bool mahj_find_hint(mahj_state_t *st, int *out_a, int *out_b);
const char *mahj_tile_label(uint8_t kind);

#endif /* MAHJONG_H */
