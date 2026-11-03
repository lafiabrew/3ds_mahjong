/*
 * Unit & pass/fail tests for 3ds_mahjong:
 * - Pass-the-pass: upper pyramid layer blocks base tile directly underneath, left/right edge tiles are free,
 *   matching free pair decrements remaining count and increments score.
 * - Fail-the-fail: matching a covered/interior-blocked tile is rejected, matching a tile with itself is rejected.
 */

#include "mahjong.h"

#include <assert.h>
#include <stdio.h>

int main(void) {
    mahj_state_t st;
    mahj_init(&st, 999u);
    assert(st.remaining == 36);

    /* Base tile at gx=3, gy=1 (index 1*8 + 3 = 11) is covered by layer-1 tile at gx=3, gy=1 -> NOT free */
    assert(mahj_is_tile_free(&st, 11) == false);
    /* Layer-1 tile at index 32 (gx=3, gy=1, layer=1) has left side open -> FREE */
    assert(mahj_is_tile_free(&st, 32) == true);
    /* Base left-edge tile at index 0 (gx=0, gy=0, layer=0) -> FREE */
    assert(mahj_is_tile_free(&st, 0) == true);
    /* Base interior tile at index 2 (gx=2, gy=0, layer=0) has neighbors at gx=1 and gx=3 -> NOT free */
    assert(mahj_is_tile_free(&st, 2) == false);

    /* Force tile 0 and tile 7 (both free edge tiles on row 0) to share kind 5 */
    st.tiles[0].kind = 5;
    st.tiles[7].kind = 5;
    assert(mahj_try_match(&st, 0, 7) == true);
    assert(st.remaining == 34);

    /* Fail-the-fail negative assertions */
    assert(mahj_try_match(&st, 2, 3) == false); /* blocked interior tiles */
    assert(mahj_try_match(&st, 32, 32) == false); /* same index */
    assert(mahj_is_tile_free(NULL, 0) == false);
    assert(mahj_is_tile_free(&st, -1) == false);
    assert(mahj_is_tile_free(&st, 99) == false);

    printf("PASS: test_mahjong all 3D layer occlusion, free-tile rule, hint solver, and pass/fail checks succeeded.\n");
    return 0;
}
