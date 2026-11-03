#include "mahjong.h"

#include <string.h>

static const char *const k_labels[18] = {
    "1B", "2B", "3B", "4B", "5B", "6B", "7B", "8B", "9B",
    "1D", "2D", "3D", "4D", "EW", "SW", "WW", "NW", "RD"
};

const char *mahj_tile_label(uint8_t kind) {
    return k_labels[kind % 18u];
}

void mahj_init(mahj_state_t *st, uint32_t seed) {
    if (st == NULL) return;
    memset(st, 0, sizeof(*st));
    st->selected_idx = -1;
    st->hint_a = -1;
    st->hint_b = -1;

    /* 36 tiles = 18 pairs:
       - Base layer (layer 0): 8x4 = 32 tiles
       - Top pyramid layer (layer 1): 2x2 = 4 tiles centered at gx=3..4, gy=1..2 */
    uint8_t deck[MAHJ_MAX_TILES];
    for (int i = 0; i < MAHJ_MAX_TILES; i++) {
        deck[i] = (uint8_t)((i / 2) % 18);
    }
    uint32_t rng = seed ? seed : 0x3D5BA3B0u;
    for (int i = MAHJ_MAX_TILES - 1; i > 0; i--) {
        rng = rng * 1664525u + 1013904223u;
        int j = (int)((rng >> 16) % (uint32_t)(i + 1));
        uint8_t tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }

    int idx = 0;
    for (uint8_t y = 0; y < 4; y++) {
        for (uint8_t x = 0; x < 8; x++) {
            st->tiles[idx].kind = deck[idx];
            st->tiles[idx].gx = x;
            st->tiles[idx].gy = y;
            st->tiles[idx].layer = 0;
            st->tiles[idx].present = true;
            idx++;
        }
    }
    for (uint8_t y = 1; y <= 2; y++) {
        for (uint8_t x = 3; x <= 4; x++) {
            st->tiles[idx].kind = deck[idx];
            st->tiles[idx].gx = x;
            st->tiles[idx].gy = y;
            st->tiles[idx].layer = 1;
            st->tiles[idx].present = true;
            idx++;
        }
    }
    st->tile_count = MAHJ_MAX_TILES;
    st->remaining = MAHJ_MAX_TILES;
    mahj_find_hint(st, &st->hint_a, &st->hint_b);
}

bool mahj_is_tile_free(const mahj_state_t *st, int idx) {
    if (st == NULL || idx < 0 || idx >= st->tile_count || !st->tiles[idx].present) {
        return false;
    }
    const mahj_tile_t *t = &st->tiles[idx];
    bool blocked_left = false;
    bool blocked_right = false;

    for (int i = 0; i < st->tile_count; i++) {
        if (i == idx || !st->tiles[i].present) continue;
        const mahj_tile_t *other = &st->tiles[i];
        /* Covered from above on same (gx, gy)? */
        if (other->layer > t->layer && other->gx == t->gx && other->gy == t->gy) {
            return false;
        }
        if (other->layer == t->layer && other->gy == t->gy) {
            if (other->gx + 1 == t->gx) blocked_left = true;
            if (other->gx == t->gx + 1) blocked_right = true;
        }
    }
    return !(blocked_left && blocked_right);
}

bool mahj_try_match(mahj_state_t *st, int idx_a, int idx_b) {
    if (st == NULL || idx_a == idx_b) return false;
    if (!mahj_is_tile_free(st, idx_a) || !mahj_is_tile_free(st, idx_b)) {
        return false;
    }
    if (st->tiles[idx_a].kind != st->tiles[idx_b].kind) {
        return false;
    }
    st->tiles[idx_a].present = false;
    st->tiles[idx_b].present = false;
    st->remaining -= 2;
    st->matches++;
    st->score += 100u + (uint32_t)(st->tiles[idx_a].layer * 50u);
    st->cleared = (st->remaining <= 0);
    mahj_find_hint(st, &st->hint_a, &st->hint_b);
    return true;
}

bool mahj_find_hint(mahj_state_t *st, int *out_a, int *out_b) {
    if (st == NULL) return false;
    for (int i = 0; i < st->tile_count; i++) {
        if (!mahj_is_tile_free(st, i)) continue;
        for (int j = i + 1; j < st->tile_count; j++) {
            if (!mahj_is_tile_free(st, j)) continue;
            if (st->tiles[i].kind == st->tiles[j].kind) {
                if (out_a) *out_a = i;
                if (out_b) *out_b = j;
                st->hint_a = i;
                st->hint_b = j;
                return true;
            }
        }
    }
    if (out_a) *out_a = -1;
    if (out_b) *out_b = -1;
    st->hint_a = -1;
    st->hint_b = -1;
    return false;
}
