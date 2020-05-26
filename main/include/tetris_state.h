#ifndef _TETRIS_STATE_H
#define _TETRIS_STATE_H


#include <stdint.h>

#include <gl/gl.h>

#include <tutil.h>

#include <board.h>
#include <piece.h>


/*
 * game states
 */
#define PLAY 0
#define GAME_OVER 1


/*
 * falling piece flags
 */
#define HIT_GROUND_LAST_FRAME 0x1

// desired fast falling speed, will try to get as close to this as possible
// (but we require fast falling delay to be an integer quotient of gravity
// delay)
#define DESIRED_MINOR_TICK_SPEED 5

// default avg. # frames between callbacks to held-down keys
#define DEFAULT_HELD_KEY_PERIOD 4.

// max number of times we allow the tile to hit into the ground before locking
// it there anyway (so you can't rotate forever and never place a tile)
#define MAX_GROUND_HIT_COUNT 10

// max number of frames we allow falling piece to stay controllable without
// having decreased the minimum y-coordinate it has ever been at
#define MAX_MIN_H_INC_TIME 10


// fraction of major time step between a ground hit detection and the
// time the ground hit will be tested again
#define CTRL_HIT_GROUND_LAST_DELAY .4f


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
     * tracks how many game ticks it has been since the minimum y-coordinate
     * of the falling piece has changed. If this is above a certain value, then
     * we do not allow ground hit count or hit ground last frame to prevent the
     * piece from sticking, nor do we allow wall kicks
     */
    uint8_t min_h_inc_time;
    // min y-value this piece has ever been at
    int8_t min_h;

} falling_piece_data;



// set after a piece swap occurs, and to be unset after the currently falling
// piece is placed (so you can't swap more than once on a single tile)
#define PIECE_HOLD_STALE 0x1

typedef struct piece_hold {
    // current piece being held
    uint8_t piece_idx;

    uint8_t flags;
} piece_hold;


#define piece_hold_is_stale(state) \
    ((state)->hold.flags & PIECE_HOLD_STALE)




/*
 *
 * scorer handles both the counting of the score (single player) and the
 * sending of lines to other players (multiplayer)
 *
 *
 * Scoring rules:
 *
 * Single player:
 *
 *  Single:                  100 * lvl
 *  Double:                  300 * lvl
 *  T-Spin:                  400 * lvl
 *  Triple:                  500 * lvl
 *  Tetris/T-Spin Single:    800 * lvl
 *  B2B Tetris /
 *      B2B T-Spin Single /
 *      T-Spin Double:      1200 * lvl
 *  T-Spin Triple:          1600 * lvl
 *  B2B T-Spin Double:      1800 * lvl
 *  B2B T-Spin Triple:      2400 * lvl
 *  Combo:                    50 * Combo len * lvl
 *
 *
 * Single/Double/Triple/Tetris: awarded for clearing 1/2/3/4 total lines
 *      in a single move (respectively)
 *
 * T-Spins:
 *      - Tetromino being locked is T
 *      - Last successful movement was a rotate
 *      - 3 of the 4 squares diagonally adjacent to center of T are occupied
 *
 * B2B: this clear and last clear were both "hard" clears, which constitutes
 *      one of the following:
 *      - Tetris
 *      - T-Spin Single/Double/Triple
 */



// move types
#define MOVE_TRANSLATE  0x1
#define MOVE_ROTATE     0x2


// set if the last successful movement was a rotate
#define SCORER_LAST_ACTION_WAS_ROTATE 0x1

// set if the last clear was a "hard" clear (defined above)
#define SCORER_LAST_CLEAR_WAS_HARD 0x2


// amount by which to increment cleared_lines_threshhold each time a level is
// passed
#define LINE_CLEAR_THRESH_INC 10

typedef struct scorer {
    // where the above status flags are put
    uint8_t status;


    // length of combo
    int8_t combo_len;

    // for single player mode, the current game score
    int32_t score;

    // number of lines that have been cleared since the start of the level
    int32_t cleared_lines;
    // number of lines needed to be cleared before advancing onto the next
    // level
    int32_t cleared_lines_threshhold;

    // the level that the game started on
    int32_t start_level;
    // game level, which determines game speed
    int32_t level;
} scorer_t;


// forward declare
struct tetris_state;

/*
 * to be called after a piece is placed. This method takes the game state
 * struct, the number of rows cleared in the move, and the type of the piece
 * which was just placed
 */
void tetris_scorer_count_move(struct tetris_state *s, int32_t num_rows_cleared,
        int print);

typedef struct tetris_state {

    /*
     * contains data about the current falling piece, like its location, type,
     * and runtime statistics such as max ground hit count,
     * time-since-last-drop, etc.
     */
    falling_piece_data fp_data;

    /*
     * piece hold is temporary storage for a falling piece the player can swap
     * out for the current falling piece at a later time
     */
    piece_hold hold;

    /*
     * used in the scoring system of the game
     */
    scorer_t scorer;


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

    // piece currently being controlled by player
    piece_t falling_piece;

    // graphics object used to draw the game to the screen and to check if
    // tiles are empty or filled
    board_t board;


    // time counter, starts at 0 and is incremented every frame
    uint64_t time;

    // frames per step of animation (60 fps, so if major tick count is 60,
    // then the game advances by one step every second)
    // a frame is counted if (time / tick_count) 
    float major_tick_count;
    float major_tick_time;
    // minor tick count is always an integer quotient of major tick count, so
    // we use major tick time mod minor tick count to check if it is a minor
    // time step
    float minor_tick_count;

    // frames between calls to key callbacks (i.e. how fast a held down key
    // will be pressed)
    float key_callback_count;
    float key_callback_time;

} tetris_state;



