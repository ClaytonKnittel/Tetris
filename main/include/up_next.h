#ifndef _UP_NEXT_H
#define _UP_NEXT_H


#include <gl/font.h>

#include <board.h>
#include <tetris.h>


typedef struct up_next {
    board_t board;
    font_t *font;
    tetris_t *t;
    uint32_t size;

    // for "Next" text on screen
    float text_x, text_y, text_w, text_h;
} up_next_t;



/*
 * width of up next board is calculated based on height and queue size
 *
 * x, y are coordinates of lower left corner of window, w is width of text
 * field (width of queue is calculated so that the tiles are squares) and
 * h is the height of the entire window (text + queue)
 */
int up_next_init(up_next_t *u, uint32_t queue_size, float x, float y,
        float w, float h, font_t *font, tetris_t *t);


void up_next_destroy(up_next_t *u);


void up_next_set(up_next_t *u, uint8_t *pieces);


void up_next_draw(up_next_t *u);



#endif /* _UP_NEXT_H */
