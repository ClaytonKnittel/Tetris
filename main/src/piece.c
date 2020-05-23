
#include <stdint.h>
#include <stdio.h>

#include <piece.h>


const struct piece_layout pieces[N_PIECES] = {
    // I tetromino
    {
        .__bitv = {
            /*
             * . . . .
             * O O O O
             * . . . .
             * . . . .
             *
             * (0, 2), (1, 2), (2, 2), (3, 2)
             */
            0xba98,
            /*
             * . . O .
             * . . O .
             * . . O .
             * . . O .
             *
             * (2, 0), (2, 1), (2, 2), (2, 3)
             */
            0xea62,
            /*
             * . . . .
             * . . . .
             * O O O O
             * . . . .
             *
             * (0, 1), (1, 1), (2, 1), (3, 1)
             */
            0x7654,
            /*
             * . O . .
             * . O . .
             * . O . .
             * . O . .
             *
             * (1, 0), (1, 1), (1, 2), (1, 3)
             */
            0xd951
        }
    },
    // S tetromino
    {
        .__bitv = {
            /*
             * . O O
             * O O .
             * . . .
             *
             * (0, 1), (1, 1), (1, 2), (2, 2)
             */
            0xa954,
            /*
             * . O .
             * . O O
             * . . O
             *
             * (2, 0), (1, 1), (2, 1), (1, 2)
             */
            0x9652,
            /*
             * . . .
             * . O O
             * O O .
             *
             * (0, 0), (1, 0), (1, 1), (2, 1)
             */
            0x6510,
            /*
             * O . .
             * O O .
             * . O .
             *
             * (1, 0), (0, 1), (1, 1), (0, 2)
             */
            0x8541
        }
    },
    // J tetromino
    {
        .__bitv = {
            /*
             * O . .
             * O O O
             * . . .
             *
             * (0, 1), (1, 1), (2, 1), (0, 2)
             */
            0x8654,
            /*
             * . O O
             * . O .
             * . O .
             *
             * (1, 0), (1, 1), (1, 2), (2, 2)
             */
            0xa951,
            /*
             * . . .
             * O O O
             * . . O
             *
             * (2, 0), (0, 1), (1, 1), (2, 1)
             */
            0x6542,
            /*
             * . O .
             * . O .
             * O O .
             *
             * (0, 0), (1, 0), (1, 1), (1, 2)
             */
            0x9510
        }
    },
    // T tetromino
    {
        .__bitv = {
            /*
             * . O .
             * O O O
             * . . .
             *
             * (0, 1), (1, 1), (2, 1), (1, 2)
             */
            0x9654,
            /*
             * . O .
             * . O O
             * . O .
             *
             * (1, 0), (1, 1), (2, 1), (1, 2)
             */
            0x9651,
            /*
             * . . .
             * O O O
             * . O .
             *
             * (1, 0), (0, 1), (1, 1), (2, 1)
             */
            0x6541,
            /*
             * . O .
             * O O .
             * . O .
             *
             * (1, 0), (0, 1), (1, 1), (1, 2)
             */
            0x9541
        }
    },
    // L tetromino
    {
        .__bitv = {
            /*
             * . . O
             * O O O
             * . . .
             *
             * (0, 1), (1, 1), (2, 1), (2, 2)
             */
            0xa654,
            /*
             * . O .
             * . O .
             * . O O
             *
             * (1, 0), (2, 0), (1, 1), (1, 2)
             */
            0x9521,
            /*
             * . . .
             * O O O
             * O . .
             *
             * (0, 0), (0, 1), (1, 1), (2, 1)
             */
            0x6540,
            /*
             * O O .
             * . O .
             * . O .
             *
             * (1, 0), (1, 1), (0, 2), (1, 2)
             */
            0x9851
        }
    },
    // Z tetromino
    {
        .__bitv = {
            /*
             * O O .
             * . O O
             * . . .
             *
             * (1, 1), (2, 1), (0, 2), (1, 2)
             */
            0x9865,
            /*
             * . . O
             * . O O
             * . O .
             *
             * (1, 0), (1, 1), (2, 1), (2, 2)
             */
            0xa651,
            /*
             * . . .
             * O O .
             * . O O
             *
             * (1, 0), (2, 0), (0, 1), (1, 1)
             */
            0x5421,
            /*
             * . O .
             * O O .
             * O . .
             *
             * (0, 0), (0, 1), (1, 1), (1, 2)
             */
            0x9540
        }
    },
    // O tetromino
    {
        .__bitv = {
            /*
             * . O O .
             * . O O .
             * . . . .
             *
             * (1, 1), (2, 1), (1, 2), (2, 2)
             */
            0xa965,
            /*
             * . O O .
             * . O O .
             * . . . .
             *
             * (1, 1), (2, 1), (1, 2), (2, 2)
             */
            0xa965,
            /*
             * . O O .
             * . O O .
             * . . . .
             *
             * (1, 1), (2, 1), (1, 2), (2, 2)
             */
            0xa965,
            /*
             * . O O .
             * . O O .
             * . . . .
             *
             * (1, 1), (2, 1), (1, 2), (2, 2)
             */
            0xa965
        }
    }
};



