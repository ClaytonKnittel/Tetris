#ifndef _PIECE_H
#define _PIECE_H



#define N_PIECES 7


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


const extern struct piece_layout pieces[N_PIECES];



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
 * rotates piece by 90 degrees clockwise
 */
static void piece_rotate(piece_t *p) {
    p->orientation = (p->orientation + 1) & 0x3;
}



#endif /* _PIECE_H */