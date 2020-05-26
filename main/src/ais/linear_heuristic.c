
#include <math.h>
#include <stdlib.h>

#include <util.h>
#include <data_structs/min_heap.h>

#include <tetris.h>
#include <tetris_state.h>
#include <ai.h>
#include <ais/linear_heuristic.h>
#include <tutil.h>


// number of rows to add to the top of the board to allow for pieces that are
// rotated off the top of the screen or something
#define CEIL_BUFFER 2

// number of moves the AI is to look ahead to (1 means only think about current
// move)
#define DEFAULT_DEPTH 4

// consider only best n possible landing locations according to heuristic,
// lowers branching factor
#define DEFAULT_BEST_N 4



lha_t * linear_heuristic_agent_init() {
    lha_t * agent = (lha_t *) calloc(1, sizeof(lha_t));

    agent->depth = DEFAULT_DEPTH;
    agent->best_n = DEFAULT_BEST_N;

    return agent;
}


void linear_heuristic_agent_destroy(lha_t *a) {
    free(a);
}


static void print_board(tetris_state * s) {
    piece_t fp = s->falling_piece;
    board_t * board = &s->board;

    for (int y = TETRIS_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < TETRIS_WIDTH; x++) {
            if (piece_contains(fp, x, y)) {
                printf("X ");
            }
            else {
                uint8_t tile = board_get_tile(board, x, y);
                printf("%c ", (tile ? 'O' : '.'));
            }
        }
        printf("\n");
    }
}




static int _get_tile(tetris_state *s, int8_t x, int8_t y) {
    if (board_get_tile(&s->board, x, y)) {
        return 1;
    }
    return piece_contains(s->falling_piece, x, y);
}



// goal of AI is to maximize this value
static float heuristic(tetris_state *s) {
    uint32_t aggregate_height = 0;
    //a hole is defined as an empty tile with the tile above it non-empty
    uint32_t n_holes = 0;

    uint32_t n_complete_lines = 0;

    // cumulative sum of absolute differences in adjacent column heights
    uint32_t bumpiness = 0;


    // height of each column, initialize all elements to 0
    uint8_t heights[TETRIS_WIDTH] = { 0 };


    if (tetris_game_is_over(s)) {
        return -INFINITY;
    }


    for (int8_t col = 0; col < TETRIS_WIDTH; col++) {
        for (int8_t row = TETRIS_HEIGHT - 1; row >= 0; row--) {
            if (_get_tile(s, col, row) && heights[col] == 0) {
                // first tile in column that is occupied (from top to bottom)
                // gives the height of the column
                heights[col] = row + 1;
            }
        }
    }

    for (int8_t col = 0; col < TETRIS_WIDTH; col++) {
        aggregate_height += heights[col];
        if (col > 0) {
            bumpiness += abs(heights[col - 1] - heights[col]);
        }
    }

    for (int8_t row = 0; row < TETRIS_HEIGHT; row++) {
        int line_complete = 1;
        for (int8_t col = 0; col < TETRIS_WIDTH; col++) {
            n_holes += (!_get_tile(s, col, row) &&
                         _get_tile(s, col, row + 1));
            line_complete = line_complete && _get_tile(s, col, row);
        }
        n_complete_lines += line_complete;
    }

    static const float a = -.510066f,
                       b =  .760666f,
                       c = -.35663f,
                       d = -.184483f;

    float h = a * aggregate_height + b * n_complete_lines +
        c * n_holes + d * bumpiness;

    return h;
}


// actions (can be a combination of the GO's and ROTATE's)
#define GO_LEFT   0x1
#define GO_RIGHT  0x2
#define GO_DOWN   0x4
// rotate clockwise
#define ROTATE_C  0x8
// rotate counterclockwise
#define ROTATE_CC 0x10

// wait until the falling piece on the board is no longer equal to the falling
// piece of this node
#define WAIT      0x20

#define INFTY 0x7fffffffffffffffL


typedef struct state_node {
    /*
     * number of ticks to get to this position, i.e. dist, is the key in node
     * this value is used to keep track of when it is a major time step
     *
     * we put the time of the state in the upper 56 bits of the key in the heap, and
     * we put the number of keystrokes made along the path in the lower 8 bits
     */
    heap_node node;

    // snapshot of the game state at this particular state node
    tetris_state game_state;

    // time at which to perform the action, must be at or before
    // game_state.time
    uint64_t cb_time;

    // index of state node preceeding this one on shortest path
    int32_t parent_idx;
    // action taken to get here from parent
    int action;

    // singly linked list of possible falling spots
    struct state_node * next;
} state_node;


