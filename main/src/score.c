
#include <stdio.h>

#include <score.h>


#define SCORE_STR_LEN 9


// height of score text, relative to score box of height 1
#define SCORE_TEXT_H .45f
// height of score number, relative to score box of height 1
#define SCORE_NUM_H .35f

// height of padding between score text and score number
#define SCORE_PAD_H (1.f - SCORE_TEXT_H - SCORE_NUM_H)


int scoreboard_init(scoreboard *s, font_t *f, float x, float y, float w, float h) {

    //gl_load_program(&s->p, "", "");

    //shape_init(&s->base, &s->p);

    s->score = 0;

    s->f = f;

    shape_set_pos(&s->base, x, y);
    shape_set_xscale(&s->base, w);
    shape_set_yscale(&s->base, h);

    return 0;
}


void scoreboard_destroy(scoreboard *s) {
    return;
}


void scoreboard_set_score(scoreboard *s, uint32_t score) {
    s->score = score;
}


/*
 * draws scoreboard to the screen. Layout is
 *
 *                  s->base.xscale
 *               +--------------------+  +
 *               |                    |  |
 * SCORE_TEXT_H  | SCORE              |  |
 *               |                    |  |
 *               +--------------------+  |
 * SCORE_PAD_H   |                    |  | s->base.yscale
 *               +--------------------+  |
 *               |                    |  |
 * SCORE_NUM_H   | 1234...            |  |
 *               |                    |  |
 *               +--------------------+  +
 *
 */
void scoreboard_draw(scoreboard *s) {
    char buf[SCORE_STR_LEN + 1];

    font_set_color(s->f, gen_color(255, 255, 255, 255));
    font_render(s->f, "score", s->base.pos.x,
            s->base.pos.y + s->base.yscale * (SCORE_NUM_H + SCORE_PAD_H),
            s->base.xscale, s->base.yscale * SCORE_TEXT_H);

    snprintf(buf, SCORE_STR_LEN + 1, "%*u", SCORE_STR_LEN, s->score);

    font_set_color(s->f, gen_color(255, 255, 255, 255));
    font_render_mono_num(s->f, buf, s->base.pos.x, s->base.pos.y, s->base.xscale,
            s->base.yscale * SCORE_NUM_H);
}


