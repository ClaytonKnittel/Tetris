#ifndef _BOARD_H
#define _BOARD_H

#include <square.h>


typedef struct board {
    shape *tiles;

    uint32_t *color_idxs;

    uint32_t width, height;

    program p;
    GLuint color_array_loc, color_idx_loc;
} board_t;


int board_init(board_t *b, uint32_t width, uint32_t height);

void board_destroy(board_t *b);

void board_draw(board_t *b);


#endif /* _BOARD_H */
