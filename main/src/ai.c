
#include <ai.h>

#include <ais/basic.h>
#include <ais/linear_heuristic.h>


// id's of each of the AI's
#define BASIC_ID 0
#define LINEAR_HEURISTIC_ID 1


struct ai builtin_ais[NUM_AIS] = {
    {
        .name = "basic",
        .callback = &basic_go,
        .id = BASIC_ID
    },
    {
        .name = "lh",
        .callback = (int(*)(void*, tetris_state*)) &linear_heuristic_go,
        .id = LINEAR_HEURISTIC_ID
    }
};


struct ai * fetch_ai(char *name) {
    for (int i = 0; i < NUM_AIS; i++) {
        if (strcmp(builtin_ais[i].name, name) == 0) {
            return &builtin_ais[i];
        }
    }
    return NULL;
}



static void callback(tetris_t *t, void *arg) {
    struct ai * ai = (struct ai *) arg;
    tetris_state * s = &t->game_state;
    ai->callback(ai->ai_struct_ptr, s);
}


int ai_init(struct ai * ai, game_t *g) {

    game_set_ctrl_callback(g, &callback, (void*) ai);

    switch (ai->id) {
        case BASIC_ID:
            // do nothing for this one
            break;
        case LINEAR_HEURISTIC_ID:
            ai->ai_struct_ptr = linear_heuristic_agent_init();
            break;
    }

    return 0;
}

void ai_destroy(struct ai * ai) {
    switch (ai->id) {
        case BASIC_ID:
            // do nothing here, basic has no internal data
            break;
        case LINEAR_HEURISTIC_ID:
            linear_heuristic_agent_destroy((lha_t*) ai->ai_struct_ptr);
            break;
    }
}


