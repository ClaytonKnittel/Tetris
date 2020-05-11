#ifndef _HOLD_H
#define _HOLD_H


#include <gl/font.h>

#include <board.h>
#include <tetris.h>


typedef struct hold {
    board_t board;
    font_t *font;
    tetris_t *t;
    
    // for "Hold" text on screen
    float text_x, text_y, text_w, text_h;
} hold_t;


/*
 * width of hold is calculated based on height
 *
 * x and y are coordinates of lower left conrer of window, w is width of
 * text field (width of box is calculated so that tiles are squares) and
 * h is the height of the endtire window (text + box)
 */
int hold_init(hold_t *ph, float x, float y, float w, float h, font_t *font,
        tetris_t *t);


void hold_destroy(hold_t *h);


void hold_set(hold_t *h, uint8_t piece_idx);


void hold_draw(hold_t *h);


#endif /* _HOLD_H */
