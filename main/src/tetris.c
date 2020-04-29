
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

    for (int i = 0; i < N_PIECES; i++) {
        printf("%d ", t->piece_queue[i]);
    }
    printf("\n");

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



/*
 * places a piece on the board by setting each of the tiles it occupies to its
 * color
 */
static void _place_piece(tetris_t *t, piece_t piece) {
    define_each_piece_tile(p, piece);
    uint32_t tile_color = piece.piece_idx;

    board_set_tile(&t->board, p_x1, p_y1, tile_color);
    board_set_tile(&t->board, p_x2, p_y2, tile_color);
    board_set_tile(&t->board, p_x3, p_y3, tile_color);
    board_set_tile(&t->board, p_x4, p_y4, tile_color);
}


/*
 * removes a piece on the board by setting each of the tiles it occupies back
 * to EMPTY
 */
static void _remove_piece(tetris_t *t, piece_t piece) {
    define_each_piece_tile(p, piece);

    board_set_tile(&t->board, p_x1, p_y1, EMPTY);
    board_set_tile(&t->board, p_x2, p_y2, EMPTY);
    board_set_tile(&t->board, p_x3, p_y3, EMPTY);
    board_set_tile(&t->board, p_x4, p_y4, EMPTY);
}



/*
 * checks to see if the given piece will be colliding with any pieces that
 * are already on the given board
 */
static int _piece_collides(tetris_t *t, piece_t piece) {
    define_each_piece_tile(p, piece);
    
    uint8_t p1 = board_get_tile(&t->board, p_x1, p_y1);
    uint8_t p2 = board_get_tile(&t->board, p_x2, p_y2);
    uint8_t p3 = board_get_tile(&t->board, p_x3, p_y3);
    uint8_t p4 = board_get_tile(&t->board, p_x4, p_y4);

    // if all squares are empty, then all four tiles or-ed together will be
    // 0, otherwise, if any tile isn't empty, we will get a nonzero result
    return ((p1 | p2) | (p3 | p4)) != 0;
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

    // first, remove the piece from the board where it is
    _remove_piece(t, falling);

    // and now advance the piece downward
    piece_t new_falling = falling;
    piece_move(&new_falling, 0, -1);

    // check to see if there would be any collisions here
    if (_piece_collides(t, new_falling)) {
        // then the piece cannot move down, it is now stuck where it was.
        // First, put the old piece back, then unset the falling piece
        _place_piece(t, falling);
        piece_init(&t->falling_piece, EMPTY, 0, 0);
    }
    else {
        // otherwise, the piece can now be moved down into the new location
        _place_piece(t, new_falling);
        t->falling_piece = new_falling;
    }
}


void tetris_step(tetris_t *t) {

    if (t->time % t->frames_per_step == 0) {
        // advance game state
        _advance(t);
    }

    t->time++;
}


