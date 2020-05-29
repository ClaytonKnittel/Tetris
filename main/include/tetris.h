#ifndef _TETRIS_H
#define _TETRIS_H

#include <math/vec2.h>
#include <gl/gl.h>

#include <board.h>
#include <key_event.h>
#include <piece.h>
#include <tetris_state.h>


#define TETRIS_WIDTH 10
#define TETRIS_HEIGHT 20



// additional state added to possible states in tetris_state

// a row is currently being cleared out (gameplay freeezes during this
// animation). This is the stage during which the clear animator is active
#define CLEAR_ANIMATION 2


// key flags
#define CTRL_ACTIVE 0x1
#define DOWN_KEY 0x2
#define LEFT_KEY 0x4
#define RIGHT_KEY 0x8

// number of frames a key needs to be held down before it is continually
// repeated
#define REPEAT_TIMER 2



typedef struct controller {

    /*
     * counts of the number of frames both the left and right arrow keys have
     * been held for
     */
    uint8_t l_hold_count;
    uint8_t r_hold_count;

    /*
     * bitvector of status flags that apply to key presses. Possible values are
     *  CTRL_ACTIVE: set when the controller is responsive to input (to be
     *          unset when there is no falling piece)
     *  DOWN_KEY:    set when the down key is pressed
     *  LEFT_KEY:    set when the left key is pressed
     *  RIGHT_KEY:   set when the right key is pressed
     */
    uint8_t keypress_flags;

} controller;


#define ctrl_is_active(ctrl) \
    ((ctrl)->keypress_flags & CTRL_ACTIVE)

#define ctrl_set_active(ctrl) \
    (ctrl)->keypress_flags |= CTRL_ACTIVE


#define is_key_pressed(tet, key) \
    ((tet)->ctrl.keypress_flags & (key))


#define is_only_key_pressed(tet, key) \
    ((tet)->ctrl.keypress_flags == (key))


/*
 * resets the controller state back to the default settings (inactive, no keys
 * pressed)
 */
static void ctrl_reset(controller *c) {
    __builtin_memset(c, 0, sizeof(controller));
}



/*
 * bitmask to apply to cleared_lines to get the starting row index
 */
#define CANIM_START_ROW_MASK 0xf

/*
 * bit offset in cleared_lines that the first bit denoting whether a row is
 * included in the cleared lines set is
 */
#define CANIM_ROW_BITV_OFF 4

/*
 * default number of frames between animation steps
 */
#define CANIM_STEP_COUNT 6


typedef struct clear_animator {
    /*
     * at most 4 lines can be cleared at once, and they must be within a 4 row
     * sequence. For that reason, we reserve only the first 4 bits for the
     * location of the bottom-most line in the sequence (which we require to be
     * between 0-15), and the next 4 bits as a bitvector for which of the for
     * following lines (starting from the bottom) are being cleared
     *
     *     7       6       5       4     3    -    0
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

    // count of which frame delay we are on, always between 0 and
    // CANIM_STEP_COUNT - 1 and increments by 1 each frame
    uint8_t canim_time;

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


// gives true if this time step is a clear animator time step, false otherwise
#define is_canim_time_step(anim) \
    ((anim)->canim_time == 0)

// increments clear animator time by one
#define canim_step(anim) \
    ((anim)->canim_time = ((anim)->canim_time + 1) % CANIM_STEP_COUNT)



typedef struct tetris {

    // for interfacing user input with the game mechanics
    controller ctrl;

    // for handling line clear animation
    clear_animator c_anim;

    tetris_state game_state;

    // align shared data with cache line
    char __attribute__((aligned(64))) __pad2[0];

    key_event_queue kq;

} tetris_t;


void tetris_init(tetris_t *t, gl_context *context, float x, float y,
        float screen_width, float screen_height);


static void tetris_destroy(tetris_t *t) {
    tetris_state_destroy(&t->game_state);
}



/*
 * returns 1 if in fast-falling mode, 0 otherwise
 */
static int is_fast_falling(tetris_t *t) {
    return ((t->ctrl.keypress_flags & DOWN_KEY) != 0);
}


/*
 * returns a pointer to the beginning of the up-next queue of pieces
 */
static uint8_t* tetris_get_up_next(tetris_t *t) {
    return &t->game_state.piece_queue[t->game_state.queue_idx];
}



/*
 * advance game state by one time step (should be called once per frame)
 */
void tetris_step(tetris_t *t);




#endif /* _TETRIS_H */
