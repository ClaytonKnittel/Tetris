#ifndef _PIECE_H
#define _PIECE_H



#define N_PIECES 7

#define ROTATE_CLOCKWISE 1
#define ROTATE_COUNTERCLOCKWISE -1

// max bounding-box widths and heights for any piece
#define PIECE_BB_W 4
#define PIECE_BB_H 4


#define EMPTY 0


/*
 * The layout and rotation rules of the peices follow the Super Rotation
 * System (SRS)
 *
 * All 7 tetrominoes follow, shown in each of their four possible orientations
 * (which are in order of orientation 0 - 3)
 */

/*
 * I tetromino (light blue)
 *
 * . . . .
 * O O O O
 * . . . .
 * . . . .
 *
 * . . O .
 * . . O .
 * . . O .
 * . . O .
 *
 * . . . .
 * . . . .
 * O O O O
 * . . . .
 *
 * . O . .
 * . O . .
 * . O . .
 * . O . .
 */
#define PIECE_I 1

/*
 * S tetromino (green)
 *
 * . O O
 * O O .
 * . . .
 *
 * . O .
 * . O O
 * . . O
 *
 * . . .
 * . O O
 * O O .
 *
 * O . .
 * O O .
 * . O .
 */
#define PIECE_S 2

/*
 * J tetromino (blue)
 * O . .
 * O O O
 * . . .
 *
 * . O O
 * . O .
 * . O .
 *
 * . . .
 * O O O
 * . . O
 *
 * . O .
 * . O .
 * O O .
 */
#define PIECE_J 3

/*
 * T tetromino (purple)
 *
 * . O .
 * O O O
 * . . .
 *
 * . O .
 * . O O
 * . O .
 *
 * . . .
 * O O O
 * . O .
 *
 * . O .
 * O O .
 * . O .
 */
#define PIECE_T 4

/*
 * L tetromino (orange)
 *
 * . . O
 * O O O
 * . . .
 *
 * . O .
 * . O .
 * . O O
 *
 * . . .
 * O O O
 * O . .
 *
 * O O .
 * . O .
 * . O .
 */
#define PIECE_L 5

/*
 * Z tetromino (red)
 *
 * O O .
 * . O O
 * . . .
 *
 * . . O
 * . O O
 * . O .
 *
 * . . .
 * O O .
 * . O O
 *
 * . O .
 * O O .
 * O . .
 */
#define PIECE_Z 6

/*
 * O tetromino (yellow)
 *
 * . O O .
 * . O O .
 * . . . .
 *
 * . O O .
 * . O O .
 * . . . .
 *
 * . O O .
 * . O O .
 * . . . .
 *
 * . O O .
 * . O O .
 * . . . .
 */
#define PIECE_O 7


/*
 * struct for recording layout of each piece
 */
struct __attribute__((aligned(8))) piece_layout {
    /*
     * bitvector describing piece layout. The layout is 4 pairs of coordinates,
     * each being between 0 - 3 (2 bits). There are 4 layouts (1 per
     * orientation)
     * 
     * layout of each 2-bitvector:
     *  15  14 13  12 11  10 9    8 7    6 5    4 3    2 1    0
     * +------+------+------+------+------+------+------+------+
     * |  y3  |  x3  |  y2  |  x2  |  y1  |  x1  |  y0  |  x0  |
     * +------+------+------+------+------+------+------+------+
     *
     */
    uint16_t __bitv[4];
};


/*
 * layout data of each of the pieces
 */
const extern struct piece_layout pieces[N_PIECES];


/*
 * defines 8 variables, names <var_name>_[xy]<index>, where (x1, y1) is the
 * coordinate pair for the first tile in the given piece_idx in given
 * orientation, (x2, y2) is the second, etc.
 */
#define __define_each_tile(var_name, piece_idx, orientation, x, y) \
    uint16_t __bitv_tmp = pieces[(piece_idx) - 1].__bitv[(orientation)]; \
    int8_t var_name ## _x1 = (x) + ( __bitv_tmp        & 0x3); \
    int8_t var_name ## _y1 = (y) + ((__bitv_tmp >>  2) & 0x3); \
    int8_t var_name ## _x2 = (x) + ((__bitv_tmp >>  4) & 0x3); \
    int8_t var_name ## _y2 = (y) + ((__bitv_tmp >>  6) & 0x3); \
    int8_t var_name ## _x3 = (x) + ((__bitv_tmp >>  8) & 0x3); \
    int8_t var_name ## _y3 = (y) + ((__bitv_tmp >> 10) & 0x3); \
    int8_t var_name ## _x4 = (x) + ((__bitv_tmp >> 12) & 0x3); \
    int8_t var_name ## _y4 = (y) + ((__bitv_tmp >> 14) & 0x3)

