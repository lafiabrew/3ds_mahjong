#include "mahjong.h"
#include "pk_platform.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    char sd_root[512];
    mahj_state_t st;
    int cursor_idx;
    uint32_t seed;
    char status_msg[128];
} mahj_app_t;

static void app_init(mahj_app_t *app, const char *sd_root) {
    memset(app, 0, sizeof(*app));
    snprintf(app->sd_root, sizeof(app->sd_root), "%s",
             sd_root != NULL ? sd_root : pk_platform_default_sd_root());
    app->seed = 0x2026u;
    mahj_init(&app->st, app->seed);
    snprintf(app->status_msg, sizeof(app->status_msg),
             "MAHJONG 3DS | A:SELECT FREE TILE X:AUTO-HINT Y:SHUFFLE");
    pk_log_event(app->sd_root, "mahjong", "INFO", app->status_msg);
}

static void app_step(mahj_app_t *app, const pk_input_t *in) {
    if (in->down & PK_BTN_DLEFT)  app->cursor_idx = (app->cursor_idx + MAHJ_MAX_TILES - 1) % MAHJ_MAX_TILES;
    if (in->down & PK_BTN_DRIGHT) app->cursor_idx = (app->cursor_idx + 1) % MAHJ_MAX_TILES;
    if (in->down & PK_BTN_DUP)    app->cursor_idx = (app->cursor_idx + MAHJ_MAX_TILES - 8) % MAHJ_MAX_TILES;
    if (in->down & PK_BTN_DDOWN)  app->cursor_idx = (app->cursor_idx + 8) % MAHJ_MAX_TILES;

    if (in->down & PK_BTN_A) {
        if (!mahj_is_tile_free(&app->st, app->cursor_idx)) {
            snprintf(app->status_msg, sizeof(app->status_msg), "TILE BLOCKED BY NEIGHBORS OR UPPER LAYER");
        } else if (app->st.selected_idx < 0) {
            app->st.selected_idx = app->cursor_idx;
            snprintf(app->status_msg, sizeof(app->status_msg),
                     "SELECTED TILE [%s] - PICK MATCHING FREE TILE",
                     mahj_tile_label(app->st.tiles[app->cursor_idx].kind));
        } else {
            if (mahj_try_match(&app->st, app->st.selected_idx, app->cursor_idx)) {
                snprintf(app->status_msg, sizeof(app->status_msg),
                         "MATCHED PAIR! REMAINING:%d SCORE:%u",
                         app->st.remaining, (unsigned)app->st.score);
            } else {
                snprintf(app->status_msg, sizeof(app->status_msg), "TILES DO NOT MATCH");
            }
            app->st.selected_idx = -1;
        }
    }
    if (in->down & PK_BTN_X) {
        int a = -1, b = -1;
        if (mahj_find_hint(&app->st, &a, &b) && mahj_try_match(&app->st, a, b)) {
            snprintf(app->status_msg, sizeof(app->status_msg),
                     "HINT MATCHED [%s]! REMAINING:%d SCORE:%u",
                     mahj_tile_label(app->st.tiles[a].kind),
                     app->st.remaining, (unsigned)app->st.score);
        }
    }
    if (in->down & PK_BTN_Y) {
        app->seed += 13u;
        mahj_init(&app->st, app->seed);
        snprintf(app->status_msg, sizeof(app->status_msg), "NEW MAHJONG PYRAMID DEALT");
    }
}

