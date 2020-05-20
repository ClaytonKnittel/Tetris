
#include <stdlib.h>

#include <ais/linear_heuristic.h>



lha_t * linear_heuristic_agent_init() {
    lha_t * agent = (lha_t *) malloc(sizeof(lha_t));
    
    return agent;
}


void linear_heuristic_agent_destroy(lha_t *a) {
    free(a);
}


int linear_heuristic_go(lha_t *a, board_t *b) {
    return 1;
}


