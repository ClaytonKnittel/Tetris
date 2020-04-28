
#include <stdio.h>
#include <math.h>

#include <gl/gl.h>
#include <gl/shader.h>

#include <board.h>


#define BOARD_W 10
#define BOARD_H 20

#define TILE_SIZE 40


int main(int argc, char *argv[]) {
    gl_context c;

    board_t b;

    gl_init(&c, BOARD_W * TILE_SIZE, BOARD_H * TILE_SIZE);

    color_t bg = gen_color(3, 30, 48, 255);
    gl_set_bg_color(bg);

    board_init(&b, BOARD_W, BOARD_H);

    do {
        gl_clear(&c);

        board_draw(&b);

        gl_render(&c);
    } while (!gl_should_exit(&c));

    board_destroy(&b);

    return 0;
}

