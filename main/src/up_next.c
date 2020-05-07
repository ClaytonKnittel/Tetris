
#include <assert.h>
#include <up_next.h>
#include <piece.h>


int up_next_init(up_next_t *u, uint32_t queue_size, float x, float y,
        float h) {

    assert(queue_size > 0);

    uint32_t b_w = PIECE_BB_W;
    uint32_t b_h = PIECE_BB_H * queue_size;

    board_init(&u->board, b_w, b_h);
    board_set_pos(&u->board, x, y);
    board_set_xscale(&u->board, h * b_w / b_h);
    board_set_yscale(&u->board, h);

    u->size = queue_size;

    return 0;
}


void up_next_destroy(up_next_t *u) {
    board_destroy(&u->board);
}


void up_next_set(up_next_t *u, uint8_t *pieces) {
    board_clear(&u->board);

    piece_t p = {
        .orientation = 0,
        .board_x = 0
    };

    for (uint32_t i = 0; i < u->size; i++) {
        p.piece_idx = pieces[i];
        p.board_y = (u->size - 1 - i) * PIECE_BB_H;
        board_place_piece(&u->board, p);
    }
}


void up_next_draw(up_next_t *u) {
    board_draw(&u->board);
}


