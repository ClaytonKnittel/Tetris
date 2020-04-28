
#include <stdlib.h>

#include <gl/shader.h>
#include <gl/color.h>

#include <board.h>


// number of different colors of tiles (not including invisible)
#define N_COLORS 8


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
    b->color_idxs = (uint32_t*) malloc(width * height * sizeof(uint32_t));

    b->width = width;
    b->height = height;
    gl_load_program(&b->p, "main/res/board.vs", "main/res/board.fs");

    b->color_array_loc = gl_uniform_location(&b->p, "color_array");

    b->width_loc = gl_uniform_location(&b->p, "width");
    b->height_loc = gl_uniform_location(&b->p, "height");

    b->color_idx_loc = gl_uniform_location(&b->p, "color_idx");

    b->tile_idx_loc = gl_uniform_location(&b->p, "tile_idx");


    square_init(&b->tile_prot, .08f, &b->p);

    shape_set_pos(&b->tile_prot, 0.f, 0.f);
    shape_set_xscale(&b->tile_prot, 1.f);
    shape_set_yscale(&b->tile_prot, 1.f);

    for (uint32_t i = 0; i < width * height; i++) {
        uint32_t x = i % width;
        uint32_t y = i / width;
        b->color_idxs[i] = ((x ^ y) % 8);
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


void board_draw(board_t *b) {
    gl_use_program(&b->p);

    for (uint32_t i = 0; i < b->width * b->height; i++) {
        glUniform1ui(b->color_idx_loc, b->color_idxs[i]);
        glUniform1ui(b->tile_idx_loc, i);
        shape_draw(&b->tile_prot);
    }
}


