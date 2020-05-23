#ifndef _LINEAR_HEURISTIC_H
#define _LINEAR_HEURISTIC_H

#include <board.h>
#include <piece.h>

typedef struct linear_heuristic_agent {
    piece_t prev_falling_piece;
} lha_t;


lha_t * linear_heuristic_agent_init();


void linear_heuristic_agent_destroy(lha_t *a);


int linear_heuristic_go(lha_t *a, tetris_state *s);



#endif /* _LINEAR_HEURISTIC_H */
