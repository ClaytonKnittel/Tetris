
#include <math/combinatorics.h>

#include <tetris.h>
#include <tetris_state.h>
#include <util.h>
#include <tutil.h>



static void _init_fp_data(falling_piece_data *f) {
    f->falling_status = 0;
    f->ground_hit_count = 0;

    f->min_h_inc_time = 0;
    // set min_h to max value of height
    f->min_h = 0x7f;
}

static void _reset_fp_data(falling_piece_data *f) {
    f->falling_status = 0;
    f->ground_hit_count = 0;

    f->min_h_inc_time = 0;
    // set min_h to max value of height
    f->min_h = 0x7f;
}

/*
 * returns true if the falling piece has been set to stick wherever it is
 */
int tetris_piece_is_sticking(tetris_state *s) {
    return (s->fp_data.falling_status & STICK_NOW) != 0;
}


static void _init_piece_hold(piece_hold *p) {
    __builtin_memset(p, 0, sizeof(piece_hold));
}


static void _init_scorer(scorer_t *s) {
    __builtin_memset(s, 0, sizeof(scorer_t));
    s->level = 0;
    s->cleared_lines_threshhold = LINE_CLEAR_THRESH_INC;
}


static float _level_drop_rate(uint32_t level) {
    if (level < 9) {
        return 48 - 5 * level;
    }
    else if (level < 22) {
        return 6 - ((level + 2) / 3 - 3);
    }
    else if (level < 29) {
        return 2;
    }
    else {
        return 1;
    }
}


/*
 * to be called after a piece is placed. This method takes the game state
 * struct, the number of rows cleared in the move, and the type of the piece
 * which was just placed
 *
 * if msg board is non-null, then the a message will be sent to the board
 * whenever points are scored
 */