static uint64_t get_node_time(state_node * node) {
    return ((uint64_t) node->node.key) >> 8;
}


static state_node * __heap_node_to_state_node(heap_node * node) {
    return (state_node *) (((uint64_t) node) - offsetof(state_node, node));
}


// denotes end of list, we require pointers to be 8-byte aligned so this won't
// cause any ambiguity
#define LIST_END ((void*) 0x1)


typedef struct state {
    state_node * m;

    // current time in tetris state object
    uint64_t t0;

    // heap of states for dijkstra
    heap_t h;

    // maintain singly-linked list of states which are possible landing
    // locations (if this is NULL, then it is not in the list, and we mark the
    // end of the list with LIST_END instead of NULL)
    struct state_node * falling_spots;

    // pointer to game state
    tetris_state * game_state;
} state_t;



/*
 * gives index in state array where the state with falling piece = p is
 */
static uint32_t _find_idx(tetris_state * game_state) {
    int8_t x, y;

    piece_t p = game_state->falling_piece;

    // find bottom left corner of piece, which must be in bounds of the the
    // board
    piece_bottom_left_corner(p, &x, &y);

    TETRIS_ASSERT(x < TETRIS_WIDTH && y < TETRIS_HEIGHT + CEIL_BUFFER);

    uint32_t idx =
        (y * TETRIS_WIDTH + x) * N_PIECE_ORIENTATIONS + p.orientation;

    return idx;
}


/*
 * gives a pointer to the state node struct in the state array, given the
 * falling piece's location
 */
static state_node * __find_state_node(state_t * s, tetris_state * game_state) {
    uint32_t idx = _find_idx(game_state);
    return &s->m[idx];
}



/*
 * discover new path to new_state, where action is performed at time "cb_time"
 * and the preceding state is parent_idx
 */
static void __try_decrease(state_t * s, tetris_state * new_state,
        uint64_t cb_time, uint32_t parent_idx, int action) {

    uint64_t new_time = new_state->time;

    state_node * node = __find_state_node(s, new_state);

    // we will be using lower 8 bits of key to store number of keystrokes
    TETRIS_ASSERT(new_time < 0x0080000000000000);

    uint8_t parent_key_strokes;
    if (parent_idx == -1) {
        parent_key_strokes = 0;
    }
    else {
        state_node * parent = &s->m[parent_idx];
        parent_key_strokes = (parent->node.key & 0xff);

        // if the action is a keystroke (i.e. not a wait), then add one to the
        // key stroke count
        parent_key_strokes += (action != WAIT);

        TETRIS_ASSERT(parent_key_strokes < 0xff);
    }

    uint64_t key_val = (new_time << 8) | parent_key_strokes;

    if (node->node.key == INFTY) {
        HEAP_NODE_SET(&node->node, key_val);
        heap_insert(&s->h, &node->node);
    }
    else if (node->node.key > key_val) {
        heap_decrease_key(&s->h, &node->node, key_val);
    }
    else {
        return;
    }

    // if the node was decreased, update its fields
    tetris_state_shallow_copy(&node->game_state, new_state);
    node->cb_time = cb_time;
    node->parent_idx = parent_idx;
    node->action = action;
}


/*
 * append node to list of possible falling spots if it is not already in there,
 * otherwise do nothing
 */
static void __try_falling_spot_append(state_t * s, state_node * node) {
    if (node->next == NULL) {
        // only add node to the list if it's not already in the list
        state_node * head = s->falling_spots;
        node->next = head;
        s->falling_spots = node;
    }
}


/*
 * run dijkstra's algorithm on s and find 
 */