/*
 * encodes the 5 displacement trials to be done on a tile which was rotated for
 * J, L, S, T, and Z tetrominoes
 *
 * rules are as follows:
 *
 *  trial             1       2       3       4       5
 *  Orientation 0: ( 0, 0) ( 0, 0) ( 0, 0) ( 0, 0) ( 0, 0)
 *  Orientation 1: ( 0, 0) (+1, 0) (+1,-1) ( 0,+2) (+1,+2)
 *  Orientation 2: ( 0, 0) ( 0, 0) ( 0, 0) ( 0, 0) ( 0, 0)
 *  Orientation 3: ( 0, 0) (-1, 0) (-1,-1) ( 0,+2) (-1,+2)
 *
 *
 * to find the 5 displacements to try, take the row of the orientation you are
 * rotating to, subtract it element-wise from the row you are rotating from.
 * The result is the 5 pairs of offsets to be tried
 *
 * Because the displacements remain in the range -2 to 2, we only need 3 bits
 * per number
 *
 * encode:
 *      0 as 0000 = 0x0
 *      1 as 0001 = 0x1
 *      2 as 0010 = 0x2
 *     -1 as 0111 = 0x7
 *     -2 as 0110 = 0x6
 *
 * each number is given an extra bit of padding so they can all be added
 * together with a single 32-bit integer addition
 *
 * to negate all of the numbers, we have to bitwise invert, mask off the
 * padding bits which were set by the inversion, then add 1 to each of the
 * numbers, which looks like
 *
 *  pad_mask = 0x77777;
 *  neg_row = (~row & pad_mask) + 0x11111;
 *  res_row = pos_row + neg_row
 *
 * the addition that will be done next is guaranteed not to overflow because
 * the max value of two numbers which will ever be added is -1 (0x7) and -0
 * (~0x0 + 1 = 0x8), which sum to 0xf exactly
 *
 *
 */
const uint32_t displacement_trial_data[8] = {
    // orientation 0, x
    0x00000,
    // orientation 0, y
    0x00000,

    // orientation 1, x
    0x10110,
    // orientation 1, y
    0x22700,

    // orientation 2, x
    0x00000,
    // orientation 2, y
    0x00000,

    // orientation 3, x
    0x70770,
    // orientation 3, y
    0x22700,
};



/*
 * here, we denote the four orientations as 0, R, 2, and L (going from 0 - 3
 * in the notation we have been using so far)
 *
 * here is the table of trials laid out:
 *
 * trial    1       2       3       4       5
 *  0->R ( 0, 0) (-2, 0) (+1, 0) (+1,+2) (-2,-1)
 *  R->0 ( 0, 0) (+2, 0) (-1, 0) (+2,+1) (-1,-2)
 *  R->2 ( 0, 0) (-1, 0) (+2, 0) (-1,+2) (+2,-1)
 *  2->R ( 0, 0) (-2, 0) (+1, 0) (-2,+1) (+1,-1)
 *  2->L ( 0, 0) (+2, 0) (-1, 0) (+2,+1) (-1,-1)
 *  L->2 ( 0, 0) (+1, 0) (-2, 0) (+1,+2) (-2,-1)
 *  L->0 ( 0, 0) (-2, 0) (+1, 0) (-2,+1) (+1,-2)
 *  0->L ( 0, 0) (+2, 0) (-1, 0) (-1,+2) (+2,-1)
 *
 * the rotation data layout is the same as above, except now all 8 orientation-
 * rotation pairs are laid out explicitly (since there is no way to do what we
 * did above for this tile, with the changes from Arika SRS)
 */
