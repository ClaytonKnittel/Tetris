
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



lha_t * linear_heuristic_agent_init() {
    lha_t * agent = (lha_t *) calloc(1, sizeof(lha_t));

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
    union {
        // number of ticks to get to this position, i.e. dist, is the key in node
        // this value is used to keep track of when it is a major time step
        heap_node node;

        struct {
            uint64_t __unused_[2];
            // time at which this action is to be performed, aliases key in node
            uint64_t time;
        };
    };

    // snapshot of the game state at this particular state node
    tetris_state game_state;

    // index of state node preceeding this one on shortest path
    int32_t parent_idx;
    // action taken to get here from parent
    int action;

    // singly linked list of possible falling spots
    struct state_node * next;
} state_node;


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



static void __try_decrease(state_t * s, tetris_state * new_state,
        uint32_t parent_idx, int action) {

    uint64_t new_time = new_state->time;

    state_node * node = __find_state_node(s, new_state);

    if (node->node.key == INFTY) {
        HEAP_NODE_SET(&node->node, new_time);
        heap_insert(&s->h, &node->node);

        tetris_state_shallow_copy(&node->game_state, new_state);
        node->parent_idx = parent_idx;
        node->action = action;
    }
    else if (node->node.key > new_time) {
        heap_decrease_key(&s->h, &node->node, new_time);

        tetris_state_shallow_copy(&node->game_state, new_state);
        node->parent_idx = parent_idx;
        node->action = action;
    }
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
    uint64_t time;

    while ((node = __heap_node_to_state_node(heap_extract_min(&s->h))) !=
            NULL) {
        game_state = &node->game_state;
        //print_board(game_state);

        time = (uint64_t) node->node.key;
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
            __try_decrease(s, &new_state, parent_idx, WAIT);
        }

        // KEYBOARD INPUTS:

        // translations:
        // press left
        tetris_state_shallow_copy(&new_state, game_state);
        if (tetris_move_piece_transient(&new_state, -1, 0)) {
            // advance game by input delay ticks
            uint64_t adv = AI_INPUT_DELAY;
            if (tetris_advance_by_transient(&new_state, &adv) != 0) {
                // could not advance because either the game ended or 
            }
            __try_decrease(s, &new_state, parent_idx, GO_LEFT);
        }

        // press right
        tetris_state_shallow_copy(&new_state, game_state);
        if (tetris_move_piece_transient(&new_state, 1, 0)) {
            // advance game by input delay ticks
            uint64_t adv = AI_INPUT_DELAY;
            if (tetris_advance_by_transient(&new_state, &adv) != 0) {
                // could not advance because either the game ended or 
            }
            __try_decrease(s, &new_state, parent_idx, GO_RIGHT);
        }

        // press down
        tetris_state_shallow_copy(&new_state, game_state);
        if (tetris_move_piece_transient(&new_state, 0, -1)) {

            piece_t cur_piece = new_state.falling_piece;

            // advance game by input delay ticks
            uint64_t adv = AI_INPUT_DELAY;
            if (tetris_advance_by_transient(&new_state, &adv) != 0) {
                // could not advance because either the game ended or 
            }
            else if (piece_equals(cur_piece, new_state.falling_piece)) {
                // only allow pressing down if gravity will not be moving the
                // falling piece down
                __try_decrease(s, &new_state, parent_idx, GO_DOWN);
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
                __try_decrease(s, &new_state, parent_idx, ROTATE_C);
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
                __try_decrease(s, &new_state, parent_idx, ROTATE_CC);
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

    while (1) {
        node->next = prev;
        prev = node;

        // action is now the action needed to be taken to get from this node
        // to the next node, not the action to get here from the previous
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


/*
 * go through list of possible landing spots and choose which one makes
 * heuristic highest
 */
static void _choose_best_dst(lha_t *a, state_t *s) {

    state_node * best = NULL;
    float max_h = -INFINITY;

    for (state_node * fs = s->falling_spots; fs != LIST_END; fs = fs->next) {
        //printf("%llu\n", fs->game_state.time);
        //print_board(&fs->game_state);

        float h = heuristic(&fs->game_state);

        if (h > max_h) {
            max_h = h;
            best = fs;
        }
    }

    if (best == NULL) {
        return;
    }

    state_node * path = _construct_path_to(s, best);

    a->__int_state.action_list = path;

    /*printf("Path:\n");
    for (state_node * fs = path; fs != NULL; fs = fs->next) {
        printf("t = %llu\n", fs->time);
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


// calculate all places we can go and construct a path to the place with
// highest heuristic score
static void _find_best_path(lha_t *a, tetris_state *s) {
    state_t state;

    // calculate max number of moves that can be input between any major time
    // steps to determine how many actions can be performed, at most, between
    // major time steps (i.e. the falling piece going down by one tile
    //uint32_t moves_per_mjts = s->major_tick_count / AI_INPUT_DELAY;

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
    HEAP_NODE_SET(&fp_node->node, state.t0);

    // copy the game state into the first node
    tetris_state_shallow_copy(&fp_node->game_state, s);
    // and remove the falling piece from the board
    board_remove_piece(&s->board, s->falling_piece);

    // starting node has no parent, so make parent index -1 (invalid)
    fp_node->parent_idx = -1;
    heap_insert(&state.h, &fp_node->node);

    // calculate paths to all locations on the board and find a list of
    // possible landing locaations
    _run_dijkstra(&state);

    // will be freed once the list is no longer in use
    a->__int_state.to_free = state.m;
    heap_destroy(&state.h);

    // and finally choose the best place to land of those landing spots, based
    // on heuristic
    _choose_best_dst(a, &state);

    // place the falling piece back on the board
    board_place_piece(&s->board, s->falling_piece);
}


/*
 * try to perform an action, doing so if the next action in the queue is ready,
 * otherwise wait
 *
 * return 1 if the action could be performed, 0 if we are waiting
 */
int _try_move(state_node * action, tetris_state *s) {

    if (action->time <= s->time) {
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
            _find_best_path(a, s);
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

    if (next_action != NULL && _try_move(next_action, s)) {
        // TODO assert piece position

        // remove action from the action list
        a->__int_state.action_list = next_action->next;

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


