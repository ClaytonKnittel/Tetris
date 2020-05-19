
#include <game.h>

extern const float aspect_ratio;


int game_init(game_t *g, int flags, gl_context *c, font_t *font) {
    tetris_t *t = &g->t;
    frame_t *f = &g->f;
    scoreboard *sb = &g->sb;
    up_next_t *u = &g->u;
    hold_t *h = &g->h;

    g->flags = flags;

    float w = 2.f * aspect_ratio * ((float) TETRIS_WIDTH) /
        ((float) TETRIS_HEIGHT);
    vec2 pos = {
        .x = -w / 2.f,
        .y = -1.f
    };
    tetris_init(t, c, pos, w, 2.f);

    if (flags & SHOW_FRAME) {
        frame_init(f, TETRIS_WIDTH, TETRIS_HEIGHT);
        frame_set_pos(f, -w / 2.f, -1.f);
        frame_set_xscale(f, w);
        frame_set_yscale(f, 2.f);
    }

    if (flags & SHOW_SCORE) {
        scoreboard_init(sb, font, .5f, .45f, .5f, .15f);
    }

    if (flags & SHOW_UP_NEXT) {
        up_next_init(u, 3, .55f, -.8f, .28f, .9f, font, t);
    }

    if (flags & SHOW_HOLD) {
        hold_init(h, -.8f, .45f, .28f, .5f, font, t);
    }

    return 0;
}

void game_destroy(game_t *g) {
    int flags = g->flags;

    if (flags & SHOW_HOLD) {
        hold_destroy(&g->h);
    }
    if (flags & SHOW_UP_NEXT) {
        up_next_destroy(&g->u);
    }
    if (flags & SHOW_SCORE) {
        scoreboard_destroy(&g->sb);
    }
    if (flags & SHOW_FRAME) {
        frame_destroy(&g->f);
    }
    tetris_destroy(&g->t);
}

void game_tick(game_t *g) {
    tetris_step(&g->t);
}

void game_render(game_t *g) {
    int flags = g->flags;

    board_draw(&g->t.board);
    if (flags & SHOW_FRAME) {
        frame_draw(&g->f);
    }
    if (flags & SHOW_UP_NEXT) {
        up_next_draw(&g->u);
    }
    if (flags & SHOW_HOLD) {
        hold_draw(&g->h);
    }

    if (flags & SHOW_SCORE) {
        scoreboard_set_score(&g->sb, g->t.scorer.score);
        scoreboard_draw(&g->sb);
    }
}