#define define_each_piece_tile(var_name, piece) \
    __define_each_tile(var_name, (piece).piece_idx, (piece).orientation, \
        (piece).board_x, (piece).board_y)


/*
 * data encoding which displacements to try placing a piece at when rotated,
 * for J, L, S, T, and Z tetrominoes
 */
const extern uint32_t displacement_trial_data[8];

/*
 * data encoding which displacements to try placing the I tetromino at when
 * rotated
 */
const extern uint32_t displacement_trial_data_I[16];

// given the previous orientation and rotation direction, gets the displacement
// vector index from displacement_trial_data_I
#define dtdI_idx_x(prev_or, rot) \
    (2 * (2 * (prev_or) + (((rot) >> 1) + 1)))

#define dtdI_idx_y(prev_or, rot) \
    (dtdI_idx_x(prev_or, rot) + 1)

/*
 * computes the difference between row1 and row2 from the displacement
 * trial data table
 */
static uint32_t sub_rows(uint32_t row1, uint32_t row2) {
    const uint32_t pad_mask = 0x77777;

    uint32_t neg_row2 = ((~row2) & pad_mask) + 0x11111;

    return row1 + neg_row2;
}

static int8_t sign_extend_3_bit(uint32_t val) {
    return (int8_t) ((((int32_t) val) << 29) >> 29);
}


#define for_each_displacement_trial(tile_idx, prev_or, rot, dx, dy) \
    uint32_t __x_row, __y_row; \
    if ((tile_idx) == PIECE_I) { \
        __x_row = displacement_trial_data_I[dtdI_idx_x(prev_or, rot)]; \
        __y_row = displacement_trial_data_I[dtdI_idx_y(prev_or, rot)]; \
    } \
    else { \
        __x_row = sub_rows( \
                displacement_trial_data[2 * (prev_or)], \
                displacement_trial_data[2 * (((prev_or) + (rot)) & 0x3)]); \
        __y_row = sub_rows( \
                displacement_trial_data[1 + 2 * (prev_or)], \
                displacement_trial_data[1 + 2 * (((prev_or) + (rot)) & 0x3)]); \
    } \
    uint8_t __cnt = 0; \
    for (dx = sign_extend_3_bit(__x_row & 0x7), dy = sign_extend_3_bit(__y_row & 0x7); \
            __cnt < 5; \
            __cnt++, __x_row >>= 4, __y_row >>= 4, \
            dx = sign_extend_3_bit(__x_row & 0x7), \
            dy = sign_extend_3_bit(__y_row & 0x7))



typedef struct piece {
    /*
     * 0 - 7, is either EMPTY or PIECE_*, where * is one of
     * I, S, J, T, L, Z, or O
     */
    uint8_t piece_idx;

    /*
     * either 0, 1, 2, or 3, where 0 is the default rotation, and 1,
     * 2, and 3 are successive 90 degree clockwise rotations of the
     * default
     */
    uint8_t orientation;

    /*
     * board_x and board_y are xy coordinates of bottom left corner of
     * enclosing rectangle about the piece (may be negative if the encloding
     * rectangle hangs off the screen, so long as the piece is still fully
     * contained in the screen
     */
    int8_t board_x;
    int8_t board_y;
} piece_t;


/*
 * initializes a new piece which is to be placed on a board of size
 * board_w x board_h. The piece is placed centered (biased to the left
 * in the case of a piece of width opposite the parity of board_w) on
 * top of the board, so that none of the segments of the piece are on
 * the board, but if it were to move down by one tile you could see it
 */
static void piece_init(piece_t *p, uint8_t piece_idx, uint8_t board_w,
        uint8_t board_h) {
    p->piece_idx = piece_idx;
    p->orientation = 0;

    // if the piece is width 4, then it is spawned centered, otherwise, it is
    // spawned just to the left of center
    p->board_x = board_w / 2 - 2;

    // if the piece is I, then its starting y-index has to be 2 below the top,
    // otherwise just 1 below the top
    p->board_y = board_h - (piece_idx == PIECE_I ? 2 : 1);
}

static void piece_move(piece_t *p, int8_t dx, int8_t dy) {
    p->board_x += dx;
    p->board_y += dy;
}

/*
 * rotates piece by 90 degrees clockwise or counterclockwise, depending on
 * rotation (either ROTATE_CLOCKWISE or ROTATE_COUNTERCLOCKWISE)
 */
static void piece_rotate(piece_t *p, int rotation) {
    p->orientation = (p->orientation + rotation) & 0x3;
}



#endif /* _PIECE_H */