void tetris_scorer_count_move(tetris_state *s, int32_t num_rows_cleared,
        msg_board_t *m) {

    int t_spin = 0;
    int b2b = 0;

    uint8_t piece_type = s->falling_piece.piece_idx;

    char buf[MAX_MSG_LEN];

    if (m != NULL) {
        buf[0] = '\0';
    }

    if (piece_type == PIECE_T &&
            (s->scorer.status & SCORER_LAST_ACTION_WAS_ROTATE)) {

        // check the four diagonally adjacent tiles to center of T piece to see
        // if at least 3 are occupied
        // note: center of piece is at (1, 1), relative to piece coordinates

        int8_t x = s->falling_piece.board_x;
        int8_t y = s->falling_piece.board_y;

        int tot_occ = 0;
        tot_occ += board_get_tile(&s->board, x, y) != EMPTY;
        tot_occ += board_get_tile(&s->board, x + 2, y) != EMPTY;
        tot_occ += board_get_tile(&s->board, x, y + 2) != EMPTY;
        tot_occ += board_get_tile(&s->board, x + 2, y + 2) != EMPTY;

        if (tot_occ >= 3) {
            if (m != NULL) {
                snprintf(buf, MAX_MSG_LEN, "%sT-Spin ", buf);
            }
            t_spin = 1;
        }
    }

    // difficult moves are either t spins with > 0 clears or tetris's
    if (t_spin || num_rows_cleared == 4) {
        if (s->scorer.status & SCORER_LAST_CLEAR_WAS_HARD) {
            if (m != NULL) {
                snprintf(buf, MAX_MSG_LEN, "%sB2B ", buf);
            }
            b2b = 1;
        }
        s->scorer.status |= SCORER_LAST_CLEAR_WAS_HARD;
    }
    else if (num_rows_cleared > 0) {
        // if this was a clear, but not a hard one, unset the flag
        s->scorer.status &= ~SCORER_LAST_CLEAR_WAS_HARD;
    }


    if (num_rows_cleared > 0) {

        // if any lines were cleared, we have a combo of at least 1
        s->scorer.combo_len++;
        if (s->scorer.combo_len >= 2) {
            if (m != NULL) {
                snprintf(buf, MAX_MSG_LEN, "%sx%d ", buf, s->scorer.combo_len);
            }
        }


        int32_t pts;

        TETRIS_ASSERT(num_rows_cleared <= 4);
        switch (num_rows_cleared) {
            case 1:
                if (m != NULL) {
                    snprintf(buf, MAX_MSG_LEN, "%sSingle", buf);
                }
                pts = t_spin ? (b2b ? 1200 : 800) : 100;
                break;
            case 2:
                if (m != NULL) {
                    snprintf(buf, MAX_MSG_LEN, "%sDouble", buf);
                }
                pts = t_spin ? (b2b ? 1800 : 1200) : 300;
                break;
            case 3:
                if (m != NULL) {
                    snprintf(buf, MAX_MSG_LEN, "%sTriple", buf);
                }
                pts = t_spin ? (b2b ? 2400 : 1600) : 500;
                break;
            case 4:
                if (m != NULL) {
                    snprintf(buf, MAX_MSG_LEN, "%sTetris", buf);
                }
                TETRIS_ASSERT(!t_spin);
                pts = b2b ? 1200 : 800;
                break;
        }

        if (s->scorer.combo_len >= 2) {
            pts += 50 * s->scorer.combo_len;
        }

        s->scorer.score += pts * (s->scorer.level + 1);

        if (m != NULL) {
            snprintf(buf, MAX_MSG_LEN, "%s %d", buf, pts);
        }
    }
    else {
        // if no lines were cleared, then we have to reset the combo count
        s->scorer.combo_len = 0;

        if (t_spin) {
            // 400 points for a t-spin without any lines cleared
            s->scorer.score += 400 * (s->scorer.level + 1);
        }
    }

    s->scorer.cleared_lines += num_rows_cleared;

    if (s->scorer.cleared_lines > s->scorer.cleared_lines_threshhold) {
        s->scorer.cleared_lines_threshhold += LINE_CLEAR_THRESH_INC;
        s->scorer.level++;

        tetris_set_falling_speed(s, _level_drop_rate(s->scorer.level));
    }

    if (m != NULL) {
        msg_board_post(m, buf);
    }
}




void tetris_state_init(tetris_state *state, gl_context *context, float x,
        float y, float screen_width, float screen_height) {


    board_init(&state->board, TETRIS_WIDTH, TETRIS_HEIGHT);
    board_set_pos(&state->board, x, y);
    board_set_xscale(&state->board, screen_width);
    board_set_yscale(&state->board, screen_height);

    uint8_t *piece_queue = state->piece_queue;

    // first assign all pieces sequentially in the queue
    for (uint32_t i = 0; i < 2 * N_PIECES; i++) {
        piece_queue[i] = (i % N_PIECES) + 1;
    }
    state->queue_idx = 0;

    // then randomize both groups of 7
    permute(&piece_queue[0],        N_PIECES, sizeof(piece_queue[0]));
    permute(&piece_queue[N_PIECES], N_PIECES, sizeof(piece_queue[0]));

    // initialize falling piece to empty (no piece)
    piece_init(&state->falling_piece, EMPTY, 0, 0);

    _init_fp_data(&state->fp_data);

    _init_piece_hold(&state->hold);

    _init_scorer(&state->scorer);

    state->state = PLAY;

    // initialize time to 0
    state->time = 0LU;

    float init_drop_rate = _level_drop_rate(state->scorer.level);

    // to make set falling speed happy
    state->major_tick_count = 1.f;
    state->major_tick_time  = 0.f;
    tetris_set_falling_speed(state, init_drop_rate);

    state->key_callback_count = DEFAULT_HELD_KEY_PERIOD;
    state->key_callback_time  = 0.f;


}


void tetris_state_destroy(tetris_state *state) {
    board_destroy(&state->board);
}


void tetris_state_shallow_copy(tetris_state *dst, tetris_state *src) {
    __builtin_memcpy(dst, src, sizeof(tetris_state));
}