static void _run_dijkstra(state_t *s) {
    state_node * node;
    tetris_state * game_state, new_state;
    uint64_t time = 0;

    while ((node = __heap_node_to_state_node(heap_extract_min(&s->h))) !=
            NULL) {
        game_state = &node->game_state;
        //print_board(game_state);

        // discovered nodes must be non-decreasing in time
        TETRIS_ASSERT(node->game_state.time >= time);

        time = game_state->time;
        //printf("Extracted %p %d (%f)\n", node, fp.piece_idx, (time - s->t0) / 60.f);
        uint32_t parent_idx = _find_idx(game_state);

        // find all possible successors of this node

        // wait for it to fall
        tetris_state_shallow_copy(&new_state, game_state);
        // advance game until the piece drops
        int ret = tetris_advance_until_drop_transient(&new_state);
        if (ret == 1) {
            // this piece can stick
            state_node * falling_spot = __find_state_node(s, &new_state);
            __try_falling_spot_append(s, falling_spot);
        }
        else {
            // if the piece did not stick, it must have moved
            TETRIS_ASSERT(!piece_equals(game_state->falling_piece,
                        new_state.falling_piece));
            // wait until the gravity would drop the piece (new_state.time)
            __try_decrease(s, &new_state, new_state.time, parent_idx, WAIT);
        }

        // KEYBOARD INPUTS:

        // translations:
        // press left
        tetris_state_shallow_copy(&new_state, game_state);
        if (tetris_move_piece_transient(&new_state, -1, 0)) {
            // advance game by input delay ticks
            uint64_t adv = AI_INPUT_DELAY;
            if (tetris_advance_by_transient(&new_state, &adv) == 0) {
                // only add to the list of reachable states if the move is
                // possible
                __try_decrease(s, &new_state, time, parent_idx, GO_LEFT);
            }
        }

        // press right
        tetris_state_shallow_copy(&new_state, game_state);
        if (tetris_move_piece_transient(&new_state, 1, 0)) {
            // advance game by input delay ticks
            uint64_t adv = AI_INPUT_DELAY;
            if (tetris_advance_by_transient(&new_state, &adv) == 0) {
                // only add to the list of reachable states if the move is
                // possible
                __try_decrease(s, &new_state, time, parent_idx, GO_RIGHT);
            }
        }

        // press down
        tetris_state_shallow_copy(&new_state, game_state);
        // can only move down on minor time steps
        if ((tetris_is_minor_time_step(&new_state) &&
                    !tetris_is_major_time_step(&new_state)) ||
                tetris_advance_to_next_minor_time_step(&new_state) == 0) {

            abort();

            if (tetris_move_piece_transient(&new_state, 0, -1)) {

                uint64_t down_time = new_state.time;

                // advance game by input delay ticks
                uint64_t adv = AI_INPUT_DELAY;
                if (tetris_advance_by_transient(&new_state, &adv) != 0) {
                    // could not advance because either the game ended or 
                }
                else {
                    //TETRIS_ASSERT(piece_equals(cur_piece, new_state.falling_piece));
                    // only allow pressing down if gravity will not be moving
                    // the falling piece down
                    __try_decrease(s, &new_state, down_time, parent_idx,
                            GO_DOWN);
                }
            }
        }

        // press rotate clockwise
        tetris_state_shallow_copy(&new_state, game_state);
        if (tetris_rotate_piece_transient(&new_state, ROTATE_CLOCKWISE, 1)) {
            // advance game by input delay ticks
            uint64_t adv = AI_INPUT_DELAY;
            if (tetris_advance_by_transient(&new_state, &adv) != 0) {
                // could not advance because either the game ended or 
            }
            else {
                __try_decrease(s, &new_state, time, parent_idx, ROTATE_C);
            }
        }

        // press rotate counterclockwise
        tetris_state_shallow_copy(&new_state, game_state);
        if (tetris_rotate_piece_transient(&new_state,
                    ROTATE_COUNTERCLOCKWISE, 1)) {
            // advance game by input delay ticks
            uint64_t adv = AI_INPUT_DELAY;
            if (tetris_advance_by_transient(&new_state, &adv) != 0) {
                // could not advance because either the game ended or 
            }
            else {
                __try_decrease(s, &new_state, time, parent_idx, ROTATE_CC);
            }
        }

    }

}



/*
 * traces node back to the starting point and constructs a singly linked list
 * of nodes from the starting point to node (using the next field in node), and
 * returning a pointer to the head of the list
 *
 * the list is NULL terminated
 */
state_node * _construct_path_to(state_t *s, state_node *node) {
    
    state_node * prev = NULL;
    // the last action is always a wait, since we have to let the piece stick
    // into place
    int prev_action = WAIT;
    // wait until the final state of the game, which is wherever the game state
    // ends after performing the action
    uint64_t prev_cb_time = node->game_state.time;

    while (1) {
        node->next = prev;
        prev = node;

        // action is now the action needed to be taken to get from this node
        // to the next node, not the action to get here from the previous
        uint64_t next_cb_time = node->cb_time;
        node->cb_time = prev_cb_time;
        prev_cb_time = next_cb_time;
        int next_action = node->action;
        node->action = prev_action;
        prev_action = next_action;

        int32_t parent_idx = node->parent_idx;
        if (parent_idx == -1) {
            break;
        }
        node = &s->m[parent_idx];
    }

    return prev;
}


