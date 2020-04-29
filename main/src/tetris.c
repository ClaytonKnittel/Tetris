
#include <string.h>

#include <math/combinatorics.h>

#include <tetris.h>


void tetris_init(tetris_t *t, vec2 pos, float screen_width,
        float screen_height) {

    board_init(&t->board, TETRIS_WIDTH, TETRIS_HEIGHT);
    board_set_pos(&t->board, pos.x, pos.y);
    board_set_xscale(&t->board, screen_width);
    board_set_yscale(&t->board, screen_height);

    // first assign all pieces sequentially in the queue
    for (uint32_t i = 0; i < 2 * N_PIECES; i++) {
        t->piece_queue[i] = (i % N_PIECES) + 1;
    }
    t->queue_idx = 0;

    // then randomize both groups of 7
    permute(&t->piece_queue[0], 7, sizeof(t->piece_queue[0]));
    permute(&t->piece_queue[7], 7, sizeof(t->piece_queue[0]));

    // initialize falling piece to empty (no piece)
    piece_init(&t->falling_piece, EMPTY, 0, 0);

    // initialize time to 0
    t->time = 0LU;
    t->frames_per_step = 20;
}


static uint32_t _fetch_next_piece_idx(tetris_t *t) {
    uint32_t next = t->piece_queue[t->queue_idx];
    t->queue_idx++;

    if (t->queue_idx == N_PIECES) {
        // if we just grabbed the last piece in a group of 7, move
        // the subsequent group of 7 down and generate a new group
        // 7 to follow it
        memcpy(&t->piece_queue[0], &t->piece_queue[N_PIECES],
                N_PIECES * sizeof(t->piece_queue[0]));
    
        // generate next sequence and permute it
        for (uint32_t i = 0; i < 2 * N_PIECES; i++) {
            t->piece_queue[i] = (i % N_PIECES) + 1;
        }
        permute(&t->piece_queue[N_PIECES], N_PIECES,
                sizeof(t->piece_queue[0]));

        // reset queue index to reflect where the pieces moved
        t->queue_idx = 0;
    }
    return next;
}


static void _advance(tetris_t *t) {
    piece_t falling = t->falling_piece;

    if (falling.piece_idx == EMPTY) {
        // need to fetch next piece
        uint32_t next_piece_idx = _fetch_next_piece_idx(t);
        piece_init(&falling, next_piece_idx, t->board.width,
                t->board.height);
        t->falling_piece = falling;
    }
    // move the piece down
}


void tetris_step(tetris_t *t) {

    if (t->time % t->frames_per_step == 0) {
        // advance game state
        _advance(t);
    }

    t->time++;
}


