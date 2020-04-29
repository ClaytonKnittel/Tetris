#ifndef _TETRIS_H
#define _TETRIS_H

#include <math/vec2.h>

#include <board.h>
#include <piece.h>


#define TETRIS_WIDTH 10
#define TETRIS_HEIGHT 20



typedef struct tetris {
    // piece currently being controlled by player
    piece_t falling_piece;

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
    uint32_t queue_idx;

    // graphics object used to draw the game to the screen and to check if
    // tiles are empty or filled
    board_t board;

    // time counter, starts at 0 and is incremented every frame
    uint64_t time;

    // frames per step of animation (60 fps, so if this is 60, then the
    // game advances by one step every second)
    uint32_t frames_per_step;
} tetris_t;


void tetris_init(tetris_t *t, vec2 pos, float screen_width,
        float screen_height);


static void tetris_destroy(tetris_t *t) {
    board_destroy(&t->board);
}


/*
 * advance game state by one time step (should be called once per frame)
 */
void tetris_step(tetris_t *t);




#endif /* _TETRIS_H */
