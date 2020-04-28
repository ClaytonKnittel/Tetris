#ifndef _M_MAT3_H
#define _M_MAT3_H

#include <math.h>

#include <math/vec3.h>

typedef union mat3 {
    float __m[9];
    struct {
        float m00, m01, m02,
              m10, m11, m12,
              m20, m21, m22;
    };
} mat3;


static void init_mat3(mat3 *m, float m00, float m01, float m02,
                               float m10, float m11, float m12,
                               float m20, float m21, float m22) {
    m->m00 = m00;
    m->m01 = m01;
    m->m02 = m02;
    m->m10 = m10;
    m->m11 = m11;
    m->m12 = m12;
    m->m20 = m20;
    m->m21 = m21;
    m->m22 = m22;
}


static void mat3_mul(mat3 *res, mat3 *a, mat3 *b) {
    float am00 = a->m00;
    float am01 = a->m01;
    float am02 = a->m02;
    float am10 = a->m10;
    float am11 = a->m11;
    float am12 = a->m12;
    float am20 = a->m20;
    float am21 = a->m21;
    float am22 = a->m22;

    float bm00 = b->m00;
    float bm01 = b->m01;
    float bm02 = b->m02;
    float bm10 = b->m10;
    float bm11 = b->m11;
    float bm12 = b->m12;
    float bm20 = b->m20;
    float bm21 = b->m21;
    float bm22 = b->m22;

    res->m00 = am00 * bm00 + am01 * bm10 + am02 * bm20;
    res->m01 = am00 * bm01 + am01 * bm11 + am02 * bm21;
    res->m02 = am00 * bm02 + am01 * bm12 + am02 * bm22;

    res->m10 = am10 * bm00 + am11 * bm10 + am12 * bm20;
    res->m11 = am10 * bm01 + am11 * bm11 + am12 * bm21;
    res->m12 = am10 * bm02 + am11 * bm12 + am12 * bm22;

    res->m20 = am20 * bm00 + am21 * bm10 + am22 * bm20;
    res->m21 = am20 * bm01 + am21 * bm11 + am22 * bm21;
    res->m22 = am20 * bm02 + am21 * bm12 + am22 * bm22;
}

static void mat3_vecmul(vec3 *res, mat3 * restrict a, vec3 *b) {
    float bx = b->x;
    float by = b->y;
    float bz = b->z;

    res->x = a->m00 * bx + a->m01 * by + a->m02 * bz;
    res->y = a->m10 * bx + a->m11 * by + a->m12 * bz;
    res->z = a->m20 * bx + a->m21 * by + a->m22 * bz;
}

static float mat3_det(mat3 *m) {
    return m->m00 * (m->m11 * m->m22 - m->m12 * m->m21) +
           m->m01 * (m->m12 * m->m20 - m->m10 * m->m22) +
           m->m02 * (m->m10 * m->m21 - m->m11 * m->m20);
}

#endif /* _M_MAT3_H */
