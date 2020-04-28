#ifndef _COLOR_H
#define _COLOR_H

#include <math/vec4.h>


typedef uint32_t color_t;


#define gen_color(r, g, b, a) \
    ((color_t) ((r) | (g << 8) | (b << 16) | (a << 24)))


static float color_r(color_t color) {
    return (color & 0xff) / 255.f;
}

static float color_g(color_t color) {
    return ((color >> 8) & 0xff) / 255.f;
}

static float color_b(color_t color) {
    return ((color >> 16) & 0xff) / 255.f;
}

static float color_a(color_t color) {
    return ((color >> 24) & 0xff) / 255.f;
}

static float color_fval(color_t color) {
    union {
        uint32_t ival;
        float fval;
    } c = {
        .ival = color
    };
    return c.fval;
}

static vec4 color_to_vec4(color_t color) {
    vec4 ret;
    init_vec4(&ret, color_r(color),
                    color_g(color),
                    color_b(color),
                    color_a(color));
    return ret;
}


#endif /* _COLOR_H */
