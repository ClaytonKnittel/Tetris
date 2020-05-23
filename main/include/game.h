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


/*
 * flags determining how the game is to be played
 */

// calls callback each frame and does not setup key listeners (used for AI to
// play instead of human). If this flag is not set, then the game is controlled
// with the keyboard keys
#define MANUAL_CONTROL 0x10


typedef struct game {
    tetris_t t;
    frame_t f;
    font_t *font;
    scoreboard sb;
    up_next_t u;
    hold_t h;

    int flags;

    /*
     * if MANUAL_CONTROL is set, this function is to be called each game tick,
     * with the game as the first argument and ctrl_arg as the second
     */
    void (*ctrl_callback)(tetris_t*, void*);
    void *ctrl_arg;
} game_t;


int game_init(game_t *g, int flags, gl_context *c, font_t *font);

void game_destroy(game_t *g);

// initializes control callback function to be made each game tick when manual
// controls are enabled
void game_set_ctrl_callback(game_t *g, void (*ctrl_cb)(tetris_t *t, void*),
        void *ctrl_arg);



/*
 * places given keypress in event queue of the tetris game
 */
void game_press(game_t *g, int key);


/*
 * calculates a single game tick, advancing time by 1 tick
 */
void game_tick(game_t *g);

/*
 * draws all components of the game to the screen
 */
void game_render(game_t *g);


#endif /* _GAME_H */
