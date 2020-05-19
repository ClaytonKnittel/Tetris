
#include <stdio.h>
#include <unistd.h>

#include <math/random.h>

#include <gl/gl.h>
#include <gl/shader.h>

#include <game.h>


#define WIDTH 1024
#define HEIGHT 780


const float aspect_ratio = ((float) HEIGHT) / ((float) WIDTH);


int main(int argc, char *argv[]) {
    gl_context c;
    game_t g;

    font_t font;

    uint64_t stan = time(NULL);
    seed_rand(stan, 0);
    printf("srand: (%llu, 0)\n", stan);

    gl_init(&c, WIDTH, HEIGHT);
    c.user_data = (void*) &g;

    font_init(&font, "fonts/8bit_font.ttf", 12lu);

    color_t bg = gen_color(3, 30, 48, 255);
    gl_set_bg_color(bg);

    game_init(&g, SHOW_ALL, &c, &font);

    while (!gl_should_exit(&c)) {
        game_tick(&g);

        gl_clear(&c);
        game_render(&g);
        gl_render(&c);
        glfwPollEvents();
    }

    game_destroy(&g);
    font_destroy(&font);
    gl_exit(&c);

    return 0;
}

