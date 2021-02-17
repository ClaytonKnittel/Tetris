#ifndef _LINEAR_HEURISTIC_H
#define _LINEAR_HEURISTIC_H

#include <board.h>
#include <piece.h>


#define N_CNSTS 4


struct lha_state {
    // pointer to data region which is to be freed when exiting an lha_state
    void * to_free;

    // list of actions to be taken, along with the times to take those actions
    struct state_node * action_list;
};


typedef struct linear_heuristic_agent {
    // number of turns in the future to search to (must be at least 1)
    int depth;

    // consider only best n possible landing locations according to heuristic,
    // lowers branching factor
    int best_n;

    float cnsts[N_CNSTS];

    // internal state of AI, which is updated whenever a new falling piece is
    // grabbed or the path of a falling piece is interrupted/corrupted to where
    // the next action that should be performed is no longer clear
    struct lha_state __int_state;
} lha_t;


lha_t * linear_heuristic_agent_init();

/*
 * initialize agent with provided heuristic parameters
 */
lha_t * linear_heuristic_agent_init_cnsts(const float * cnsts);


void linear_heuristic_agent_destroy(lha_t *a);


int linear_heuristic_go(lha_t *a, tetris_state *s);



#endif /* _LINEAR_HEURISTIC_H */
