
#include <stdlib.h>
#include <string.h>

#include <gl/shader.h>
#include <gl/color.h>

#include <board.h>


// number of different colors of tiles (not including invisible)
#define N_COLORS N_STATES


const static color_t color_theme[4 * N_COLORS] = {
    // color 0 (empty)
    gen_color(0, 0, 0, 0),
    gen_color(0, 0, 0, 0),
    gen_color(0, 0, 0, 0),
    gen_color(0, 0, 0, 0),

    // color 1 (long boy)
    gen_color(0,   240, 240, 255),
    gen_color(179, 251, 251, 255),
    gen_color(0,   216, 216, 255),
    gen_color(0,   120, 120, 255),

    // color 2 (green)
    gen_color(0,   240, 0,   255),
    gen_color(179, 251, 179, 255),
    gen_color(0,   216, 0,   255),
    gen_color(0,   120, 0,   255),

    // color 3 (blue)
    gen_color(0,   0,   240, 255),
    gen_color(179, 179, 251, 255),
    gen_color(0,   0,   216, 255),
    gen_color(0,   0,   120, 255),

    // color 4 (purple)
    gen_color(160, 0,   240, 255),
    gen_color(227, 179, 251, 255),
    gen_color(144, 0,   216, 255),
    gen_color(80,  0,   120, 255),

    // color 5 (orange)
    gen_color(240, 160, 0,   255),
    gen_color(251, 227, 179, 255),
    gen_color(216, 144, 0,   255),
    gen_color(120, 80,  0,   255),

    // color 6 (red)
    gen_color(240, 0,   0,   255),
    gen_color(251, 179, 179, 255),
    gen_color(216, 0,   0,   255),
    gen_color(120, 0,   0,   255),

    // color 7 (yellow)
    gen_color(240, 240, 0,   255),
    gen_color(251, 251, 179, 255),
    gen_color(216, 216, 0,   255),
    gen_color(120, 120, 0,   255),
};


int board_init(board_t *b, uint32_t width, uint32_t height) {
    uint32_t color_idxs_len = color_idxs_arr_len(width * height);
    b->color_idxs = (uint32_t*) calloc(color_idxs_len, sizeof(uint32_t));

    b->width = width;
    b->height = height;
    gl_load_program(&b->p, "main/res/board.vs", "main/res/board.fs");

    b->color_array_loc = gl_uniform_location(&b->p, "color_array");

    b->width_loc = gl_uniform_location(&b->p, "width");
    b->height_loc = gl_uniform_location(&b->p, "height");

    b->color_idxs_loc = gl_uniform_location(&b->p, "color_idxs");


    square_init(&b->tile_prot, .08f, &b->p);

    for (uint32_t i = 0; i < width * height; i++) {
        uint32_t x = i % width;
        uint32_t y = i / width;
        board_set_tile(b, x, y, 0);
    }

    shape_set_visible(&b->tile_prot);


    // initialize constant uniform variables

    vec4 colors[4 * N_COLORS];
    for (uint32_t i = 0; i < 4 * N_COLORS; i++) {
        colors[i] = color_to_vec4(color_theme[i]);
    }

    gl_use_program(&b->p);
    glUniform4fv(b->color_array_loc, 4 * N_COLORS, (float*) colors);

    glUniform1ui(b->width_loc, b->width);
    glUniform1ui(b->height_loc, b->height);

    return 0;
}

void board_destroy(board_t *b) {
    shape_destroy(&b->tile_prot);
    free(b->color_idxs);
    gl_unload_program(&b->p);
}



void board_clear(board_t *b) {
    memset(b->color_idxs, 0, color_idxs_arr_len(b->width * b->height) *
            sizeof(uint32_t));
    b->tiles_changed = 1;
}


/*
 * returns 0 if the tile could not be set, 1 otherwise
 */
int board_set_tile(board_t *b, int32_t x, int32_t y,
        uint32_t tile_color) {
    // by comparing as unsigned, take care of negative case
    if (((uint32_t) x) >= b->width || ((uint32_t) y) >= b->height) {
        return 0;
    }

    b->tiles_changed = 1;

    uint32_t idx = y * b->width + x;
    uint32_t color_idx = idx / COLOR_IDXS_PER_INT;
    uint32_t el_idx = idx - (color_idx * COLOR_IDXS_PER_INT);

    uint32_t mask = COLOR_IDX_MASK << (el_idx * LOG_N_STATES);
    tile_color <<= el_idx * LOG_N_STATES;
    b->color_idxs[color_idx] = (b->color_idxs[color_idx] & ~mask) |
        tile_color;
    return 1;
}

