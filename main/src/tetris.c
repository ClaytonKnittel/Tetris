
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



static void _init_clear_animator(clear_animator *a) {
    __builtin_memset(a, 0, sizeof(clear_animator));
}


/*
 * cleares the next row of tiles in the clear animation and updates the state
 * of the clear animator
 */
static void _clear_next_row(tetris_t *t) {
    clear_animator *a = &t->c_anim;
    int8_t start_row = canim_get_start_row(a);

    int8_t l_col = a->l_col;
    int8_t r_col = a->r_col;

    // for each possible row being cleared (up to the height of the peice
    // box), check if it is in the set of lines to be cleared, and if so,
    // clear another column from it
    for (uint8_t i = 0; i < PIECE_BB_H; i++) {
        if (canim_is_row_idx_set(a, i)) {
            // row is in the set of rows to be cleared
            uint8_t r = start_row + i;

            // clear the columns if they are in range of the board
            if (l_col >= 0) {
                board_set_tile(&t->board, l_col, r, EMPTY);
            }
            if (r_col < t->board.width) {
                board_set_tile(&t->board, r_col, r, EMPTY);
            }
        }
    }

    // advance both columns
    canim_inc_cols(a);

}


/*
 * returns 1 if the clear animation has complete, 0 otherwise
 */
static int _clear_animation_done(tetris_t *t) {
    clear_animator *a = &t->c_anim;

    return a->l_col < 0 && a->r_col >= t->board.width;
}


/*
 * after clear animation has complete, goes through and drops down the rows
 * above the cleared row to fill in the gap and sets the game state back to
 * play
 */
static void _finish_clear_animation(tetris_t *t) {
    clear_animator *c_anim = &t->c_anim;

    int8_t row = canim_get_start_row(c_anim);
    int8_t dst_row = row;

    // topple down all rows that 
    for (int i = 0; i < PIECE_BB_H; i++) {

        if (!canim_is_row_idx_set(c_anim, i)) {
            // if this row is not set, then we have to move it down
            if (dst_row != row) {
                // if the current row is not empty and dst_row has already been
                // offset from row, we need to topple row down to dst_row
                board_copy_row(&t->board, dst_row, row);
            }

            dst_row++;
        }
        row++;
    }
    // topple down remaining rows
    for (; row < t->board.height; row++, dst_row++) {
        board_copy_row(&t->board, dst_row, row);
    }

    // clear the top most rows which were already toppled but not overwritten
    for (; dst_row < t->board.height; dst_row++) {
        board_clear_row(&t->board, dst_row);
    }

    // now may resume the game
    t->state = PLAY;
}


static void _init_scorer(scorer_t *s) {
    __builtin_memset(s, 0, sizeof(scorer_t));
}

/*
 * to be called after a piece is placed. This method takes the game state
 * struct, the number of rows cleared in the move, and the type of the piece
 * which was just placed
 */
