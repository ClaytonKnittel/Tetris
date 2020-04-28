#ifndef _SHAPE_H
#define _SHAPE_H

#include <gl/drawable.h>

#include <math/vec2.h>
#include <math/mat3.h>

typedef struct shape {
    drawable base;

    GLuint mat_loc;
    float xscale, yscale;
    vec2 pos;

    int visible;

    // to be set when a drawable is updated, unset when the updates have been
    // synchronized with uniform variables in the shader (to minimize GPU
    // comminucation traffic)
    int modified;
} shape;


/*
 * all shape's shaders must have a uniform mat3 trans variable, which is
 * multiplied by the vertex locations to generate screen coordinates. The
 * handling of coloring and all else can be done by implementations of shape
 */
static void shape_init(shape *s, program * p) {

    // responsibility of calling proper gl_load_*_drawable is on "subclass"

    s->mat_loc = gl_uniform_location(p, "trans");

    s->visible = 0;
    s->modified = 1;
}

static void shape_destroy(shape *s) {
    gl_unload_static_monochrome_drawable(&s->base);
}


static void shape_set_pos(shape *s, float x, float y) {
    init_vec2(&s->pos, x, y);
    s->modified = 1;
}

static void shape_set_xscale(shape *s, float xscale) {
    s->xscale = xscale;
    s->modified = 1;
}

static void shape_set_yscale(shape *s, float yscale) {
    s->yscale = yscale;
    s->modified = 1;
}

static void shape_set_visible(shape *s) {
    s->visible = 1;
}

static void shape_set_invisible(shape *s) {
    s->visible = 0;
}


static void shape_draw(shape *s) {
    if (!s->visible) {
        return;
    }

    if (s->modified) {
        mat3 trans = {
            .m00 = s->xscale,
            .m01 = 0.f,
            .m02 = s->pos.x,
            .m10 = 0.f,
            .m11 = s->yscale,
            .m12 = s->pos.y,
            .m20 = 0.f,
            .m21 = 0.f,
            .m22 = 1.f
        };

        glUniformMatrix3fv(s->mat_loc, 1, GL_TRUE, (GLfloat*) &trans.__m);

        s->modified = 0;
    }

    gl_draw(&s->base);
}

static void shape_draw_instanced(shape *s, size_t primcount) {
    if (!s->visible) {
        return;
    }

    if (s->modified) {
        mat3 trans = {
            .m00 = s->xscale,
            .m01 = 0.f,
            .m02 = s->pos.x,
            .m10 = 0.f,
            .m11 = s->yscale,
            .m12 = s->pos.y,
            .m20 = 0.f,
            .m21 = 0.f,
            .m22 = 1.f
        };

        glUniformMatrix3fv(s->mat_loc, 1, GL_TRUE, (GLfloat*) &trans.__m);

        s->modified = 0;
    }

    gl_draw_instanced(&s->base, primcount);
}


#endif /* _SHAPE_H */