void tetris_state_init(tetris_state *state, gl_context *context, float x,
        float y, float screen_width, float screen_height);


void tetris_state_destroy(tetris_state *state);


void tetris_state_shallow_copy(tetris_state *dst, tetris_state *src);


void tetris_state_deep_copy(tetris_state *dst, tetris_state *src);

/*
 * sets falling speed of the game (period is average number of frames between
 * major time steps)
 */
void tetris_set_falling_speed(tetris_state *s, double period);



/*
 * fetches next piece from the piece queue and places the piece at the top of
 * the board, to begin falling
 */
void tetris_get_next_falling_piece(tetris_state *state);


/*
 * similar to get_next_falling_piece, but populates the fp pointer passed with
 * the new piece
 */
void tetris_get_next_falling_piece_transient(tetris_state *state);


/*
 * move the falling piece by dx, dy. If successful, the piece's location is
 * updated and 1 is returned, otherwise it is left where it was and 0 is
 * returned
 */
int tetris_move_piece(tetris_state *state, int dx, int dy);

/*
 * similar to move_piece, but expects the falling piece to not be on the board,
 * and does not place the new falling piece on the board
 *
 * this method only reads the state of the board, and updates fp to either the
 * new position if it could successfully move there, or it is not changed
 */
int tetris_move_piece_transient(tetris_state *state, int dx, int dy);


/*
 * rotate the following piece either clockwise or counterclockwise, depending
 * on the parameter rotation. If unsuccessful, we attempt to place the piece
 * in up to 4 more locations offset from where it would naturally end up,
 * which depend on the current orientation and rotation
 *
 * If all attempts to place the piece fail, 0 is returned and the piece is put
 * back where it was, otherwise 1 is returned and the piece is placed in the
 * new location
 */
int tetris_rotate_piece(tetris_state *state, int rotation,
        int allow_wall_kicks);


/*
 * similar to rotate_piece, but expects the falling piece to not be on the
 * board, and does not place the new falling piece on the board
 */
int tetris_rotate_piece_transient(tetris_state *state,
        int rotation, int allow_wall_kicks);


/*
 * performs hold action, which takes the currently falling piece and places it
 * in the hold slot. If there was already a piece in the hold slot, it becomes
 * the next falling piece and the piece queue does not move. Otherwise, if
 * the hold slot was empty, the next piece is immediately chosen off the queue
 */
void tetris_hold_piece(tetris_state *s);


/*
 * similar to hold_piece, but expects the falling piece to not be on the
 * board, and does not place the new falling piece on the board
 *
 * returns 1 if the piece could be held, 0 if it could not (because the hold
 * is locked from another piece being held recently)
 */
int tetris_hold_piece_transient(tetris_state *s);

/*
 * checks the current state to see if there are any lines to be cleared, and if
 * so, clears them and returns the number of lines cleared
 */
int tetris_clear_lines(tetris_state *state);


/*
 * advances game state by 1 tick (only modifies time values, does not make any
 * callbacks)
 */
void tetris_tick(tetris_state *s);


/*
 * advances game state forward by the given number of ticks, which may cause
 * the game state to change (i.e. if gravity moves a piece or something)
 * If the falling piece stuck at some point, the number of ticks remaining
 * is written back into ticks, and 1 is returned. Otherwise, if the piece was
 * not placed and the game was able to advance by ticks, then 0 is returned
 */
int tetris_advance_by_transient(tetris_state *state, uint64_t *ticks);


/*
 * advances the game state until the falling piece either falls by one tile or
 * sticks to the ground
 *
 * returns 1 if the piece stuck
 * returns 0 if the piece dropped
 */
int tetris_advance_until_drop_transient(tetris_state *state);




/*
 * returns 1 if the tetris state is in a major time step.
 *
 * In major time steps, the following actions happen:
 *  the falling piece tries to move down by one tile
 */
int tetris_is_major_time_step(tetris_state *s);

int tetris_is_minor_time_step(tetris_state *s);

int tetris_is_key_callback_step(tetris_state *s);

/*
 * advances time in tetris state to the next time in which some action happens
 */
void tetris_advance_to_next_action(tetris_state *s);

/*
 * advance to the next minor time step which is not a major time step
 *
 * return 0 if a piece was not placed/the game did not end before the next
 * minor time step, 1 otherwise
 */
int tetris_advance_to_next_minor_time_step(tetris_state *s);



// could not advance game state (game over)
#define ADVANCE_FAIL 0

// the falling piece only moved and was not placed
#define ADVANCE_MOVED_PIECE 1

// a piece was placed
#define ADVANCE_PLACED_PIECE 2

// the falling piece could not move down, but did not yet stick
#define ADVANCE_STALLED 3


/*
 * advances game state by one step. If it was successfully able to do so, then
 * 1 or 2 is returned, otherwise, if the game ended due to a game over, 0 is
 * returned
 *
 * should only be called on major time steps
 */
int tetris_advance(tetris_state *s);

/*
 * transient version of advance
 */
int tetris_advance_transient(tetris_state *s);




#endif /* _TETRIS_STATE_H */
