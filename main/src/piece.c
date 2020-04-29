
#include <stdint.h>

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

