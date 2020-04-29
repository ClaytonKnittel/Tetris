#ifndef _COMBINATORICS_H
#define _COMBINATORICS_H

#include <assert.h>
#include <stdio.h>

#include <math/random.h>


void permute_8(uint8_t *list, size_t size);
void permute_16(uint16_t *list, size_t size);
void permute_32(uint32_t *list, size_t size);
void permute_64(uint64_t *list, size_t size);


/*
 * randomly permutes a list of length "size" whose elements are of size
 * "memb_size" bytes
 */
static void permute(void *list, size_t size, size_t memb_size) {
    switch (memb_size) {
        case 1:
            permute_8((uint8_t*) list, size);
            break;
        case 2:
            permute_16((uint16_t*) list, size);
            break;
        case 4:
            permute_32((uint32_t*) list, size);
            break;
        case 8:
            permute_64((uint64_t*) list, size);
            break;
        default:
            fprintf(stderr, "No such member size %lu\n", memb_size);
            assert(0);
    }
}


#endif /* _COMBINATORICS_H */
