#ifndef _RANDOM_H
#define _RANDOM_H

#include <pthread.h>
#include <stdint.h>


// each threads get a unique random seed
extern __thread uint32_t __seed;



static void seed_rand(uint64_t init_seed) {
    __seed = init_seed;
}


static uint32_t __rand_hash(uint32_t seed) {
    uint64_t v = (uint64_t) seed;
    v = (~v) + (v << 18);
    v ^= v >> 31;
    v *= 21;
    v &= v >> 11;
    v += v << 6;
    v ^= v >> 22;
    return (uint32_t) v;
}

/*
 * generate random number (32 bit)
 */
static uint32_t gen_rand() {
    uint32_t res = __rand_hash(__seed);
    __seed = res;
    return res;
}



#endif /* _RANDOM_H */
