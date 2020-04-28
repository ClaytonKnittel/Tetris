
#include <util.h>

#include <square.h>

#define N_ELS 90

#define FILL 0
#define LRIM 1
#define MRIM 2
#define DRIM 3


void square_init(shape *s, float rim_w, program * p) {

    // reinterpret casts just so c will allow us to compile
    float fill = int_to_float(FILL);
    float lrim = int_to_float(LRIM);
    float mrim = int_to_float(MRIM);
    float drim = int_to_float(DRIM);

    float data[N_ELS] = {
        // fill
        rim_w,        rim_w,        fill,
        1.f - rim_w,  rim_w,        fill,
        1.f - rim_w,  1.f - rim_w,  fill,

        1.f - rim_w,  1.f - rim_w,  fill,
        rim_w,        1.f - rim_w,  fill,
        rim_w,        rim_w,        fill,

        // bottom rim 
        0.f,          0.f,          drim,
        1.f,          0.f,          drim,
        1.f - rim_w,  rim_w,        drim,

        1.f - rim_w,  rim_w,        drim,
        rim_w,        rim_w,        drim,
        0.f,          0.f,          drim,

        // right rim 
        1.f - rim_w,  rim_w,        mrim,
        1.f,          0.f,          mrim,
        1.f,          1.f,          mrim,

        1.f,          1.f,          mrim,
        1.f - rim_w,  1.f - rim_w,  mrim,
        1.f - rim_w,  rim_w,        mrim,

        // top rim 
        rim_w,        1.f - rim_w,  lrim,
        1.f - rim_w,  1.f - rim_w,  lrim,
        1.f,          1.f,          lrim,

        1.f,          1.f,          lrim,
        0.f,          1.f,          lrim,
        rim_w,        1.f - rim_w,  lrim,

        // left rim 
        0.f,          0.f,          mrim,
        rim_w,        rim_w,        mrim,
        rim_w,        1.f - rim_w,  mrim,

        rim_w,        1.f - rim_w,  mrim,
        0.f,          1.f,          mrim,
        0.f,          0.f,          mrim,
    };

    shape_init(s, p);
    gl_load_static_indexed_drawable(&s->base, (uint32_t*) data, N_ELS / 3);
}

