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
#define GAME_OVER 1

// a row is currently being cleared out (gameplay freeezes during this
// animation). This is the stage during which the clear animator is active
#define CLEAR_ANIMATION 2



/*
 * falling piece flags
 */
#define HIT_GROUND_LAST_FRAME 0x1

#define FAST_FALLING_SPEEDUP 4

// max number of times we allow the tile to hit into the ground before locking
// it there anyway (so you can't rotate forever and never place a tile)
#define MAX_GROUND_HIT_COUNT 10

// max number of frames we allow falling piece to stay controllable without
// having decreased the minimum y-coordinate it has ever been at
#define MAX_MIN_H_INC_TIME 10


// key flags
#define DOWN_KEY 0x1
#define LEFT_KEY 0x4
#define RIGHT_KEY 0x8

// number of frames a key needs to be held down before it is continually
// repeated
#define REPEAT_TIMER 2


// number of frames (not time steps) between a ground hit detection and the
// time the ground hit will be tested again
#define CTRL_HIT_GROUND_LAST_DELAY 6


typedef struct falling_piece_data {

    /*
     * bitvector of status flags that apply to the falling piece. Possible
     * statuses are:
     *  HIT_GROUNT_LAST_FRAME: set when the piece hits the ground, and only if
     *          it hits the ground a second time (when this flag is set) does
     *          it stick, otherwise it is unset
     */
    uint8_t falling_status;

    // counts the number of successive frames the falling tile hit the ground
    uint8_t ground_hit_count;

    /*
     * tracks how many game ticks it has been since the miniimum y-coordinate
     * of the falling piece has changed. If this is above a certain value, then
     * we do not allow ground hit count or hit ground last frame to prevent the
     * piece from sticking
     */
    uint8_t min_h_inc_time;
    // min y-value this piece has ever been at
    int8_t min_h;

} falling_piece_data;



typedef struct controller {

    /*
     * counts of the number of frames both the left and right arrow keys have
     * been held for
     */
    uint8_t l_hold_count;
    uint8_t r_hold_count;

    /*
     * bitvector of status flags that apply to key presses. Possible values are
     *  DOWN_KEY:  set when the down key is pressed
     *  LEFT_KEY:  set when the left key is pressed
     *  RIGHT_KEY: set when the right key is pressed
     */
    uint8_t keypress_flags;

} controller;


#define is_key_pressed(tet, key) \
    ((tet)->ctrl.keypress_flags & (key))


#define is_only_key_pressed(tet, key) \
    ((tet)->ctrl.keypress_flags == (key))


/*
 * bitmask to apply to cleared_lines to get the starting row index
 */
#define CANIM_START_ROW_MASK 0xf

/*
 * bit offset in cleared_lines that the first bit denoting whether a row is
 * included in the cleared lines set is
 */
#define CANIM_ROW_BITV_OFF 4


typedef struct clear_animator {
    /*
     * at most 4 lines can be cleared at once, and they must be within a 4 row
     * sequence. For that reason, we reserve only the first 4 bits for the
     * location of the bottom-most line in the sequence (which we require to be
     * between 0-15), and the next 4 bits as a bitvector for which of the for
     * following lines (starting from the bottom) are being cleared
     *
     *     7       6       5       4     3    -    1
     * +-------+-------+-------+-------+-------------+
     * | row+3 | row+2 | row+1 |  row  |  start row  |
     * +-------+-------+-------+-------+-------------+
     *
     */
    uint8_t cleared_lines;

    // leftmost column to be cleared
    int8_t l_col;
    // rightmost column to be cleared
    int8_t r_col;
} clear_animator;


// gives the start row
#define canim_get_start_row(anim) \
    ((anim)->cleared_lines & CANIM_START_ROW_MASK)

// initializes start row (must be called before set_row_idx, as it overwrites
// cleared_lines entirely)
#define canim_set_start_row(anim, row) \
    ((anim)->cleared_lines = (row))


// gives whether or not the given row index (0-3) is in the set of rows to be
// cleared
#define canim_is_row_idx_set(anim, idx) \
    (((anim)->cleared_lines >> (CANIM_ROW_BITV_OFF + (idx))) & 0x1)

// true if any rows are set, false otherwise
#define canim_any_rows_set(anim) \
    ((anim)->cleared_lines >> CANIM_ROW_BITV_OFF)

// sets a given row index (0-3) to be in the set of rows to be cleared
#define canim_set_row_idx(anim, idx) \
    (anim)->cleared_lines |= (1U << (CANIM_ROW_BITV_OFF + (idx)))


// advance l_col down 1 and r_col up 1, without checking overflow as they are
// each 8 bits, and we will only increment a row at most 9 times beyond its end
#define canim_inc_cols(anim) \
    ({ \
     a->l_col--; \
     a->r_col++; \
    })



typedef struct tetris {
    // piece currently being controlled by player
    piece_t falling_piece;

    // for interfacing user input with the game mechanics
    controller ctrl;

    // for handling line clear animation
    clear_animator c_anim;

    /*
     * contains data about the current falling piece, like its location, type,
     * and runtime statistics such as max ground hit count,
     * time-since-last-drop, etc.
     */
    falling_piece_data fp_data;

    /* status of the game, can be one of
     *  PLAY: normal running state
     *  GAME_OVER: game has ended because player lost
     *  CLEAR_ANIMATION: currently animating the clearing of rows
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

    // frames per step of animation (60 fps, so if major tick count is 60,
    // then the game advances by one step every second)
    uint32_t major_tick_count;
    uint32_t minor_tick_count;

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
 * returns 1 if in fast-falling mode, 0 otherwise
 */
static int is_fast_falling(tetris_t *t) {
    return ((t->ctrl.keypress_flags & DOWN_KEY) != 0);
}



/*
 * advance game state by one time step (should be called once per frame)
 */
void tetris_step(tetris_t *t);




#endif /* _TETRIS_H */
