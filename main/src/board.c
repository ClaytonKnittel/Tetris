
#include <stdlib.h>

#include <gl/shader.h>

#include <board.h>


// number of different colors of tiles (not including invisible)
#define N_COLORS 2



int board_init(board_t *b, uint32_t width, uint32_t height) {
    b->tiles = (shape*) malloc(width * height * sizeof(shape));
    b->color_idxs = (uint32_t*) malloc(width * height * sizeof(uint32_t));

    b->width = width;
    b->height = height;
    gl_load_program(&b->p, "main/res/board.vs", "main/res/board.frag");
    b->color_array_loc = gl_uniform_location(&b->p, "color_array");
    b->color_idx_loc = gl_uniform_location(&b->p, "color_idx");

    for (uint32_t i = 0; i < width * height; i++) {
        square_init(&b->tiles[i], .05f, &b->p);

        uint32_t x = i % width;
        uint32_t y = i / width;
        shape_set_pos(&b->tiles[i], (2.f * x) / width - 1.f, (2.f * y) / height - 1.f);
        shape_set_xscale(&b->tiles[i], 2.f / width);
        shape_set_yscale(&b->tiles[i], 2.f / height);

        b->color_idxs[i] = (((x ^ y) & 1) == 0 ? 1 : 0);

        shape_set_visible(&b->tiles[i]);
    }
    return 0;
}

void board_destroy(board_t *b) {
    for (uint32_t i = 0; i < b->width * b->height; i++) {
        shape_destroy(&b->tiles[i]);
    }
    gl_unload_program(&b->p);
}


void board_draw(board_t *b) {
    const vec4 colors[3 * N_COLORS] = {
        color_to_vec4(gen_color(200, 200, 200, 255)),
        color_to_vec4(gen_color(240, 240, 240, 255)),
        color_to_vec4(gen_color(150, 150, 150, 255)),

        color_to_vec4(gen_color(20, 160, 180, 255)),
        color_to_vec4(gen_color(24, 210, 240, 255)),
        color_to_vec4(gen_color(15, 110, 130, 255)),
    };

    glUniform4fv(b->color_array_loc, 3 * N_COLORS, (float*) colors);

    gl_use_program(&b->p);
    for (uint32_t i = 0; i < b->width * b->height; i++) {
        glUniform1ui(b->color_idx_loc, b->color_idxs[i]);
        shape_draw(&b->tiles[i]);
    }
}


