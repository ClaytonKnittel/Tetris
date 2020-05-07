
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#include <gl/gl.h>
#include <gl/shader.h>

#include <tetris.h>
#include <frame.h>
#include <score.h>


#define WIDTH 1024
#define HEIGHT 780


int main(int argc, char *argv[]) {
    gl_context c;
    tetris_t t;
    frame_t f;

    font_t font;
    scoreboard sb;


    gl_init(&c, WIDTH, HEIGHT);
    c.user_data = (void*) &t;

    font_init(&font, "fonts/8bit_font.ttf", 12lu);

    color_t bg = gen_color(3, 30, 48, 255);
    gl_set_bg_color(bg);


    float w = 2.f * ((float) HEIGHT) / ((float) WIDTH) *
        ((float) TETRIS_WIDTH) / ((float) TETRIS_HEIGHT);
    vec2 pos = {
        .x = -w / 2.f,
        .y = -1.f
    };
    tetris_init(&t, &c, pos, w, 2.f);

    frame_init(&f, TETRIS_WIDTH, TETRIS_HEIGHT);
    frame_set_pos(&f, -w / 2.f, -1.f);
    frame_set_xscale(&f, w);
    frame_set_yscale(&f, 2.f);

    scoreboard_init(&sb, &font, .5f, .45f, .5f, .15f);

    while (!gl_should_exit(&c)) {
        gl_clear(&c);

        tetris_step(&t);

        board_draw(&t.board);
        frame_draw(&f);

        scoreboard_set_score(&sb, t.scorer.score);
        scoreboard_draw(&sb);

        gl_render(&c);
        glfwPollEvents();
    }

    scoreboard_destroy(&sb);
    font_destroy(&font);
    frame_destroy(&f);
    tetris_destroy(&t);
    gl_exit(&c);

    return 0;
}

