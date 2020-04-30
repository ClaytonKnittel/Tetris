
#include <gl/shader.h>
#include <gl/color.h>

#include <frame.h>
#include <square.h>

const static color_t color_theme[4] = {
    gen_color(180, 180, 180, 255),
    gen_color(240, 240, 240, 255),
    gen_color(150, 150, 150, 255),
    gen_color(100, 100, 100, 255)
};


int frame_init(frame_t *f, uint32_t width, uint32_t height) {
    gl_load_program(&f->p, "main/res/frame.vs", "main/res/frame.fs");

    f->colors_loc = gl_uniform_location(&f->p, "colors");
    f->width_loc = gl_uniform_location(&f->p, "width");
    f->height_loc = gl_uniform_location(&f->p, "height");

    f->width = width;
    f->height = height;

    square_init(&f->tile_prot, .08f, &f->p);
    shape_set_visible(&f->tile_prot);


    // initialize constant uniform variables
    vec4 colors[4];
    for (uint32_t i = 0; i < 4; i++) {
        colors[i] = color_to_vec4(color_theme[i]);
    }
    gl_use_program(&f->p);
    glUniform4fv(f->colors_loc, 4, (float*) colors);

    glUniform1ui(f->width_loc, f->width);
    glUniform1ui(f->height_loc, f->height);

    return 0;
}


void frame_destroy(frame_t *f) {
    shape_destroy(&f->tile_prot);
    gl_unload_program(&f->p);
}


void frame_draw(frame_t *f) {
    gl_use_program(&f->p);

    shape_draw_instanced(&f->tile_prot, 2 * f->height);
}


