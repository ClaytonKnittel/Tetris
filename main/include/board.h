#ifndef _BOARD_H
#define _BOARD_H

#include <square.h>
#include <piece.h>

// number of possible states for each tile
#define N_STATES 8
#define LOG_N_STATES 3


// number of color indices that can be packed into a single uint32_t
#define COLOR_IDXS_PER_INT \
    ((sizeof(uint32_t) * 8) / LOG_N_STATES)

// mask for each individual color index
#define COLOR_IDX_MASK (N_STATES - 1)

// length of color_idxs 3-bitvector
#define color_idxs_arr_len(n_tiles) \
    (((n_tiles) + COLOR_IDXS_PER_INT - 1) / COLOR_IDXS_PER_INT)


typedef struct board {
    // tile prototype shape (will be instanced)
    shape tile_prot;

    // packed color indices for each tile
    uint32_t *color_idxs;

    // width and height of board, in tiles
    uint32_t width, height;

    program p;
    // see board.vs for descriptions of these
    GLuint color_array_loc, color_idxs_loc;
    GLuint width_loc, height_loc;
} board_t;


int board_init(board_t *b, uint32_t width, uint32_t height);

void board_destroy(board_t *b);


static void board_set_pos(board_t *b, float x, float y) {
    shape_set_pos(&b->tile_prot, x, y);
}

static void board_set_xscale(board_t *b, float xscale) {
    shape_set_xscale(&b->tile_prot, xscale);
}

static void board_set_yscale(board_t *b, float yscale) {
    shape_set_yscale(&b->tile_prot, yscale);
}


/*
 * returns 0 if the tile could not be set, 1 otherwise
 */
static int board_set_tile(board_t *b, int32_t x, int32_t y,
        uint32_t tile_color) {
    // by comparing as unsigned, take care of negative case
    if (((uint32_t) x) >= b->width || ((uint32_t) y) >= b->height) {
        return 0;
    }

    uint32_t idx = y * b->width + x;
    uint32_t color_idx = idx / COLOR_IDXS_PER_INT;
    uint32_t el_idx = idx - (color_idx * COLOR_IDXS_PER_INT);

    uint32_t mask = COLOR_IDX_MASK << (el_idx * LOG_N_STATES);
    tile_color <<= el_idx * LOG_N_STATES;
    b->color_idxs[color_idx] = (b->color_idxs[color_idx] & ~mask) |
        tile_color;
    return 1;
}

static uint8_t board_get_tile(board_t *b, int32_t x, int32_t y) {
    // by comparing as unsigned, take care of negative case
    if (((uint32_t) x) >= b->width || ((uint32_t) y) >= b->height) {
        // if this tile is above the top of the screen, we count it as empty,
        // otherwise, there is an imaginary border just outside the screen that
        // we say the piece is colliding with (return any nonzero value less
        // than 8)
        return (((uint32_t) x) < b->width && y >= 0) ? EMPTY : 1;
    }

    uint32_t idx = y * b->width + x;
    uint32_t color_idx = idx / COLOR_IDXS_PER_INT;
    uint32_t el_idx = idx - (color_idx * COLOR_IDXS_PER_INT);

    uint32_t set = b->color_idxs[color_idx] >> (el_idx * LOG_N_STATES);
    return set & COLOR_IDX_MASK;
}


/*
 * retusn 1 if the given row is full on the board (all nonzero entries), else 0
 */
static int board_row_full(board_t *b, int32_t row) {
    for (int32_t col = 0; col < b->width; col++) {
        if (board_get_tile(b, col, row) == EMPTY) {
            return 0;
        }
    }
    return 1;
}


/*
 * copies the entirety of src_row into dst_row
 * TODO optimize
 */
static void board_copy_row(board_t *b, int32_t dst_row, int32_t src_row) {
    for (int32_t col = 0; col < b->width; col++) {
        board_set_tile(b, col, dst_row, board_get_tile(b, col, src_row));
    }
}

static void board_clear_row(board_t *b, int32_t row) {
    for (int32_t col = 0; col < b->width; col++) {
        board_set_tile(b, col, row, EMPTY);
    }
}


void board_draw(board_t *b);


#endif /* _BOARD_H */