uint8_t board_get_tile(board_t *b, int32_t x, int32_t y) {
    // by comparing as unsigned, take care of negative case
    if (((uint32_t) x) >= b->width || ((uint32_t) y) >= b->height) {
        // if this tile is above the top of the screen, we count it as empty,
        // otherwise, there is an imaginary border just outside the screen that
        // we say the piece is colliding with (return any nonzero value less
        // than 8)
        return (((uint32_t) x) < b->width && y >= 0) ? EMPTY : 1;
    }

    uint32_t idx = y * b->width + x;
    uint32_t color_idx = idx / COLOR_IDXS_PER_INT;
    uint32_t el_idx = idx - (color_idx * COLOR_IDXS_PER_INT);

    uint32_t set = b->color_idxs[color_idx] >> (el_idx * LOG_N_STATES);
    return set & COLOR_IDX_MASK;
}


/*
 * retusn 1 if the given row is full on the board (all nonzero entries), else 0
 */
int board_row_full(board_t *b, int32_t row) {
    for (int32_t col = 0; col < b->width; col++) {
        if (board_get_tile(b, col, row) == EMPTY) {
            return 0;
        }
    }
    return 1;
}


/*
 * copies the entirety of src_row into dst_row
 * TODO optimize
 */
void board_copy_row(board_t *b, int32_t dst_row, int32_t src_row) {
    for (int32_t col = 0; col < b->width; col++) {
        board_set_tile(b, col, dst_row, board_get_tile(b, col, src_row));
    }
}

void board_clear_row(board_t *b, int32_t row) {
    for (int32_t col = 0; col < b->width; col++) {
        board_set_tile(b, col, row, EMPTY);
    }
}




/*
 * places a piece on the board by setting each of the tiles it occupies to its
 * color
 *
 * returns 1 if any piece could be placed on the board, otherwise 0
 */
int board_place_piece(board_t *b, piece_t piece) {
    define_each_piece_tile(p, piece);
    uint32_t tile_color = piece.piece_idx;

    int piece_placed = 0;

    piece_placed |= board_set_tile(b, p_x1, p_y1, tile_color);
    piece_placed |= board_set_tile(b, p_x2, p_y2, tile_color);
    piece_placed |= board_set_tile(b, p_x3, p_y3, tile_color);
    piece_placed |= board_set_tile(b, p_x4, p_y4, tile_color);

    return piece_placed;
}


/*
 * removes a piece on the board by setting each of the tiles it occupies back
 * to EMPTY
 */
void board_remove_piece(board_t *b, piece_t piece) {
    define_each_piece_tile(p, piece);

    board_set_tile(b, p_x1, p_y1, EMPTY);
    board_set_tile(b, p_x2, p_y2, EMPTY);
    board_set_tile(b, p_x3, p_y3, EMPTY);
    board_set_tile(b, p_x4, p_y4, EMPTY);
}



/*
 * checks to see if the given piece will be colliding with any pieces that
 * are already on the given board
 */
int board_piece_collides(board_t *b, piece_t piece) {
    define_each_piece_tile(p, piece);

    uint8_t p1 = board_get_tile(b, p_x1, p_y1);
    uint8_t p2 = board_get_tile(b, p_x2, p_y2);
    uint8_t p3 = board_get_tile(b, p_x3, p_y3);
    uint8_t p4 = board_get_tile(b, p_x4, p_y4);

    // if all squares are empty, then all four tiles or-ed together will be
    // 0, otherwise, if any tile isn't empty, we will get a nonzero result
    return ((p1 | p2) | (p3 | p4)) != 0;
}





void board_draw(board_t *b) {
    gl_use_program(&b->p);

    if (b->tiles_changed) {
        // send over all color information about tiles
        glUniform1uiv(b->color_idxs_loc, b->width * b->height, b->color_idxs);

        b->tiles_changed = 0;
    }

    shape_draw_instanced(&b->tile_prot, b->width * b->height);
}


