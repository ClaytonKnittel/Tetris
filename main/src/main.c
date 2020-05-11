
#include <stdio.h>
#include <unistd.h>

#include <math/random.h>

#include <gl/gl.h>
#include <gl/shader.h>

#include <tetris.h>
#include <frame.h>
#include <hold.h>
#include <score.h>
#include <up_next.h>


#define WIDTH 1024
#define HEIGHT 780


const float aspect_ratio = ((float) HEIGHT) / ((float) WIDTH);


int main(int argc, char *argv[]) {
    gl_context c;
    tetris_t t;
    frame_t f;

    font_t font;
    scoreboard sb;
    up_next_t u;
    hold_t h;

    uint64_t stan = time(NULL);
    seed_rand(stan);
    printf("srand: %llu\n", stan);

    gl_init(&c, WIDTH, HEIGHT);
    c.user_data = (void*) &t;

    font_init(&font, "fonts/8bit_font.ttf", 12lu);

    color_t bg = gen_color(3, 30, 48, 255);
    gl_set_bg_color(bg);


    float w = 2.f * aspect_ratio * ((float) TETRIS_WIDTH) /
        ((float) TETRIS_HEIGHT);
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

    up_next_init(&u, 3, .55f, -.8f, .28f, .9f, &font, &t);

    hold_init(&h, -.8f, .45f, .28f, .5f, &font, &t);

    while (!gl_should_exit(&c)) {
        gl_clear(&c);

        tetris_step(&t);

        board_draw(&t.board);
        frame_draw(&f);
        up_next_draw(&u);
        hold_draw(&h);

        scoreboard_set_score(&sb, t.scorer.score);
        scoreboard_draw(&sb);

        gl_render(&c);
        glfwPollEvents();
    }

    hold_destroy(&h);
    up_next_destroy(&u);
    scoreboard_destroy(&sb);
    font_destroy(&font);
    frame_destroy(&f);
    tetris_destroy(&t);
    gl_exit(&c);

    return 0;
}

