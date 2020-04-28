#ifndef _M_VEC2_H
#define _M_VEC2_H

#include <math.h>

typedef union vec2 {
    float __m[2];
    struct {
        float x, y;
    };
} vec2;


static void init_vec2(vec2 *v, float x, float y) {
    v->x = x;
    v->y = y;
}


static float vec2_dot(vec2 *a, vec2 *b) {
    return a->x * b->x + a->y * b->y;
}

static float vec2_mag2(vec2 *a) {
    return vec2_dot(a, a);
}

static float vec2_mag(vec2 *a) {
    return sqrt(vec2_mag2(a));
}

#endif /* _M_VEC2_H */
