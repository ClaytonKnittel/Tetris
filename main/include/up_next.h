#ifndef _UP_NEXT_H
#define _UP_NEXT_H


#include <board.h>


typedef struct up_next {
    board_t board;
    uint32_t size;
} up_next_t;



/*
 * width of up next board is calculated based on height and queue size
 */
int up_next_init(up_next_t *u, uint32_t queue_size, float x, float y,
        float h);


void up_next_destroy(up_next_t *u);


void up_next_set(up_next_t *u, uint8_t *pieces);


void up_next_draw(up_next_t *u);



#endif /* _UP_NEXT_H */
