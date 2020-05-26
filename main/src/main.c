
#include <getopt.h>
#include <stdio.h>
#include <tutil.h>
#include <unistd.h>

#include <math/random.h>

#include <gl/gl.h>
#include <gl/shader.h>

#include <game.h>
#include <ai.h>

#include <syslog.h>
#include <util.h>


#define WIDTH 1024
#define HEIGHT 780


const float aspect_ratio = ((float) HEIGHT) / ((float) WIDTH);

typedef struct board board_t;

void cb(game_t *g, board_t *b, void* arg) {
    game_press(g, GLFW_KEY_LEFT);
    game_press(g, GLFW_KEY_UP);
}


int usage(char *argv[]) {
    fprintf(stderr, "Usage: %s\n", argv[0]);
    return -1;
}


int main(int argc, char *argv[]) {
    gl_context c;
    game_t g;
    font_t font;
    uint32_t level = 0;

    openlog("tetris", LOG_CONS, LOG_USER);

    // by default, seed is system time
    uint64_t seed = time(NULL);
    char *buf;

    // by default don't use an AI
    struct ai *ai = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "p:a:s:l:")) != -1) {
        switch(opt) {
            case 'a':
                // use AI
                ai = fetch_ai(optarg);
                if (ai == NULL) {
                    fprintf(stderr, "%s is not a builtin AI name\n", optarg);
                    return -1;
                }
                break;
            case 's':
                // manually set the seed
                seed = strtoul(optarg, &buf, 10);
                if (*optarg == '\0' || *buf != '\0') {
                    // invalid number
                    fprintf(stderr, "%s is not a valid base 10 unsigned "
                            "number\n", optarg);
                    return usage(argv);
                }
                break;
            case 'l':
                // set the level
                level = strtoul(optarg, &buf, 10);
                if (*optarg == '\0' || *buf != '\0') {
                    // invalid number
                    fprintf(stderr, "%s is not a valid base 10 unsigned "
                            "number\n", optarg);
                    return usage(argv);
                }
            case 'p':
                // for MacOS compatibility (gives sn_0_...)
                break;
            default:
                return usage(argv);
        }
    }

    seed_rand(seed, 0);
    printf("srand: (%llu, 0)\n", seed);

    gl_init(&c, WIDTH, HEIGHT);
    c.user_data = (void*) &g.t;

    font_init(&font, "fonts/8bit_font.ttf", 12lu);

    color_t bg = gen_color(3, 30, 48, 255);
    gl_set_bg_color(bg);

    if (ai != NULL) {
        game_init(&g, SHOW_ALL | MANUAL_CONTROL, &c, &font);
        ai_init(ai, &g);
    }
    else {
        game_init(&g, SHOW_ALL, &c, &font);
    }

    tetris_set_level(&g.t.game_state, level);

    while (!gl_should_exit(&c)) {
        game_tick(&g);

        gl_clear(&c);
        game_render(&g);
        gl_render(&c);
        glfwPollEvents();
    }

    if (ai != NULL) {
        ai_destroy(ai);
    }

    game_destroy(&g);
    font_destroy(&font);
    gl_exit(&c);

    closelog();

    return 0;
}

