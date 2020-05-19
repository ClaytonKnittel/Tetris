
#include <stdio.h>

#include <math/random.h>



int main(int argc, char *argv[]) {

    seed_rand(0, 0);

#define N 16
    int cnts[N];

    __builtin_memset(cnts, 0, N * sizeof(int));

    for (int i = 0; i < 100000000; i++) {
        uint32_t r = gen_rand_r(N);
        cnts[r]++;
    }
    
    for (int i = 0; i < N; i++) {
        printf("[%2d] = %d\n", i, cnts[i]);
    }

    return 0;
}

