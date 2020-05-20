#ifndef _AI_H
#define _AI_H

#include <string.h>

#include <board.h>
#include <game.h>


#define NUM_AIS 2


struct ai {
    char *name;
    int (*callback)(void* ai, board_t*);
    // pointer to ai-specific struct
    void *ai_struct_ptr;
    // id starting from 0 and going up for each different AI
    int id;
};



extern struct ai builtin_ais[NUM_AIS];


struct ai * fetch_ai(char *name);

int ai_init(struct ai * ai, game_t *g);

void ai_destroy(struct ai * ai);



#endif /* _AI_H */
