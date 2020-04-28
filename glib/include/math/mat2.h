#ifndef _M_MAT2_H
#define _M_MAT2_H

#include <math.h>

typedef union mat2 {
    float __m[4];
    struct {
        float m00, m01,
              m10, m11;
    };
} mat2;


static void init_mat2(mat2 *m, float m00, float m01,
                               float m10, float m11) {
    m->m00 = m00;
    m->m01 = m01;
    m->m10 = m10;
    m->m11 = m11;
}


static void mat2_mul(mat2 *res, mat2 *a, mat2 *b) {
    float am00 = a->m00;
    float am01 = a->m01;
    float am10 = a->m10;
    float am11 = a->m11;
    float bm00 = b->m00;
    float bm01 = b->m01;
    float bm10 = b->m10;
    float bm11 = b->m11;

    res->m00 = am00 * bm00 + am01 * bm10;
    res->m01 = am00 * bm01 + am01 * bm11;
    res->m10 = am10 * bm00 + am11 * bm10;
    res->m11 = am10 * bm01 + am11 * bm11;
}

static void mat2_vecmul(vec2 *res, mat2 * restrict a, vec2 *b) {
    float bx = b->x;
    float by = b->y;

    res->x = a->m00 * bx + a->m01 * by;
    res->y = a->m10 * bx + a->m11 * by;
}

static float mat2_det(mat2 *m) {
    return m->m00 * m->m11 - m->m01 * m->m10;
}

#endif /* _M_MAT2_H */
