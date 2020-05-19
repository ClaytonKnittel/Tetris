
#include <gl/gl.h>

#include <tutil.h>
#include <game.h>

extern const float aspect_ratio;



static void _default_key_event(gl_context *context, int key, int scancode,
        int action, int mods) {

    game_t *g = (game_t*) context->user_data;
    tetris_t *t = &g->t;

    key_event e = {
        .key = key,
        .scancode = scancode,
        .action = action,
        .mods = mods
    };
    key_event_queue_push(&t->kq, &e);
}


int game_init(game_t *g, int flags, gl_context *c, font_t *font) {
    tetris_t *t = &g->t;
    frame_t *f = &g->f;
    scoreboard *sb = &g->sb;
    up_next_t *u = &g->u;
    hold_t *h = &g->h;

    g->flags = flags;

    float w = 2.f * aspect_ratio * ((float) TETRIS_WIDTH) /
        ((float) TETRIS_HEIGHT);
    float x = -w / 2.f;
    float y = -1.f;
    tetris_init(t, c, x, y, w, 2.f);

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

    if (flags & MANUAL_CONTROL) {
        g->ctrl_callback = NULL;
    }
    else {
        gl_register_key_callback(c, &_default_key_event);
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

void game_set_ctrl_callback(game_t *g,
        void (*ctrl_cb)(game_t*, board_t*, void*),
        void *ctrl_arg) {

    // the callback is only called when manual controls are enabled
    TETRIS_ASSERT(g->flags & MANUAL_CONTROL);

    g->ctrl_callback = ctrl_cb;
    g->ctrl_arg = ctrl_arg;
}


// emulates a single button press by pushing both the press and release action
// on the same key to the queue
void game_press(game_t *g, int key) {

    key_event e = {
        .key = key,
        .scancode = 0,
        .action = GLFW_PRESS,
        .mods = 0
    };
    key_event_queue_push(&g->t.kq, &e);

    e.action = GLFW_RELEASE;
    key_event_queue_push(&g->t.kq, &e);
}



void game_tick(game_t *g) {
    tetris_step(&g->t);

    if (g->flags & MANUAL_CONTROL) {
        TETRIS_ASSERT(g->ctrl_callback != NULL);
        g->ctrl_callback(g, &g->t.board, g->ctrl_arg);
    }
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


