#ifndef _BOARD_H
#define _BOARD_H

#include <square.h>
#include <piece.h>

// number of possible states for each tile
#define N_STATES 8
#define LOG_N_STATES 3


// number of color indices that can be packed into a single uint32_t
#define COLOR_IDXS_PER_INT \
    ((sizeof(uint32_t) * 8) / LOG_N_STATES)

// mask for each individual color index
#define COLOR_IDX_MASK (N_STATES - 1)

// length of color_idxs 3-bitvector
#define color_idxs_arr_len(n_tiles) \
    (((n_tiles) + COLOR_IDXS_PER_INT - 1) / COLOR_IDXS_PER_INT)


#define BOARD_CHANGED 0x1
#define BOARD_GRAYED 0x2
#define BOARD_COPY 0x4


typedef struct board {
    // tile prototype shape (will be instanced)
    shape tile_prot;

    // packed color indices for each tile
    uint32_t *color_idxs;

    // width and height of board, in tiles
    uint32_t width, height;

    /*
     * status flags for board, possible values are:
     *  BOARD_CHANGED: set whenever any of the tiles on the board have
     *      been changed
     *  BOARD_GRAYED: set if the board is to be displayed grayed out
     *  BOARD_COPY: set if this is a copy of the original board
     */
    uint32_t flags;

    program p;
    // see board.vs for descriptions of these
    GLuint color_array_loc, color_idxs_loc;
    GLuint width_loc, height_loc;
    GLuint grayed_loc;
} board_t;


int board_init(board_t *b, uint32_t width, uint32_t height);

void board_destroy(board_t *b);


/*
 * makes a deep copy of the baord. The original board MUST be destroyed after
 * all of its copies are destroyed, if a copy is used after the original was
 * destroyed, the behavior is undefined
 */
void board_deep_copy(board_t *dst, const board_t *src);


static void board_set_pos(board_t *b, float x, float y) {
    shape_set_pos(&b->tile_prot, x, y);
}

static void board_set_xscale(board_t *b, float xscale) {
    shape_set_xscale(&b->tile_prot, xscale);
}

static void board_set_yscale(board_t *b, float yscale) {
    shape_set_yscale(&b->tile_prot, yscale);
}


/*
 * the two following functions set and unset the board to be rendered in
 * grayscale (respectively)
 */
void board_set_grayed(board_t *b);

void board_unset_grayed(board_t *b);



void board_clear(board_t *b);


/*
 * returns 0 if the tile could not be set, 1 otherwise
 */
int board_set_tile(board_t *b, int32_t x, int32_t y,
        uint32_t tile_color);


uint8_t board_get_tile(board_t *b, int32_t x, int32_t y);


/*
 * retusn 1 if the given row is full on the board (all nonzero entries), else 0
 */
int board_row_full(board_t *b, int32_t row);



/*
 * copies the entirety of src_row into dst_row
 * TODO optimize
 */
void board_copy_row(board_t *b, int32_t dst_row, int32_t src_row);


void board_clear_row(board_t *b, int32_t row);




/*
 * places a piece on the board by setting each of the tiles it occupies to its
 * color
 *
 * returns 1 if any piece could be placed on the board, otherwise 0
 */
int board_place_piece(board_t *b, piece_t piece);

/*
 * returns 1 if the piece can be placed on the board (i.e. at least one tile of
 * the piece would be on the board), 0 otherwise
 */
int board_can_place_piece(board_t *b, piece_t piece);


/*
 * removes a piece on the board by setting each of the tiles it occupies back
 * to EMPTY
 */
void board_remove_piece(board_t *b, piece_t piece);


/*
 * checks to see if the given piece will be colliding with any pieces that
 * are already on the given board
 */
int board_piece_collides(board_t *b, piece_t piece);


void board_draw(board_t *b);


#endif /* _BOARD_H */
