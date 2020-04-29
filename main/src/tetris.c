
#include <string.h>

#include <math/combinatorics.h>
#include <util.h>

#include <tetris.h>



static void _key_event(gl_context *context, int key, int scancode, int action,
        int mods) {

    tetris_t *t = (tetris_t*) context->user_data;

    key_event e = {
        .key = key,
        .scancode = scancode,
        .action = action,
        .mods = mods
    };
    key_event_queue_push(&t->kq, &e);
}

/*
 * initializes key event callback routine
 */
static void _init_key_listeners(tetris_t *t, gl_context *context) {
    gl_register_key_callback(context, &_key_event);
}


static void _init_fp_data(falling_piece_data *f) {
    f->falling_status = 0;
    f->ground_hit_count = 0;

    f->min_h_inc_time = 0;
    // set min_h to max value of height
    f->min_h = 0x7f;
}

static void _reset_fp_data(falling_piece_data *f) {
    f->falling_status &= ~HIT_GROUND_LAST_FRAME;
    f->ground_hit_count = 0;

    f->min_h_inc_time = 0;
    // set min_h to max value of height
    f->min_h = 0x7f;
}



static void _init_controller(controller *ctrl) {
    __builtin_memset(ctrl, 0, sizeof(controller));
}


void tetris_init(tetris_t *t, gl_context *context, vec2 pos,
        float screen_width, float screen_height) {

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
    permute(&t->piece_queue[0],        N_PIECES, sizeof(t->piece_queue[0]));
    permute(&t->piece_queue[N_PIECES], N_PIECES, sizeof(t->piece_queue[0]));

    // initialize falling piece to empty (no piece)
    piece_init(&t->falling_piece, EMPTY, 0, 0);

    _init_fp_data(&t->fp_data);

    _init_controller(&t->ctrl);

    t->state = PLAY;

    key_event_queue_init(&t->kq);
    _init_key_listeners(t, context);

    // initialize time to 0
    t->time = 0LU;
    t->major_tick_count = 16;
    t->minor_tick_count = 4;

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
 *
 * returns 1 if any piece could be placed on the board, otherwise 0
 */
static int _place_piece(tetris_t *t, piece_t piece) {
    define_each_piece_tile(p, piece);
    uint32_t tile_color = piece.piece_idx;

    int piece_placed = 0;

    piece_placed |= board_set_tile(&t->board, p_x1, p_y1, tile_color);
    piece_placed |= board_set_tile(&t->board, p_x2, p_y2, tile_color);
    piece_placed |= board_set_tile(&t->board, p_x3, p_y3, tile_color);
    piece_placed |= board_set_tile(&t->board, p_x4, p_y4, tile_color);

    return piece_placed;
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





/*
 * move the falling piece by dx, dy. If successful, the piece's location is
 * updated and 1 is returned, otherwise it is left where it was and 0 is
 * returned
 */
static int _move_piece(tetris_t *t, int dx, int dy) {
    piece_t falling;
    piece_t new_falling;

    falling = t->falling_piece;

    // first, remove the piece from the board where it is
    _remove_piece(t, falling);

    // and now advance the piece to wherever it needs to go
    new_falling = falling;
    piece_move(&new_falling, dx, dy);

    // check to see if there would be any collisions here
    if (_piece_collides(t, new_falling)) {
        // then the piece cannot move, so put it back
        _place_piece(t, falling);
        return 0;
    }
    else {
        // otherwise, the piece can now be moved down into the new location
        _place_piece(t, new_falling);
        t->falling_piece = new_falling;
        return 1;
    }
}


/*
 * rotate the following piece either clockwise or counterclockwise, depending
 * on the parameter rotation. If unsuccessful, we attempt to place the piece
 * in up to 4 more locations offset from where it would naturally end up,
 * which depend on the current orientation and rotation
 *
 * If all attempts to place the piece fail, 0 is returned and the piece is put
 * back where it was, otherwise 1 is returned and the piece is placed in the
 * new location
 */
static int _rotate_piece(tetris_t *t, int rotation) {
    piece_t falling;
    piece_t new_falling;
    int8_t dx, dy;

    falling = t->falling_piece;

    // first, remove the piece from the board where it is
    _remove_piece(t, falling);

    // and now advance the piece to wherever it needs to go
    new_falling = falling;
    piece_rotate(&new_falling, rotation);

    if (falling.piece_idx == PIECE_O) {
        // this piece cannot be rotated, so we don't loop

        // check to see if there would be any collisions here
        if (!_piece_collides(t, new_falling)) {
            // the piece can now be moved down into the new location
            _place_piece(t, new_falling);
            t->falling_piece = new_falling;
            return 1;
        }
    }
    else {
        for_each_displacement_trial(falling.piece_idx, falling.orientation,
                rotation, dx, dy) {

            piece_move(&new_falling, dx, dy);

            // check to see if there would be any collisions here
            if (!_piece_collides(t, new_falling)) {
                // the piece can now be moved down into the new location
                _place_piece(t, new_falling);
                t->falling_piece = new_falling;
                return 1;
            }

            printf("tried (%d, %d)\n", dx, dy);

            // move it back before next iteration
            piece_move(&new_falling, -dx, -dy);
        }
    }

    // there were no suitable locations for the piece, so put it back where it
    // was
    _place_piece(t, falling);
    return 0;
}



static void _piece_placed(tetris_t *t);


/*
 * advances game state by one step. If it was successfully able to do so, then
 * 1 is returned, otherwise, if the game ended due to a game over, 0 is
 * returned
 *
 * should only be called on major time steps
 */
static int _advance(tetris_t *t) {
    piece_t falling;

    for (;;) {
        falling = t->falling_piece;
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
            int placed = _place_piece(t, falling);

            if (t->fp_data.falling_status & HIT_GROUND_LAST_FRAME) {
                // if the piece spend two successive frames hitting the ground,
                // we stick it wherever it is
                _reset_fp_data(&t->fp_data);

                if (!placed) {
                    // if we could not place this piece even partially on the
                    // board, then the game is over
                    return 0;
                }

                // on a successful place, make this call to check for filled rows
                _piece_placed(t);

                piece_init(&t->falling_piece, EMPTY, 0, 0);
                // now need to move the new falling piece down
                continue;
            }
            else {
                // if we were not hitting the ground last frame, then we set
                // this flag and allow the player to try moving the piece again
                // before it sticks
                t->fp_data.falling_status |= HIT_GROUND_LAST_FRAME;
                break;
            }
        }
        else {
            // otherwise, the piece can now be moved down into the new location
            _place_piece(t, new_falling);
            t->falling_piece = new_falling;

            // unset hit ground last frame flag, in case it was set and the
            // piece was subsequently moved off the platform
            t->fp_data.falling_status &= ~HIT_GROUND_LAST_FRAME;
            t->fp_data.ground_hit_count = 0;
            // that is the completion of this move
            break;
        }
    }

    return 1;
}



/*
 * to be called whenever a piece sticks to the ground (to check for row
 * clearing)
 */
static void _piece_placed(tetris_t *t) {
    // check the rows of the currently falling piece
    // just to be safe, we check the entire bounding box

    int32_t bot = MAX(t->falling_piece.board_y, 0);
    int32_t top = MIN(t->falling_piece.board_y + PIECE_BB_H, t->board.height);

    int32_t r;
    int32_t dst_row = bot;

    for (r = bot; r < top; r++) {

        if (dst_row != r) {
            board_copy_row(&t->board, dst_row, r);
        }

        if (!board_row_full(&t->board, dst_row)) {
            dst_row++;
        }
    }
    if (r != dst_row) {
        for (; r < t->board.height; r++, dst_row++) {
            board_copy_row(&t->board, dst_row, r);
        }
        for (; dst_row < t->board.height; dst_row++) {
            board_clear_row(&t->board, dst_row);
        }
    }
}


/*
 * to be called whenever a piece has been successfully moved by player control
 */
static void _control_moved_piece(tetris_t *t) {

    // if the ground was hit last frame, then we unset the hit ground last frame
    // flag to give it another frame before it gets stuck (unless we have already
    // exceeded the maximum number of times we are allowed to do this or the
    // piece has not decreased the minimum global y value in sufficient time)
    if ((t->fp_data.falling_status & HIT_GROUND_LAST_FRAME) &&
            t->fp_data.ground_hit_count < MAX_GROUND_HIT_COUNT &&
            t->fp_data.min_h_inc_time < MAX_MIN_H_INC_TIME) {

        t->fp_data.falling_status &= ~HIT_GROUND_LAST_FRAME;
        t->fp_data.ground_hit_count++;
    }

}


static void _handle_event(tetris_t *t, key_event *ev) {

    // res is set if the falling piece was successfully moved to a new location
    // due to one of the following move/rotate methods
    int res = 0;

    if (ev->action == GLFW_PRESS) {
        switch (ev->key) {
            case GLFW_KEY_LEFT:
                res = _move_piece(t, -1, 0);
                t->ctrl.keypress_flags |= LEFT_KEY;
                break;
            case GLFW_KEY_RIGHT:
                res = _move_piece(t, 1, 0);
                t->ctrl.keypress_flags |= RIGHT_KEY;
                break;
            case GLFW_KEY_DOWN:
                t->ctrl.keypress_flags |= DOWN_KEY;
                break;
            case GLFW_KEY_A:
                res = _rotate_piece(t, ROTATE_COUNTERCLOCKWISE);
                break;
            case GLFW_KEY_D:
                res = _rotate_piece(t, ROTATE_CLOCKWISE);
                break;
        }
    }
    if (ev->action == GLFW_RELEASE) {
        switch (ev->key) {
            case GLFW_KEY_LEFT:
                t->ctrl.keypress_flags &= ~LEFT_KEY;
                t->ctrl.l_hold_count = 0;
                break;
            case GLFW_KEY_RIGHT:
                t->ctrl.keypress_flags &= ~RIGHT_KEY;
                t->ctrl.r_hold_count = 0;
                break;
            case GLFW_KEY_DOWN:
                t->ctrl.keypress_flags &= ~DOWN_KEY;
                break;
        }
    }

    // if a piece was successfully moved, then we need to register it in case
    // this has any affect on the falling piece data control
    if (res) {
        _control_moved_piece(t);
    }
}



static int _is_major_time_step(tetris_t *t) {
    return (t->time % t->major_tick_count) == 0;
}

static int _is_minor_time_step(tetris_t *t) {
    return (t->time % t->minor_tick_count) == 0;
}

/*
 * to be called every major time step, handles extra callbacks from held-down
 * keys and management of the controller state
 */
static void _handle_ctrl_callbacks(tetris_t *t) {
    controller *c = &t->ctrl;
    int res = 0;

    if (c->keypress_flags & LEFT_KEY) {
        if (c->l_hold_count == REPEAT_TIMER) {
            res |= _move_piece(t, -1, 0);
        }
        else {
            c->l_hold_count++;
        }
    }
    if (c->keypress_flags & RIGHT_KEY) {
        if (c->r_hold_count == REPEAT_TIMER) {
            res |= _move_piece(t, 1, 0);
        }
        else {
            c->r_hold_count++;
        }
    }
    if (c->keypress_flags & DOWN_KEY) {
        // only do this callback on exclusively minor time steps, since the
        // tile is moved down in major time steps by _advance
        if (!_is_major_time_step(t)) {
            res |= _move_piece(t, 0, -1);
        }
    }

    if (res) {
        _control_moved_piece(t);
    }
}



void tetris_step(tetris_t *t) {
    int could_advance;
    key_event ev;

    // first process key events
    while (key_event_queue_pop(&t->kq, &ev)) {
        _handle_event(t, &ev);
    }

    if (t->state != PLAY) {
        // don't advance time if not in the play state
        return;
    }


    if (_is_major_time_step(t)) {
        // advance game state
        could_advance = _advance(t);

        if (!could_advance) {
            // game is over
            t->state = GAME_OVER;
            printf("Game over\n");
        }

        // check to see if min y has increased
        if (t->fp_data.min_h > t->falling_piece.board_y) {
            t->fp_data.min_h = t->falling_piece.board_y;
            t->fp_data.min_h_inc_time = 0;
        }
        else {
            t->fp_data.min_h_inc_time++;
        }
    }
    else if (_is_minor_time_step(t)) {
        _handle_ctrl_callbacks(t);

    }


    t->time++;
}


