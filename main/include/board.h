#ifndef _BOARD_H
#define _BOARD_H

#include <square.h>

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


static void board_set_tile(board_t *b, uint32_t x, uint32_t y,
        uint32_t tile_color) {

    uint32_t idx = y * b->width + x;
    uint32_t color_idx = idx / COLOR_IDXS_PER_INT;
    uint32_t el_idx = idx - (color_idx * COLOR_IDXS_PER_INT);

    uint32_t mask = COLOR_IDX_MASK << (el_idx * LOG_N_STATES);
    tile_color <<= el_idx * LOG_N_STATES;
    b->color_idxs[color_idx] = (b->color_idxs[color_idx] & ~mask) |
        tile_color;
}

void board_draw(board_t *b);


#endif /* _BOARD_H */
