
#include <string.h>

#include <math/combinatorics.h>
#include <util.h>

#include <tetris.h>
#include <tutil.h>




// forward declarations
static void _switch_state(tetris_t *t, int state);
static void _piece_placed(tetris_t *t);
static int _advance(tetris_state *s);





/*
 * returns 1 if controls are locked (no wall kicking, immediate sticking),
 * otherwise 0
 */
static int _lock_controls(tetris_t *t) {
    return t->game_state.fp_data.min_h_inc_time >= MAX_MIN_H_INC_TIME ||
           t->game_state.fp_data.ground_hit_count >= MAX_GROUND_HIT_COUNT;
}



static void _init_controller(controller *ctrl) {
    ctrl_reset(ctrl);
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
                board_set_tile(&t->game_state.board, l_col, r, EMPTY);
            }
            if (r_col < t->game_state.board.width) {
                board_set_tile(&t->game_state.board, r_col, r, EMPTY);
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

    return a->l_col < 0 && a->r_col >= t->game_state.board.width;
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
                board_copy_row(&t->game_state.board, dst_row, row);
            }

            dst_row++;
        }
        row++;
    }
    // topple down remaining rows
    for (; row < t->game_state.board.height; row++, dst_row++) {
        board_copy_row(&t->game_state.board, dst_row, row);
    }

    // clear the top most rows which were already toppled but not overwritten
    for (; dst_row < t->game_state.board.height; dst_row++) {
        board_clear_row(&t->game_state.board, dst_row);
    }

    // now may resume the game
    _switch_state(t, PLAY);
}





void tetris_init(tetris_t *t, msg_board_t *m, float x,
        float y, float screen_width, float screen_height) {

    tetris_state_init5(&t->game_state, x, y, screen_width,
            screen_height);

    _init_controller(&t->ctrl);

    t->m = m;

    // note: do not need to initialize clear animator, as it is only accessed
    // when in clear animation state, and will be initialized when we enter
    // that state

    _switch_state(t, PLAY);

    key_event_queue_init(&t->kq);

}




/*
 * switches the tetris game state to the given state, performing any necessary
 * state switching overhead
 */
static void _switch_state(tetris_t *t, int state) {

    t->game_state.state = state;

    switch (state) {
        case PLAY:
            // re-activate the controller
            ctrl_set_active(&t->ctrl);

            // need to fetch next falling piece and initialize it at the top,
            // as there must always be a falling piece while the controller is
            // active
            tetris_get_next_falling_piece(&t->game_state);

            break;
        case GAME_OVER:
            // set controller to inactive and clear any current state
            ctrl_reset(&t->ctrl);

            break;
        case CLEAR_ANIMATION:
            // set controller to inactive and clear any current state
            ctrl_reset(&t->ctrl);

            break;
        default:
            fprintf(stderr, "%d not a valid tetris state\n", state);
            assert(0);
    }
}


static int _rotate_piece(tetris_t *t, int rotation) {
    return tetris_rotate_piece(&t->game_state, rotation,
            !_lock_controls(t));
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
    int32_t bot = MAX(t->game_state.falling_piece.board_y, 0);
    int32_t top = MIN(t->game_state.falling_piece.board_y + PIECE_BB_H,
            t->game_state.board.height);

    int32_t r;

    int32_t num_rows_cleared = 0;

    // if we end up needing to start the clear animation, the start row will be
    // bot (snap the bottom down to the 4th to last row if it is any higher,
    // so that every row in the bitvector will correspond to a real row on the
    // board)
    int32_t anim_bot = MIN(bot, t->game_state.board.height - PIECE_BB_H - 1);
    canim_set_start_row(&c_anim_tmp, anim_bot);

    for (r = bot; r < top; r++) {
        if (board_row_full(&t->game_state.board, r)) {
            canim_set_row_idx(&c_anim_tmp, r - anim_bot);
            num_rows_cleared++;
        }
    }

    tetris_scorer_count_move(&t->game_state, num_rows_cleared, t->m);

    if (num_rows_cleared > 0) {
        // if any rows were found to be clear, we have to do the clear animation!
        c_anim_tmp.l_col = 4;
        c_anim_tmp.r_col = 5;
        t->c_anim = c_anim_tmp;
        _switch_state(t, CLEAR_ANIMATION);
    }
    else {
        // otherwise fetch the next piece and place it at the top of the board
        tetris_get_next_falling_piece(&t->game_state);
    }


    // unset the stale flag in the piece hold struct, since a piece has been placed
    t->game_state.hold.flags &= ~PIECE_HOLD_STALE;
}



/*
 * to be called whenever a piece has been successfully moved by player control
 */
static void _control_moved_piece(tetris_t *t) {

    // if the ground was hit last frame, then we unset the hit ground last frame
    // flag to give it another frame before it gets stuck (unless we have already
    // exceeded the maximum number of times we are allowed to do this or the
    // piece has not decreased the minimum global y value in sufficient time)
    if ((t->game_state.fp_data.falling_status & HIT_GROUND_LAST_FRAME) &&
            !_lock_controls(t)) {

        t->game_state.fp_data.falling_status &= ~HIT_GROUND_LAST_FRAME;
        t->game_state.fp_data.ground_hit_count++;
    }

}




