
#include <stdio.h>
#include <math.h>
#include <unistd.h>

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
    c.user_data = (void*) &t;

    color_t bg = gen_color(3, 30, 48, 255);
    gl_set_bg_color(bg);

    vec2 pos = {
        .x = -1.f,
        .y = -1.f
    };
    tetris_init(&t, &c, pos, 2.f, 2.f);


    while (!gl_should_exit(&c)) {
        gl_clear(&c);

        tetris_step(&t);

        board_draw(&t.board);

        gl_render(&c);
        glfwPollEvents();
    }

    tetris_destroy(&t);
    gl_exit(&c);

    return 0;
}

