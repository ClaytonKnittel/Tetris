
#include <stdio.h>

#include <gl.h>
#include <drawable.h>
#include <shader.h>

int main(int argc, char *argv[]) {
    gl_context c;
    program p;

    drawable d;

    union {
        int ival;
        float fval;
    } color = {
        .ival = 0xff0000ff
    };

    float data[] = {
        -1.f, -1.f, color.fval,
        1.f, -1.f, color.fval,
        0.f, 1.f, color.fval,
    };

    gl_init(&c);
    load_program(&p, "main/shaders/two.vs", "main/shaders/two.frag");

    color_t bg = gen_color(10, 150, 140, 255);
    gl_set_bg_color(bg);

    gl_load_static_monochrome_drawable(&d, (uint32_t*) data, 3);

    do {
        gl_clear(&c);
        use_program(&p);

        gl_draw(&d);

        gl_render(&c);
    } while (!gl_should_exit(&c));

    gl_unload_static_monochrome_drawable(&d);
    unload_program(&p);

    return 0;
}