static void _handle_event(tetris_t *t, key_event *ev) {

    // res is set if the falling piece was successfully moved to a new location
    // due to one of the following move/rotate methods
    int res = 0;

    if (ev->action == GLFW_PRESS) {
        switch (ev->key) {
            case GLFW_KEY_LEFT:
                if (ctrl_is_active(&t->ctrl)) {
                    res = tetris_move_piece(&t->game_state, -1, 0);
                }
                t->ctrl.keypress_flags |= LEFT_KEY;
                break;
            case GLFW_KEY_RIGHT:
                if (ctrl_is_active(&t->ctrl)) {
                    res = tetris_move_piece(&t->game_state, 1, 0);
                }
                t->ctrl.keypress_flags |= RIGHT_KEY;
                break;
            case GLFW_KEY_UP:
                if (ctrl_is_active(&t->ctrl)) {
                    res = _rotate_piece(t, ROTATE_CLOCKWISE);
                }
                break;
            case GLFW_KEY_DOWN:
                t->ctrl.keypress_flags |= DOWN_KEY;
                break;
            case GLFW_KEY_A:
                if (ctrl_is_active(&t->ctrl)) {
                    res = _rotate_piece(t, ROTATE_COUNTERCLOCKWISE);
                }
                break;
            case GLFW_KEY_D:
                if (ctrl_is_active(&t->ctrl)) {
                    res = _rotate_piece(t, ROTATE_CLOCKWISE);
                }
                break;
            case GLFW_KEY_LEFT_SHIFT:
                if (ctrl_is_active(&t->ctrl)) {
                    tetris_hold_piece(&t->game_state);
                }
                break;
            case GLFW_KEY_SPACE:
                if (ctrl_is_active(&t->ctrl)) {
                    tetris_hard_drop(&t->game_state);
                    // the piece is always successfully moved
                    res = 1;
                }
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
        _control_moved_piece(t);
    }
}


/*
 * to be called every major time step, handles extra callbacks from held-down
 * keys and management of the controller state
 */
static void _handle_ctrl_callbacks(tetris_t *t) {

    int is_major_ts = tetris_is_major_time_step(&t->game_state);
    int is_minor_ts = tetris_is_minor_time_step(&t->game_state);
    int is_keycb_ts = tetris_is_key_callback_step(&t->game_state);

    controller *c = &t->ctrl;
    int res = 0;

    if (is_keycb_ts && (c->keypress_flags & LEFT_KEY)) {
        if (ctrl_is_active(&t->ctrl) && c->l_hold_count == REPEAT_TIMER) {
            res |= tetris_move_piece(&t->game_state, -1, 0);
        }
        else {
            c->l_hold_count++;
        }
    }
    if (is_keycb_ts && (c->keypress_flags & RIGHT_KEY)) {
        if (ctrl_is_active(&t->ctrl) && c->r_hold_count == REPEAT_TIMER) {
            res |= tetris_move_piece(&t->game_state, 1, 0);
        }
        else {
            c->r_hold_count++;
        }
    }
    if ((is_minor_ts && !is_major_ts) && (c->keypress_flags & DOWN_KEY)) {
        // only do this callback on exclusively minor time steps, since the
        // tile is moved down in major time steps by _advance
        if (ctrl_is_active(&t->ctrl) && !is_major_ts) {
            res |= tetris_move_piece(&t->game_state, 0, -1);
            if (res) {
                // and make it a major time step again if the piece
                // successfully moved
                tetris_set_major_ts(&t->game_state);
            }
        }
    }

    if (res) {
        _control_moved_piece(t);
    }
}




void tetris_step(tetris_t *t) {
    int advance_status;
    key_event ev;

    // first process key events
    while (key_event_queue_pop(&t->kq, &ev)) {
        _handle_event(t, &ev);
    }

    switch (t->game_state.state) {
        case PLAY:

            if (tetris_is_major_time_step(&t->game_state)) {
                // advance game state
                do {
                    advance_status = tetris_advance(&t->game_state);

                    if (advance_status == ADVANCE_FAIL) {
                        // game is over
                        printf("Game over\n");
                    }
                    else if (advance_status == ADVANCE_PLACED_PIECE) {
                        // on a successful place, make this call to check for
                        // filled rows
                        _piece_placed(t);
                    }

                } while (advance_status == ADVANCE_PLACED_PIECE);

            }

            _handle_ctrl_callbacks(t);

            break;

        case GAME_OVER:
            // don't advance time if the game is over
            return;

        case CLEAR_ANIMATION:
            // clear animation runs on minor time steps
            if (is_canim_time_step(&t->c_anim)) {
                _clear_next_row(t);

                if (_clear_animation_done(t)) {
                    _finish_clear_animation(t);
                }
            }

            // one time step passes for clear animator
            canim_step(&t->c_anim);
            break;
        default:
            // cannot possibly get here
            __builtin_unreachable();
    }
}


/*
 * draws tetris game to screen
 */
void tetris_draw(tetris_t *t) {
    board_t *b = &t->game_state.board;

    if (t->game_state.state != CLEAR_ANIMATION) {
        piece_t fp = t->game_state.falling_piece;

        board_place_shadow(b, &fp);
        board_draw(b);
        board_remove_shadow(b, fp);
    }
    else {
        // during clear animation, don't draw the shadow
        board_draw(b);
    }
}