static void _scorer_count_move(tetris_t *t, int32_t num_rows_cleared,
        uint8_t piece_type) {

    int t_spin = 0;
    int b2b = 0;

    if (piece_type == PIECE_T &&
            (t->scorer.status & SCORER_LAST_ACTION_WAS_ROTATE)) {

        // check the four diagonally adjacent tiles to center of T piece to see
        // if at least 3 are occupied
        // note: center of piece is at (1, 1), relative to piece coordinates

        int8_t x = t->falling_piece.board_x;
        int8_t y = t->falling_piece.board_y;

        int tot_occ = 0;
        tot_occ += board_get_tile(&t->board, x, y) != EMPTY;
        tot_occ += board_get_tile(&t->board, x + 2, y) != EMPTY;
        tot_occ += board_get_tile(&t->board, x, y + 2) != EMPTY;
        tot_occ += board_get_tile(&t->board, x + 2, y + 2) != EMPTY;

        if (tot_occ >= 3) {
            printf("T-Spin ");
            if (num_rows_cleared == 0) {
                printf("\n");
            }
            t_spin = 1;
        }
    }

    // difficult moves are either t spins with > 0 clears or tetris's
    if (t_spin || num_rows_cleared == 4) {
        if (t->scorer.status & SCORER_LAST_CLEAR_WAS_HARD) {
            printf("B2B ");
            b2b = 1;
        }
        t->scorer.status |= SCORER_LAST_CLEAR_WAS_HARD;
    }
    else {
        t->scorer.status &= ~SCORER_LAST_CLEAR_WAS_HARD;
    }


    if (num_rows_cleared > 0) {

        // if any lines were cleared, we have a combo of at least 1
        t->scorer.combo_len++;
        if (t->scorer.combo_len >= 2) {
            printf("Combo of length %d\n", t->scorer.combo_len);
        }


        int32_t pts;

        assert(num_rows_cleared <= 4);
        switch (num_rows_cleared) {
            case 1:
                printf("Single");
                pts = t_spin ? (b2b ? 1200 : 800) : 100;
                break;
            case 2:
                printf("Double");
                pts = t_spin ? (b2b ? 1800 : 1200) : 300;
                break;
            case 3:
                printf("Triple");
                pts = t_spin ? (b2b ? 2400 : 1600) : 500;
                break;
            case 4:
                printf("Tetris");
                assert(!t_spin);
                pts = b2b ? 1200 : 800;
                break;
        }

        if (t->scorer.combo_len >= 2) {
            pts += 50 * t->scorer.combo_len;
        }

        t->scorer.score += pts;

        printf("\n");
    }
    else {
        // if no lines were cleared, then we have to reset the combo count
        t->scorer.combo_len = 0;

        if (t_spin) {
            // 400 points for a t-spin without any lines cleared
            t->scorer.score += 400;
        }
    }
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

    _init_scorer(&t->scorer);
    
    // note: do not need to initialize clear animator, as it is only accessed
    // when in clear animation state, and will be initialized when we enter
    // that state

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



// forward declaration
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

    // we may loop once if a piece is placed and the new falling piece has to
    // be moved down one row. However, if in placing the piece we trigger an
    // animation (which would change the state of the game from PLAY), we do
    // not want to initialize the new piece or move it down
    while (t->state == PLAY) {

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

                // artificially advance time forward to
                // CTRL_HIT_GROUND_LAST_DELAY ticks before the next major time
                // step
                t->time += (t->major_tick_count - MIN(t->major_tick_count,
                            CTRL_HIT_GROUND_LAST_DELAY));
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

            // unset last action was rotate flag in scorer
            t->scorer.status &= ~SCORER_LAST_ACTION_WAS_ROTATE;

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

    // use placeholder clear animator to write data to while we are going, and
    // if there happens to be at least one row that needs to be clear, we will
    // copy this clear animator over to the actual clear animator and set the
    // game state to clear animation
    clear_animator c_anim_tmp;
    _init_clear_animator(&c_anim_tmp);

    // check the only rows that contain possibly modified tiles
    int32_t bot = MAX(t->falling_piece.board_y, 0);
    int32_t top = MIN(t->falling_piece.board_y + PIECE_BB_H, t->board.height);

    int32_t r;

    int32_t num_rows_cleared = 0;

    // if we end up needing to start the clear animation, the start row will be
    // bot (snap the bottom down to the 4th to last row if it is any higher,
    // so that every row in the bitvector will correspond to a real row on the
    // board)
    int32_t anim_bot = MIN(bot, t->board.height - PIECE_BB_H - 1);
    canim_set_start_row(&c_anim_tmp, anim_bot);

    for (r = bot; r < top; r++) {
        if (board_row_full(&t->board, r)) {
            canim_set_row_idx(&c_anim_tmp, r - anim_bot);
            num_rows_cleared++;
        }
    }

    _scorer_count_move(t, num_rows_cleared, t->falling_piece.piece_idx);

    if (num_rows_cleared > 0) {
        // if any rows were found to be clear, we have to do the clear animation!
        c_anim_tmp.l_col = 4;
        c_anim_tmp.r_col = 5;
        t->c_anim = c_anim_tmp;
        t->state = CLEAR_ANIMATION;
    }
}


/*
 * to be called whenever a piece has been successfully moved by player control
 */
static void _control_moved_piece(tetris_t *t, int move_type) {

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

    // update last action type in scorer
    if (move_type == MOVE_ROTATE) {
        t->scorer.status |= SCORER_LAST_ACTION_WAS_ROTATE;
    }
    else {
        t->scorer.status &= ~SCORER_LAST_ACTION_WAS_ROTATE;
    }

}


static void _handle_event(tetris_t *t, key_event *ev) {

    // res is set if the falling piece was successfully moved to a new location
    // due to one of the following move/rotate methods
    int res = 0;

    // type of move (either MOVE_TRANSLATE or MOVE_ROTATE)
    int type = 0;

    if (ev->action == GLFW_PRESS) {
        switch (ev->key) {
            case GLFW_KEY_LEFT:
                res = _move_piece(t, -1, 0);
                type = MOVE_TRANSLATE;
                t->ctrl.keypress_flags |= LEFT_KEY;
                break;
            case GLFW_KEY_RIGHT:
                res = _move_piece(t, 1, 0);
                type = MOVE_TRANSLATE;
                t->ctrl.keypress_flags |= RIGHT_KEY;
                break;
            case GLFW_KEY_UP:
                res = _rotate_piece(t, ROTATE_CLOCKWISE);
                type = MOVE_ROTATE;
                break;
            case GLFW_KEY_DOWN:
                t->ctrl.keypress_flags |= DOWN_KEY;
                break;
            case GLFW_KEY_A:
                res = _rotate_piece(t, ROTATE_COUNTERCLOCKWISE);
                type = MOVE_ROTATE;
                break;
            case GLFW_KEY_D:
                res = _rotate_piece(t, ROTATE_CLOCKWISE);
                type = MOVE_ROTATE;
                break;
        }
    }
    else if (ev->action == GLFW_RELEASE) {
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
        assert(type != 0);
        _control_moved_piece(t, type);
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
        // only translations can be repeated by holding down, so the only move
        // that could have been executed here is a translation
        _control_moved_piece(t, MOVE_TRANSLATE);
    }
}



void tetris_step(tetris_t *t) {
    int could_advance;
    key_event ev;

    // first process key events
    while (key_event_queue_pop(&t->kq, &ev)) {
        _handle_event(t, &ev);
    }

    switch (t->state) {
        case PLAY:

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

            break;

        case GAME_OVER:
            // don't advance time if the game is over
            return;

        case CLEAR_ANIMATION:
            // clear animation runs on minor time steps
            if (_is_minor_time_step(t)) {
                _clear_next_row(t);

                if (_clear_animation_done(t)) {
                    _finish_clear_animation(t);
                }
            }
            break;
        default:
            // cannot possibly get here
            __builtin_unreachable();
    }

    t->time++;
}


