
#include <assert.h>
#include <up_next.h>
#include <piece.h>


int up_next_init(&up_next_t *u, uint32_t queue_size, float x, float y, float w,
        float h) {

    assert(queue_size > 0);

    board_init(&u->board, PIECE_BB_W, (PIECE_BB_H + 1) * queue_size - 1);
    board_set_pos(&u->board, x, y);
    board_set_xscale(&u->board, w);
    board_set_yscale(&u->board, h);
}


void up_next_destroy(up_next_t *u) {
    board_destroy(&u->board);
}



static void up_next_draw(up_next_t *u) {
    board_draw(&u->board);
}


