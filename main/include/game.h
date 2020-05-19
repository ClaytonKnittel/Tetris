#ifndef _GAME_H
#define _GAME_H

#include <gl/gl.h>
#include <gl/shader.h>

#include <tetris.h>
#include <frame.h>
#include <hold.h>
#include <score.h>
#include <up_next.h>


/*
 * flags determining which components of the tetris game are to be
 * displayed
 */
#define SHOW_FRAME 0x1
#define SHOW_SCORE 0x2
#define SHOW_UP_NEXT 0x4
#define SHOW_HOLD 0x8

#define SHOW_ALL (SHOW_FRAME | SHOW_SCORE | SHOW_UP_NEXT | SHOW_HOLD)

typedef struct game {
    tetris_t t;
    frame_t f;
    font_t *font;
    scoreboard sb;
    up_next_t u;
    hold_t h;

    int flags;
} game_t;


int game_init(game_t *g, int flags, gl_context *c, font_t *font);

void game_destroy(game_t *g);

void game_tick(game_t *g);

void game_render(game_t *g);


#endif /* _GAME_H */