static void app_render(const mahj_app_t *app) {
    pk_render_begin_frame();
    pk_render_begin_target(PK_TARGET_TOP_LEFT, PK_RGBA8(14, 34, 28, 255));
    pk_draw_rect(0, 0, 400, 26, PK_RGBA8(22, 62, 48, 255));
    pk_draw_text(8, 7, 0.48f, PK_RGBA8(255, 225, 100, 255),
                 "MAHJONG 3DS - 3D MULTI-LAYER SOLITAIRE");

    char line[128];
    snprintf(line, sizeof(line), "TILES LEFT: %2d / 36  |  MATCHES: %u  |  SCORE: %u",
             app->st.remaining, (unsigned)app->st.matches, (unsigned)app->st.score);
    pk_draw_text(10, 34, 0.45f, PK_RGBA8(190, 250, 220, 255), line);

    for (int layer = 0; layer <= 1; layer++) {
        for (int i = 0; i < app->st.tile_count; i++) {
            const mahj_tile_t *t = &app->st.tiles[i];
            if (!t->present || t->layer != layer) continue;
            float tx = 44.0f + (float)t->gx * 38.0f - (float)layer * 4.0f;
            float ty = 58.0f + (float)t->gy * 36.0f - (float)layer * 4.0f;
            bool free_t = mahj_is_tile_free(&app->st, i);
            uint32_t bg = (layer == 1)
                              ? PK_RGBA8(255, 245, 210, 255)
                              : (free_t ? PK_RGBA8(238, 242, 232, 255) : PK_RGBA8(155, 170, 162, 255));
            pk_draw_rect(tx + 2.0f, ty + 3.0f, 34.0f, 30.0f, PK_RGBA8(20, 50, 38, 255));
            pk_draw_rect(tx, ty, 34.0f, 30.0f, bg);
            if (i == app->cursor_idx || i == app->st.selected_idx) {
                pk_draw_rect_outline(tx, ty, 34.0f, 30.0f, 2.0f, PK_RGBA8(255, 110, 75, 255));
            } else if (i == app->st.hint_a || i == app->st.hint_b) {
                pk_draw_rect_outline(tx, ty, 34.0f, 30.0f, 1.5f, PK_RGBA8(65, 210, 125, 255));
            }
            pk_draw_text(tx + 7.0f, ty + 9.0f, 0.44f, PK_RGBA8(24, 42, 34, 255),
                         mahj_tile_label(t->kind));
        }
    }

    pk_draw_rect(0, 216, 400, 24, PK_RGBA8(18, 48, 38, 255));
    pk_draw_text(8, 222, 0.44f, PK_RGBA8(120, 255, 175, 255), app->status_msg);

    pk_render_begin_target(PK_TARGET_BOTTOM, PK_RGBA8(12, 28, 24, 255));
    pk_draw_rect(14, 18, 292, 204, PK_RGBA8(20, 44, 36, 255));
    pk_draw_text(24, 32, 0.50f, PK_RGBA8(255, 225, 100, 255), "MAHJONG SOLITAIRE RULES:");
    pk_draw_text(24, 58, 0.44f, PK_RGBA8(215, 245, 230, 255), "1. A TILE IS FREE IF NO TILE IS ABOVE IT");
    pk_draw_text(24, 78, 0.44f, PK_RGBA8(215, 245, 230, 255), "   AND LEFT OR RIGHT SIDE IS UNBLOCKED.");
    pk_draw_text(24, 108, 0.46f, PK_RGBA8(130, 250, 175, 255), "A: SELECT / MATCH FREE PAIR");
    pk_draw_text(24, 132, 0.46f, PK_RGBA8(135, 225, 255, 255), "X: AUTO-MATCH HINT PAIR");
    pk_draw_text(24, 156, 0.46f, PK_RGBA8(255, 190, 120, 255), "Y: SHUFFLE NEW PYRAMID");
    pk_draw_text(24, 190, 0.44f, PK_RGBA8(180, 205, 195, 255), "PRESS START TO EXIT");
    pk_render_end_frame();
}

int main(int argc, char **argv) {
    const char *sd_root = NULL;
    bool one_shot = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--sd-root") == 0 && i + 1 < argc) sd_root = argv[++i];
        else if (strcmp(argv[i], "--once") == 0) one_shot = true;
    }
    if (!pk_platform_init()) return 1;
    mahj_app_t app;
    app_init(&app, sd_root);
    while (pk_platform_main_loop()) {
        pk_input_t in;
        pk_platform_poll_input(&in);
        if (in.down & PK_BTN_START) break;
        app_step(&app, &in);
        app_render(&app);
        if (one_shot) break;
    }
    pk_platform_shutdown();
    return 0;
}