static int _at_top_level(lha_t *a, int depth) {
    return depth == a->depth;
}


static float _find_best_path(lha_t *a, tetris_state *s, int depth);



/*
 * make a deep copy of the board and recursively call find best path
 */
static float _depth_find(lha_t *a, state_t *s, state_node *fs, int depth) {
    tetris_state tmp;

    // make deep copy of the game state at state_node fs
    tetris_state_deep_copy(&tmp, &fs->game_state);

    // save the current falling piece, since it will be changed
    piece_t old_fp = tmp.falling_piece;

    // put the falling piece on the board
    board_place_piece(&tmp.board, old_fp);
    // clear any lines cleared by this piece
    tetris_clear_lines(&tmp);
    // fetch the next falling piece
    tetris_get_next_falling_piece_transient(&tmp);

    // recursively call find best path on the new state
    float h = _find_best_path(a, &tmp, depth - 1);

    tetris_state_destroy(&tmp);

    return h;
}


static state_node * _find_best_n(state_node * falling_spots, int n) {
    struct sh {
        state_node * node;
        float h;
    } * best_n;

    best_n = (struct sh *) calloc(n, sizeof(struct sh));

    for (state_node * fs = falling_spots; fs != LIST_END; fs = fs->next) {
        float h = heuristic(&fs->game_state);

        for (uint32_t i = 0; i < n; i++) {
            struct sh * tmp = &best_n[i];

            if (tmp->node == NULL) {
                // take the spot
                tmp->node = fs;
                tmp->h = h;
                break;
            }
            else if (tmp->h < h) {
                // supercede this spot
                state_node * swp = fs;
                do {
                    struct sh old = best_n[i];

                    best_n[i].node = swp;
                    best_n[i].h = h;

                    swp = old.node;
                    h = old.h;
                    i++;
                } while (i < n && swp != NULL);
            }
        }
    }

    // link up best n in a list
    uint32_t i;
    for (i = 0; i < n - 1 && best_n[i + 1].node != NULL; i++) {
        best_n[i].node->next = best_n[i + 1].node;
    }
    best_n[i].node->next = LIST_END;

    return best_n[0].node;
}


/*
 * go through list of possible landing spots and choose which one makes
 * heuristic highest
 *
 * returns the heuristic value of the best landing spot
 */
static float _choose_best_dst(lha_t *a, state_t *s, int depth) {

    state_node * best = NULL;
    float max_h = -INFINITY;

    if (a->best_n == -1) {

        for (state_node * fs = s->falling_spots; fs != LIST_END; fs = fs->next) {

            float h;
            if (depth == 1) {
                h = heuristic(&fs->game_state);
            }
            else {
                h = _depth_find(a, s, fs, depth);
            }

            /*if (_at_top_level(a, depth)) {
                printf("%f\n", h);
                print_board(&fs->game_state);
            }*/

            if (h > max_h) {
                max_h = h;
                best = fs;
            }
        }
    }
    else {
        // find best n places to land
        state_node * best_n = _find_best_n(s->falling_spots, a->best_n);

        for (state_node * fs = best_n; fs != LIST_END; fs = fs->next) {

            float h;
            if (depth == 1) {
                h = heuristic(&fs->game_state);
            }
            else {
                h = _depth_find(a, s, fs, depth);
            }

            /*if (_at_top_level(a, depth)) {
                printf("%f\n", h);
                print_board(&fs->game_state);
            }*/

            if (h > max_h) {
                max_h = h;
                best = fs;
            }

        }
    }

    if (_at_top_level(a, depth) && best != NULL) {
        state_node * path = _construct_path_to(s, best);
        a->__int_state.action_list = path;
    
        /*printf("Path:\n");
        for (state_node * fs = path; fs != NULL; fs = fs->next) {
            printf("t = %llu\n", fs->cb_time);
            switch (fs->action) {
                case GO_LEFT:
                    printf("go left\n");
                    break;
                case GO_RIGHT:
                    printf("go right\n");
                    break;
                case GO_DOWN:
                    printf("go down\n");
                    break;
                case ROTATE_C:
                    printf("rotate clockwise\n");
                    break;
                case ROTATE_CC:
                    printf("rotate counterclockwise\n");
                    break;
                case WAIT:
                    printf("wait\n");
                    break;
            }
            print_board(&fs->game_state);
            printf("\n");
        }*/
    }

    return max_h;
}



