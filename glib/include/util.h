
#include <stdint.h>


// reinterpret cast of int to float (keep bits same)
static float int_to_float(uint32_t i) {
    union {
        uint32_t ival;
        float fval;
    } __v = {
        .ival = i
    };
    return __v.fval;
}

