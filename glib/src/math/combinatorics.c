
#include <math/combinatorics.h>
#include <math/random.h>



#define DEFINE_PERMUTE_N(n) \
void permute_ ## n(uint ## n ## _t *list, size_t size) { \
    uint64_t i, swap_idx;                                \
    uint ## n ## _t tmp;                                 \
    for (i = 0; i < size - 1; i++) {                     \
        swap_idx = gen_rand_r(size - i);                 \
        tmp = list[i];                                   \
        list[i] = list[swap_idx];                        \
        list[swap_idx] = tmp;                            \
    }                                                    \
}



DEFINE_PERMUTE_N(8)
DEFINE_PERMUTE_N(16)
DEFINE_PERMUTE_N(32)
DEFINE_PERMUTE_N(64)

