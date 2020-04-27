#ifndef _COLOR_H
#define _COLOR_H


typedef uint32_t color_t;


static color_t gen_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (r) | (g << 8) | (b << 16) | (a << 24);
}


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


#endif /* _COLOR_H */