void tetris_state_deep_copy(tetris_state *dst, tetris_state *src) {
    tetris_state_shallow_copy(dst, src);
    board_deep_copy(&dst->board, &src->board);
}


int tetris_game_is_over(tetris_state *state) {
    return state->state == GAME_OVER;
}


void tetris_set_falling_speed(tetris_state *s, double period) {
    double prev_period = s->major_tick_count;
    // set time to same percentage of the way through current tick as before
    s->major_tick_time *= period / prev_period;
    s->major_tick_count = period;

    if (period <= DESIRED_MINOR_TICK_SPEED * 2) {
        s->minor_tick_count = period / 2;
    }
    else {
        uint32_t divisor =
            (uint32_t) roundf(period / DESIRED_MINOR_TICK_SPEED);
        TETRIS_ASSERT(divisor != 0);
        s->minor_tick_count = period / divisor;
    }

}


void tetris_set_level(tetris_state *s, uint32_t level) {
    s->scorer.level = level;
    tetris_set_falling_speed(s, _level_drop_rate(level));

    s->scorer.cleared_lines_threshhold = level < 10 ?
        level * 10 + 10 :
        MIN(200, MAX(100, level * 10 - 50));
}

/*
 * set current time to be a major time step (to be called when a piece is moved
 * down by player control, so that stick time is fixed from the moment the
 * piece touches the ground)
 */
void tetris_set_major_ts(tetris_state *s) {
    s->major_tick_time = 0;
}



static uint32_t _fetch_next_piece_idx(tetris_state *state) {
    uint32_t next;
    uint8_t *piece_queue = state->piece_queue;
    uint16_t queue_idx = state->queue_idx;
    
    // take the next piece from the queue
    next = piece_queue[queue_idx];
    queue_idx++;

    if (queue_idx == N_PIECES) {
        // if we just grabbed the last piece in a group of 7, move
        // the subsequent group of 7 down and generate a new group
        // 7 to follow it
        __builtin_memcpy(&piece_queue[0], &piece_queue[N_PIECES],
                N_PIECES * sizeof(piece_queue[0]));

        // generate next sequence and permute it
        for (uint32_t i = N_PIECES; i < 2 * N_PIECES; i++) {
            piece_queue[i] = (i % N_PIECES) + 1;
        }
        permute(&piece_queue[N_PIECES], N_PIECES,
                sizeof(piece_queue[0]));

        // reset queue index to reflect where the pieces moved
        queue_idx = 0;

    }

    state->queue_idx = queue_idx;
    return next;
}




/*
 * fetches next piece from the piece queue and places the piece at the top of
 * the board, to begin falling
 */
void tetris_get_next_falling_piece(tetris_state *state) {
    tetris_get_next_falling_piece_transient(state);
}


/*
 * similar to get_next_falling_piece, but populates the fp pointer passed with
 * the new piece
 */
void tetris_get_next_falling_piece_transient(tetris_state *state) {
    uint32_t next_piece_idx = _fetch_next_piece_idx(state);
    piece_init(&state->falling_piece, next_piece_idx, state->board.width,
            state->board.height);
}





/*
 * move the falling piece by dx, dy. If successful, the piece's location is
 * updated and 1 is returned, otherwise it is left where it was and 0 is
 * returned
 */
int tetris_move_piece(tetris_state *state, int dx, int dy) {
    int ret;

    piece_t falling = state->falling_piece;

    // first, remove the piece from the board where it is
    board_remove_piece(&state->board, falling);

    // then do a transient rotation
    ret = tetris_move_piece_transient(state, dx, dy);

    falling = state->falling_piece;
    // then place the new falling piece back on the board
    board_place_piece(&state->board, falling);

    return ret;
}