const uint32_t displacement_trial_data_I[16] = {
    // 0 -> L (x)
    0x27720,
    //        (y)
    0x72000,

    // 0 -> R (x)
    0x61160,
    //        (y)
    0x72000,

    // R -> 0 (x)
    0x72720,
    //        (y)
    0x61000,

    // R -> 2 (x)
    0x27270,
    //        (y)
    0x72000,

    // 2 -> R (x)
    0x16160,
    //        (y)
    0x71000,

    // 2 -> L (x)
    0x72720,
    //        (y)
    0x71000,

    // L -> 2 (x)
    0x61610,
    //        (y)
    0x72000,

    // L -> 0 (x)
    0x16160,
    //        (y)
    0x61000,
};



/*
 * returns 1 if the two pieces are equal, meaning they appear in the same spot
 * on the board, though they may be in different orientations if the pieces
 * have any sort of rotational symmetry
 */
int piece_equals(piece_t p1, piece_t p2) {
    uint8_t piece_idx = p1.piece_idx;
    // two orientations
    uint8_t o1, o2;
    // two positions
    int8_t x1, y1, x2, y2;

    if (piece_idx != p2.piece_idx) {
        return 0;
    }

    o1 = p1.orientation;
    o2 = p2.orientation;

    x1 = p1.board_x;
    y1 = p1.board_y;
    x2 = p2.board_x;
    y2 = p2.board_y;

    switch (piece_idx) {
        case PIECE_O:
            // this one is easy, the orientations have no affect on the piece,
            // so we only need to compare coordinates
            break;
        case PIECE_I:
        case PIECE_S:
        case PIECE_Z:
            // for I, S, and Z tetrominos, the even and odd rotations are
            // symmetric, so the rotations must have the same parity if the
            // pieces are in the same place
            if ((o1 ^ o2) & 1) {
                // different parity
                return 0;
            }

            // to adjust for the different location of the piece in orientation
            // 0 vs 2, we add 1 to the y coordinate when in orientation 2
            // similarly with 1 vs 3, we add 1 to the x coordinate when in
            // orientation 3
            x1 += (o1 == 3);
            y1 += (o1 == 2);
            x2 += (o2 == 3);
            y2 += (o2 == 2);

            // now the two pieces are in the same position only if their
            // coordinates are the same
            break;
        case PIECE_J:
        case PIECE_T:
        case PIECE_L:
            // for J, T, and L tetrominos, the orientations are all unique, so
            // we just need to compare orientations, and if those match, we
            // move on to compare their positions
            if (o1 != o2) {
                // different orientations
                return 0;
            }
            break;
        default:
            // invalid piece index!
            return 0;
    }

    return (x1 == x2) && (y1 == y2);
}

/*
 * finds the bottom left corner of the piece, i.e. the bottom left corner of
 * the tightest bounding box around the piece
 */
void piece_bottom_left_corner(piece_t p, int8_t *res_x, int8_t *res_y) {
    uint8_t piece_idx = p.piece_idx;
    uint8_t o;
    int8_t x, y;

    o = p.orientation;
    x = p.board_x;
    y = p.board_y;

    switch (piece_idx) {
        case PIECE_O:
            x++;
            y++;
            break;
        case PIECE_I:
            // for even orientations, y is shifted up by at least one (2 if
            // o == 0)
            // for odd orientations, x is shifted up by at least one (2 if
            // o == 1)
            x += ( o & 1) + (o == 1);
            y += (~o & 1) + (o == 0);
            break;
        case PIECE_S:
        case PIECE_J:
        case PIECE_T:
        case PIECE_L:
        case PIECE_Z:
            // in orientation 0, y is shifted up by 1
            // in orientation 1, x is shifted up by 1
            x += (o == 1);
            y += (o == 0);
            break;
        default:
            // invalid piece index!
            return;
    }

    *res_x = x;
    *res_y = y;
}


/*
 * returns 1 if the given piece contains the coordinate pair (x, y), otherwise
 * returns 0
 */
int piece_contains(piece_t p, int8_t x, int8_t y) {
    __define_each_tile(p, p.piece_idx, p.orientation, p.board_x, p.board_y);

    return (p_x1 == x && p_y1 == y) ||
           (p_x2 == x && p_y2 == y) ||
           (p_x3 == x && p_y3 == y) ||
           (p_x4 == x && p_y4 == y);
}



void print_piece(piece_t p) {
    const static char pcs[8]  = { '_', 'I', 'S', 'J', 'T', 'L', 'Z', 'O' };
    const static char dirs[4] = { 'N', 'E', 'S', 'W' };
    printf("PIECE_%c: (%d, %d) %c\n", pcs[p.piece_idx], p.board_x, p.board_y, dirs[p.orientation]);
}