// calculate all places we can go and construct a path to the place with
// highest heuristic score
//
// if a is NULL, then only return the best heuristic value, don't construct
// a path
static float _find_best_path(lha_t *a, tetris_state *s, int depth) {
    state_t state;

    // initial time
    state.t0 = s->time;

    heap_init(&state.h);

    state.game_state = s;

    // list of falling spots starts off empty
    state.falling_spots = LIST_END;

    // number of possible states (num possible positions * num unique
    // orientations). There will be some unused space since you can't actually
    // have a piece with bottom left x-coord = width - 1, but we won't worry
    // about that for simplicity
    uint64_t board_size = TETRIS_WIDTH * (TETRIS_HEIGHT + CEIL_BUFFER)
        * N_PIECE_ORIENTATIONS;

    state.m = (state_node *) calloc(board_size, sizeof(state_node));

    for (uint32_t idx = 0; idx < board_size; idx++) {
        // each node starts infinitely far away
        HEAP_NODE_SET(&state.m[idx].node, INFTY);
    }


    // and add its node to the heap
    state_node * fp_node = __find_state_node(&state, s);

    // we will be using lower 8 bits of key to store number of keystrokes
    TETRIS_ASSERT(state.t0 < 0x0080000000000000);
    HEAP_NODE_SET(&fp_node->node, state.t0 << 8);

    // copy the game state into the first node
    tetris_state_shallow_copy(&fp_node->game_state, s);

    // starting node has no parent, so make parent index -1 (invalid)
    fp_node->parent_idx = -1;
    heap_insert(&state.h, &fp_node->node);

    // calculate paths to all locations on the board and find a list of
    // possible landing locaations
    _run_dijkstra(&state);

    heap_destroy(&state.h);

    // choose the best place to land of those landing spots, based
    // on heuristic
    float ret = _choose_best_dst(a, &state, depth);

    if (_at_top_level(a, depth)) {
        // will be freed once the list is no longer in use
        a->__int_state.to_free = state.m;
    }
    else {
        free(state.m);
    }

    return ret;
}


/*
 * try to perform an action, doing so if the next action in the queue is ready,
 * otherwise wait
 *
 * return 1 if the action could be performed, 0 if we are waiting
 */
int _try_move(state_node * action, tetris_state *s) {

    if (action->cb_time <= s->time) {
        // time to perform the action
        int ret = 1;

        switch (action->action) {
            case GO_LEFT:
                tetris_move_piece(s, -1, 0);
                break;
            case GO_RIGHT:
                tetris_move_piece(s, 1, 0);
                break;
            case GO_DOWN:
                tetris_move_piece(s, 0, -1);
                break;
            case ROTATE_C:
                tetris_rotate_piece(s, ROTATE_CLOCKWISE, 1);
                break;
            case ROTATE_CC:
                tetris_rotate_piece(s, ROTATE_COUNTERCLOCKWISE, 1);
                break;
            case WAIT:
                if (piece_equals(action->game_state.falling_piece,
                            s->falling_piece)) {
                    // wait until the falling pieces are different
                    ret = 0;
                }
                break;
            default:
                abort();
        }
        return ret;
    }
    return 0;
}



int linear_heuristic_go(lha_t *a, tetris_state *s) {
    state_node * next_action;

    // do this at most 2 times
    for (int i = 0; i < 2; i++) {
        if (a->__int_state.action_list == NULL) {
            // if no internal state, run dijkstra's algorithm to compute all
            // possible landing spots 

            // remove the falling piece from the board
            board_remove_piece(&s->board, s->falling_piece);

            _find_best_path(a, s, a->depth);

            // place the falling piece back on the board
            board_place_piece(&s->board, s->falling_piece);
        }

        next_action = a->__int_state.action_list;
        if (next_action == NULL) {
            if (a->__int_state.to_free != NULL) {
                free(a->__int_state.to_free);
                a->__int_state.to_free = NULL;
            }
            continue;
        }

        break;
    }

    while (next_action != NULL && _try_move(next_action, s)) {
        // TODO assert piece position

        // remove action from the action list
        a->__int_state.action_list = next_action->next;
        next_action = next_action->next;

        /*if (next_action->action != GO_DOWN &&
                !piece_equals(next_action->falling_piece, s->falling_piece)) {
            print_piece(next_action->falling_piece);
            print_piece(s->falling_piece);
            print_piece(prev_fp);
            abort();
        }*/
    }

    return 1;
}


