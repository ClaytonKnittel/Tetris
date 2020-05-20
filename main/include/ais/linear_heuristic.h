#ifndef _LINEAR_HEURISTIC_H
#define _LINEAR_HEURISTIC_H

#include <board.h>

typedef struct linear_heuristic_agent {
} lha_t;


lha_t * linear_heuristic_agent_init();


void linear_heuristic_agent_destroy(lha_t *a);


int linear_heuristic_go(lha_t *a, board_t *b);



#endif /* _LINEAR_HEURISTIC_H */
