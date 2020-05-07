#ifndef _UP_NEXT_H
#define _UP_NEXT_H


#include <board.h>


typedef struct up_next {
    board_t board;

} up_next_t;



int up_next_init(&up_next_t *u, uint32_t queue_size, float x, float y, float w,
        float h);


void up_next_destroy(&up_next_t *u);




#endif /* _UP_NEXT_H */
