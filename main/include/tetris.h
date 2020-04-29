#ifndef _TETRIS_H
#define _TETRIS_H

#include <math/vec2.h>
#include <gl/gl.h>

#include <board.h>
#include <key_event.h>
#include <piece.h>


#define TETRIS_WIDTH 10
#define TETRIS_HEIGHT 20


/*
 * game states
 */
#define PLAY 0
#define PAUSED 1
#define GAME_OVER 2



/*
 * falling piece flags
 */
#define FAST_FALLING 0x1
#define HIT_GROUND_LAST_FRAME 0x2



typedef struct tetris {
    // piece currently being controlled by player
    piece_t falling_piece;

    /*
     * bitvector of status flags that apply to the falling piece. Possible
     * statuses are:
     *  FAST_FALLING: when set, the piece drops at twice the rate it normally
     *          would
     *  HIT_GROUNT_LAST_FRAME: set when the piece hits the ground, and only if
     *          it hits the ground a second time (when this flag is set) does
     *          it stick, otherwise it is unset
     */
    uint8_t falling_status;

    /* status of the game, can be one of
     *  PLAY: normal running state
     *  PAUSED: paused
     *  GAME_OVER: game has ended because player lost
     */
    uint8_t state;

    // align piece_queue to dword
    char __attribute__((aligned(8))) __pad[0];

    /*
     * queue of next 14 pieces (2 sets of 7)
     *
     * we use the Random Generator algorithm to assign pieces, which randomly
     * permutes the list of 7 pieces over and over
     *
     * we need 2 of these groups of 7 to be calculated at all times, so piece
     * lookahead can go beyond just the next piece
     */
    uint8_t piece_queue[2 * N_PIECES];
    uint16_t queue_idx;

    // graphics object used to draw the game to the screen and to check if
    // tiles are empty or filled
    board_t board;

    // time counter, starts at 0 and is incremented every frame
    uint64_t time;

    // frames per step of animation (60 fps, so if this is 60, then the
    // game advances by one step every second)
    uint32_t frames_per_step;

    // align shared data with cache line
    char __attribute__((aligned(64))) __pad2[0];

    key_event_queue kq;

} tetris_t;


void tetris_init(tetris_t *t, gl_context *context, vec2 pos, float screen_width,
        float screen_height);


static void tetris_destroy(tetris_t *t) {
    board_destroy(&t->board);
}


/*
 * advance game state by one time step (should be called once per frame)
 */
void tetris_step(tetris_t *t);




#endif /* _TETRIS_H */
