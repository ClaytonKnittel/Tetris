
#include <math.h>
#include <stdlib.h>

#include <util.h>
#include <data_structs/min_heap.h>

#include <tetris.h>
#include <ai.h>
#include <ais/linear_heuristic.h>



lha_t * linear_heuristic_agent_init() {
    lha_t * agent = (lha_t *) calloc(1, sizeof(lha_t));

    return agent;
}


void linear_heuristic_agent_destroy(lha_t *a) {
    free(a);
}


static void print_board(tetris_state * s, piece_t fp) {
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


// goal of AI is to maximize this value
static float heuristic(tetris_state *s) {
    return -((float) s->falling_piece.board_y);
}


// actions (can be a combination of the GO's and ROTATE's)
#define GO_LEFT  0x1
#define GO_RIGHT 0x2
#define GO_DOWN  0x4
#define ROTATE_L 0x8
#define ROTATE_R 0x10

#define INFTY 0x7fffffffffffffffL


typedef struct state_node {
    // number of ticks to get to this position, i.e. dist, is the key in node
    // this value is used to keep track of when it is a major time step
    heap_node node;

    // falling piece at this state. This can theoretically be calculated based
    // on the index of the state in the "m" array, but we instead choose to
    // store it for efficiency and ease of implementation
    piece_t falling_piece;

    // index of state node preceeding this one on shortest path
    uint32_t parent_idx;
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
 * gives a pointer to the state node struct in the state array, given the
 * falling piece's location
 */
static state_node * __find_state_node(state_t *s, piece_t p) {
    int8_t x, y;

    // find bottom left corner of piece, which must be in bounds of the the board
    piece_bottom_left_corner(p, &x, &y);

    uint32_t idx = (y * TETRIS_WIDTH + x) * N_PIECE_ORIENTATIONS + p.orientation;

    return &s->m[idx];
}



static void __try_decrease(state_t * s, piece_t new_fp, uint64_t new_time) {

    state_node * node = __find_state_node(s, new_fp);

    if (node->node.key == INFTY) {
        HEAP_NODE_SET(&node->node, new_time);
        heap_insert(&s->h, &node->node);

        node->falling_piece = new_fp;
    }
    else if (node->node.key > new_time) {
        heap_decrease_key(&s->h, &node->node, new_time);
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
    piece_t fp, new_fp;
    uint64_t time;

    while ((node = __heap_node_to_state_node(heap_extract_min(&s->h))) !=
            NULL) {
        fp = node->falling_piece;
        time = (uint64_t) node->node.key;
        //printf("Extracted %p %d (%f)\n", node, fp.piece_idx, (time - s->t0) / 60.f);

        // find all possible successors of this node

        // next time that move can be made must be on a multiple of
        // AI_INPUT_DELAY
        uint64_t next_input_time = ROUND_UP(time + 1, AI_INPUT_DELAY);

        // translations:
        // press left
        new_fp = fp;
        if (tetris_move_piece_transient(s->game_state, &new_fp, -1, 0)) {
            __try_decrease(s, new_fp, next_input_time);
        }

        // press right
        new_fp = fp;
        if (tetris_move_piece_transient(s->game_state, &new_fp, 1, 0)) {
            __try_decrease(s, new_fp, next_input_time);
        }

        // wait for it to fall
        new_fp = fp;
        tetris_state * game_state = s->game_state;
        uint64_t next_major_ts = time +
            (uint64_t) ceil((game_state->major_tick_count) -
                    game_state->major_tick_time);
        if (tetris_move_piece_transient(s->game_state, &new_fp, 0, -1)) {
            __try_decrease(s, new_fp, next_major_ts);
        }
        else {
            // this piece can stick
            state_node * falling_spot = __find_state_node(s, new_fp);
            __try_falling_spot_append(s, falling_spot);
        }

    }


    for (state_node * fs = s->falling_spots; fs != LIST_END; fs = fs->next) {
        print_piece(fs->falling_piece);
        print_board(s->game_state, fs->falling_piece);
    }
    printf("\n");
}


// calculate all places we can go
static void _find_all_spots(lha_t *a, tetris_state *s) {
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
    uint64_t board_size = TETRIS_WIDTH * TETRIS_HEIGHT * N_PIECE_ORIENTATIONS;

    state.m = (state_node *) calloc(board_size, sizeof(state_node));

    for (uint32_t idx = 0; idx < board_size; idx++) {
        // each node starts infinitely far away
        HEAP_NODE_SET(&state.m[idx].node, INFTY);
    }
    printf("%llu\n", state.m[0].node.key);

    piece_t fp = s->falling_piece;
    // remove the falling piece from the board temporarily
    board_remove_piece(&s->board, fp);

    // and add its node to the heap
    state_node * fp_node = __find_state_node(&state, fp);
    HEAP_NODE_SET(&fp_node->node, state.t0);
    fp_node->falling_piece = fp;
    heap_insert(&state.h, &fp_node->node);

    // calculate paths to all locations on the board and find a list of
    // possible landing locaations
    _run_dijkstra(&state);

    // put the falling piece back
    board_place_piece(&s->board, fp);

    free(state.m);
    heap_destroy(&state.h);
}


int linear_heuristic_go(lha_t *a, tetris_state *s) {

    piece_t fp = s->falling_piece;

    if (__builtin_memcmp(&fp, &a->prev_falling_piece, sizeof(piece_t)) != 0) {
        a->prev_falling_piece = fp;

        _find_all_spots(a, s);
    }

    return 1;
}


