#ifndef _M_VEC3_H
#define _M_VEC3_H

#include <math.h>

typedef union vec3 {
    float __m[3];
    struct {
        float x, y, z;
    };
} vec3;


static void init_vec3(vec3 *v, float x, float y, float z) {
    v->x = x;
    v->y = y;
    v->z = z;
}


static float vec3_dot(vec3 *a, vec3 *b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

static float vec3_mag2(vec3 *a) {
    return vec3_dot(a, a);
}

static float vec3_mag(vec3 *a) {
    return sqrt(vec3_mag2(a));
}

#endif /* _M_VEC3_H */
