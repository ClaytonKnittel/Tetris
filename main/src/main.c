
#include <stdio.h>
#include <math.h>

#include <gl/gl.h>
#include <gl/shader.h>

#include <tetris.h>


#define BOARD_W 10
#define BOARD_H 20

// dimension of single tile in pixels
#define TILE_SIZE 40


int main(int argc, char *argv[]) {
    gl_context c;

    tetris_t t;

    gl_init(&c, BOARD_W * TILE_SIZE, BOARD_H * TILE_SIZE);

    color_t bg = gen_color(3, 30, 48, 255);
    gl_set_bg_color(bg);

    vec2 pos = {
        .x = -.8f,
        .y = -.8f
    };
    tetris_init(&t, pos, 1.6f, 1.6f);

    do {
        usleep(100000);
        gl_clear(&c);

        board_draw(&t.board);

        gl_render(&c);
    } while (!gl_should_exit(&c));

    tetris_destroy(&t);
    gl_exit(&c);

    return 0;
}

