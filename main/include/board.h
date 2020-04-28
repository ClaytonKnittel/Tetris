#ifndef _BOARD_H
#define _BOARD_H

#include <square.h>


typedef struct board {
    // tile prototype shape (will be instanced)
    shape tile_prot;

    uint32_t *color_idxs;

    // width and height of board, in tiles
    uint32_t width, height;

    program p;
    GLuint color_array_loc, color_idxs_loc;
    GLuint width_loc, height_loc;
} board_t;


int board_init(board_t *b, uint32_t width, uint32_t height);

void board_destroy(board_t *b);

void board_draw(board_t *b);


#endif /* _BOARD_H */