int tetris_move_piece_transient(tetris_state *state, int dx, int dy) {

    piece_t falling;
    piece_t new_falling;

    if (tetris_piece_is_sticking(state)) {
        // if the falling piece is sticking, disable all movement options
        return 0;
    }

    falling = state->falling_piece;

    // and now advance the piece to wherever it needs to go
    new_falling = falling;
    piece_move(&new_falling, dx, dy);

    // check to see if there would be any collisions here
    if (board_piece_collides(&state->board, new_falling)) {
        // then the piece cannot move
        return 0;
    }
    else {

        // otherwise, the piece can now be moved down into the new location
        state->falling_piece = new_falling;

        // update last move type in scorer
        state->scorer.status &= ~SCORER_LAST_ACTION_WAS_ROTATE;
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
int tetris_rotate_piece(tetris_state *state, int rotation,
        int allow_wall_kicks) {
    int ret;

    piece_t falling = state->falling_piece;

    // first, remove the piece from the board where it is
    board_remove_piece(&state->board, falling);

    // then do a transient rotation
    ret = tetris_rotate_piece_transient(state, rotation, allow_wall_kicks);

    falling = state->falling_piece;
    // then place the new falling piece back on the board
    board_place_piece(&state->board, falling);

    return ret;
}


int tetris_rotate_piece_transient(tetris_state *state, int rotation,
        int allow_wall_kicks) {


    piece_t falling;
    piece_t new_falling;
    int8_t dx, dy;

    if (tetris_piece_is_sticking(state)) {
        // if the falling piece is sticking, disable all movement options
        return 0;
    }

    falling = state->falling_piece;

    // and now advance the piece to wherever it needs to go
    new_falling = falling;
    piece_rotate(&new_falling, rotation);

    if (falling.piece_idx == PIECE_O || !allow_wall_kicks) {
        // either this piece cannot be rotated, so we don't loop, or the
        // controls are locked (to prevent spamming spin), so we don't allow
        // wall kicks

        // check to see if there would be any collisions here
        if (!board_piece_collides(&state->board, new_falling)) {
            state->falling_piece = new_falling;

            // update last action in scorer
            state->scorer.status |= SCORER_LAST_ACTION_WAS_ROTATE;
            return 1;
        }
    }
    else {
        for_each_displacement_trial(falling.piece_idx, falling.orientation,
                rotation, dx, dy) {

            piece_move(&new_falling, dx, dy);

            // check to see if there would be any collisions here
            if (!board_piece_collides(&state->board, new_falling)) {
                // the piece can now be moved down into the new location
                state->falling_piece = new_falling;

                // update last action in scorer
                state->scorer.status |= SCORER_LAST_ACTION_WAS_ROTATE;
                return 1;
            }

            // move it back before next iteration
            piece_move(&new_falling, -dx, -dy);
        }
    }

    // there were no suitable locations for the piece
    return 0;

}



/*
 * performs hold action, which takes the currently falling piece and places it
 * in the hold slot. If there was already a piece in the hold slot, it becomes
 * the next falling piece and the piece queue does not move. Otherwise, if
 * the hold slot was empty, the next piece is immediately chosen off the queue
 */
void tetris_hold_piece(tetris_state *s) {
    // take the falling piece off the board
    board_remove_piece(&s->board, s->falling_piece);

    if (tetris_hold_piece_transient(s) == 0) {
        // if the piece could not be held, then put it back
        board_place_piece(&s->board, s->falling_piece);
    }
}


int tetris_hold_piece_transient(tetris_state *s) {

    uint8_t held_piece;

    if (s->hold.flags & PIECE_HOLD_STALE ||
            tetris_piece_is_sticking(s)) {
        // if the hold is stale (a hold operation already happened after the
        // last placement), or the falling piece has been set to stick, then
        // ignore the request
        return 0;
    }

    held_piece = s->hold.piece_idx;

    s->hold.piece_idx = s->falling_piece.piece_idx;

    // set the stale flag so this operation can't be performed again until a
    // piece is placed
    s->hold.flags |= PIECE_HOLD_STALE;

    if (held_piece != EMPTY) {
        piece_init(&s->falling_piece, held_piece, s->board.width,
                s->board.height);
    }
    else {
        // need to grab the next falling piece from the queue
        tetris_get_next_falling_piece_transient(s);
    }

    return 1;
}


/*
 * performs a hard drop, which instantly places the falling piece
 */
void tetris_hard_drop(tetris_state *state) {

    piece_t falling = state->falling_piece;

    // first, remove the piece from the board where it is
    board_remove_piece(&state->board, falling);

    // then do a transient rotation
    tetris_hard_drop_transient(state);

    falling = state->falling_piece;
    // then place the new falling piece back on the board
    board_place_piece(&state->board, falling);
}

/*
 * similar to hard drop, but expects the falling piece to not be on the
 * board, and does not place the new falling piece on the board
 */
void tetris_hard_drop_transient(tetris_state *state) {
    piece_t fp = state->falling_piece;

    while (!board_piece_collides(&state->board, fp)) {
        fp.board_y--;
    }
    fp.board_y++;

    state->falling_piece = fp;

    // make the piece stick on the next major time step and disable any other
    // player inputs from moving the piece
    tetris_stick(state);
}





int tetris_clear_lines(tetris_state *state) {
    // check the only rows that contain possibly modified tiles
    int32_t bot = MAX(state->falling_piece.board_y, 0);
    int32_t top = MIN(state->falling_piece.board_y + PIECE_BB_H,
            state->board.height);

    int32_t r, dst_r;

    int32_t num_rows_cleared = 0;

    for (r = bot, dst_r = bot; r < top;) {
        if (board_row_full(&state->board, r)) {
            num_rows_cleared++;
        }
        else {
            dst_r++;
        }
        r++;

        if (dst_r != r) {
            board_copy_row(&state->board, dst_r, r);
        }
    }

    if (num_rows_cleared > 0) {
        // topple down remaining rows
        for (; r < state->board.height; r++, dst_r++) {
            board_copy_row(&state->board, dst_r, r);
        }

        // clear the top most rows which were already toppled but
        // not overwritten
        for (; dst_r < state->board.height; dst_r++) {
            board_clear_row(&state->board, dst_r);
        }
    }

    // register the line clear to the scorer
    tetris_scorer_count_move(state, num_rows_cleared, 0);

    return num_rows_cleared;
}


/*
 * makes the falling piece stick wherever it is, disabling any other inputs
 * from moving the piece until it sticks
 */
void tetris_stick(tetris_state *state) {
    // make the piece stick on the next major time step
    state->fp_data.falling_status |= STICK_NOW;

    // and make it a major time step again so the piece immediately sticks
    tetris_set_major_ts(state);
}



static uint64_t _ticks_to_next(float tick_count, float tick_time) {
    if (tick_time < 0) {
        // negative values are "dead time", they are used for short pauses in
        // gameplay
        return (uint64_t) ceil(-tick_time);
    }
    int64_t res = (int64_t) ceil(tick_count - fmod(tick_time, tick_count));
    uint64_t ans = (uint64_t) MAX(res, 1);
    TETRIS_ASSERT(fmod((tick_time + ans), tick_count) < 1);
    return ans;
}


void tetris_tick(tetris_state *s) {
    s->time++;

    float thresh = MAX(s->major_tick_count, s->minor_tick_count);
    s->major_tick_time++;
    while (s->major_tick_time >= thresh) {
        s->major_tick_time -= thresh;
    }

    s->key_callback_time++;
    while (s->key_callback_time >= s->key_callback_count) {
        s->key_callback_time -= s->key_callback_count;
    }
}


static void _tick_by(tetris_state *s, uint64_t ticks) {
    s->time += ticks;

    float thresh = MAX(s->major_tick_count, s->minor_tick_count);
    s->major_tick_time += ticks;
    while (s->major_tick_time >= thresh) {
        s->major_tick_time -= thresh;
    }

    s->key_callback_time += ticks;
    while (s->key_callback_time >= s->key_callback_count) {
        s->key_callback_time -= s->key_callback_count;
    }

}


/*
 * advances game state forward by the given number of ticks, which may cause
 * the game state to change (i.e. if gravity moves a piece or something)
 * If the falling piece stuck at some point, the number of ticks remaining
 * is written back into ticks, and 1 is returned. Otherwise, if the piece was
 * not placed and the game was able to advance by ticks, then 0 is returned
 */
int tetris_advance_by_transient(tetris_state *state, uint64_t *ticks) {
    uint64_t diff;
    int ret = 0;

    uint64_t t = *ticks;

    while (t > 0) {
        // number of ticks until next major time step
        diff = _ticks_to_next(state->major_tick_count, state->major_tick_time);

        if (diff <= t) {
            _tick_by(state, diff);

            // we moved to a major time step, so advance the game state
            int res = tetris_advance_transient(state);
            t -= diff;

            if (res == ADVANCE_PLACED_PIECE || res == ADVANCE_FAIL) {
                // if either a piece was placed or the game ended, we stop
                // advancing
                ret = 1;
                break;
            }
        }
        else {
            _tick_by(state, t);
            t = 0;
        }
    }

    *ticks = t;

    return ret;
}

/*
 * advances the game state until the falling piece either falls by one tile or
 * sticks to the ground
 *
 * returns 1 if the piece stuck
 * returns 0 if the piece dropped
 */
int tetris_advance_until_drop_transient(tetris_state *state) {

    piece_t prev_piece;
    int ret;

    do {
        uint64_t ticks_to_next_major_ts =
            _ticks_to_next(state->major_tick_count, state->major_tick_time);

        prev_piece = state->falling_piece;

        ret = tetris_advance_by_transient(state, &ticks_to_next_major_ts);

        // loop until either the piece sticks or the piece moves
    } while (ret == 0 && piece_equals(prev_piece, state->falling_piece));

    return ret;
}



int tetris_is_major_time_step(tetris_state *s) {
    float major_tick_time = fmod(s->major_tick_time, s->major_tick_count);
    return s->major_tick_time >= 0 && major_tick_time < 1;
}

int tetris_is_minor_time_step(tetris_state *s) {
    float minor_tick_time = fmod(s->major_tick_time, s->minor_tick_count);
    return s->major_tick_time >= 0 && minor_tick_time < 1;
}

int tetris_is_key_callback_step(tetris_state *s) {
    return s->key_callback_time < 1;
}



void tetris_advance_to_next_action(tetris_state *s) {
    do {
        tetris_tick(s);
    } while (!tetris_is_major_time_step(s) &&
             !tetris_is_minor_time_step(s) &&
             !tetris_is_key_callback_step(s));
}

int tetris_advance_to_next_minor_time_step(tetris_state *s) {
    int ret;
    float minor_tick_time;
    uint64_t ticks_to_next_minor_ts;

    if (s->major_tick_count <= s->minor_tick_count) {
        // if every minor tick happens on a major tick, then there are no minor
        // time steps which are not major time steps
        return 1;
    }

    do {
        minor_tick_time = s->major_tick_time;
        ticks_to_next_minor_ts =
            _ticks_to_next(s->minor_tick_count, minor_tick_time);
        ret = tetris_advance_by_transient(s, &ticks_to_next_minor_ts);
    } while(ret == 0 && (tetris_is_major_time_step(s) || !tetris_is_minor_time_step(s)));

    TETRIS_ASSERT(tetris_is_minor_time_step(s) || ret == 1);

    return ret;
}


/*
 * advances game state by one step. If it was successfully able to do so, then
 * 1, 2 or 3 is returned, otherwise, if the game ended due to a game over, 0 is
 * returned
 *
 * should only be called on major time steps
 */
int tetris_advance(tetris_state *s) {

    if (s->state == GAME_OVER) {
        // don't advance
        return ADVANCE_FAIL;
    }

    // first, remove the piece from the board where it is
    board_remove_piece(&s->board, s->falling_piece);

    int ret = tetris_advance_transient(s);

    if (ret != ADVANCE_FAIL) {
        board_place_piece(&s->board, s->falling_piece);
    }

    return ret;
}

int tetris_advance_transient(tetris_state *s) {
    piece_t falling;

    if (s->state == GAME_OVER) {
        // don't advance
        return ADVANCE_FAIL;
    }

    falling = s->falling_piece;

    // the falling piece must always have been initialized before this point
    TETRIS_ASSERT(falling.piece_idx != EMPTY);

    // now to try to move the piece down

    // and now advance the piece downward
    piece_t new_falling = falling;
    piece_move(&new_falling, 0, -1);

    // check to see if there would be any collisions here
    if (board_piece_collides(&s->board, new_falling)) {
        // then the piece cannot move down, it is now stuck where it was.

        if (s->fp_data.falling_status & HIT_GROUND_LAST_FRAME) {
            // if the piece spend two successive frames hitting the ground,
            // we stick it wherever it is
            _reset_fp_data(&s->fp_data);

            // try to see if the piece can be stuck down here
            int placed = board_can_place_piece(&s->board, falling);

            if (!placed) {
                // if we could not place this piece even partially on the
                // board, then the game is over
                s->state = GAME_OVER;
                return ADVANCE_FAIL;
            }

            // the piece can be placed
            return ADVANCE_PLACED_PIECE;
        }
        else {
            // if we were not hitting the ground last frame, then we set
            // this flag and allow the player to try moving the piece again
            // before it sticks
            s->fp_data.falling_status |= HIT_GROUND_LAST_FRAME;


            float mtc = s->major_tick_count;
            // artificially advance time forward to
            // CTRL_HIT_GROUND_LAST_DELAY% of major time step delay before
            // the next major time step
            if (mtc >= MAX_CTRL_GROUND_HIT_DELAY -
                       MIN_CTRL_GROUND_HIT_DELAY) {
                // if major tick count is greater than the max delay, then we
                // have already waited long enough for the stick to piece, we
                // can make it stick next frame
                s->major_tick_time = -MIN_CTRL_GROUND_HIT_DELAY;
            }
            else if (mtc <= MAX_CTRL_GROUND_HIT_DELAY -
                            CTRL_HIT_GROUND_LAST_DELAY) {
                // otherwise if major tick count is smaller than the difference
                // between the ground hit delay and the max ground hit delay,
                // delay as much as possible
                s->major_tick_time = -CTRL_HIT_GROUND_LAST_DELAY;
            }
            else {
                // otherwise, linearly interpolate between the two above cases
                s->major_tick_time = (mtc - MAX_CTRL_GROUND_HIT_DELAY);
            }

            return ADVANCE_STALLED;
        }
    }
    else {
        // otherwise, the piece can now be moved down into the new location
        s->falling_piece = new_falling;

        // unset hit ground last frame flag, in case it was set and the
        // piece was subsequently moved off the platform
        s->fp_data.falling_status &= ~HIT_GROUND_LAST_FRAME;
        s->fp_data.ground_hit_count = 0;

        // check to see if min y has increased
        if (s->fp_data.min_h > s->falling_piece.board_y) {
            s->fp_data.min_h = s->falling_piece.board_y;
            s->fp_data.min_h_inc_time = 0;
        }
        else {
            s->fp_data.min_h_inc_time =
                MIN(s->fp_data.min_h_inc_time + 1, MAX_MIN_H_INC_TIME);
        }

        // unset last action was rotate flag in scorer
        s->scorer.status &= ~SCORER_LAST_ACTION_WAS_ROTATE;

        // that is the completion of this move
        return ADVANCE_MOVED_PIECE;
    }
}
