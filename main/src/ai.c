
#include <ai.h>

#include <ais/basic.h>


// id's of each of the AI's
#define BASIC_ID 0


struct ai builtin_ais[NUM_AIS] = {
    {
        .name = "basic",
        .callback = &basic_go,
        .id = BASIC_ID
    }
};



static void callback(game_t *g, board_t *b, void *arg) {
    struct ai * ai = (struct ai *) arg;
    ai->callback(ai, b);
}


int ai_init(struct ai * ai, game_t *g) {

    game_set_ctrl_callback(g, &callback, (void*) ai);

    switch (ai->id) {
        case BASIC_ID:
            // do nothing for this one
            break;
    }

    return 0;
}

void ai_destroy(struct ai * ai) {
    switch (ai->id) {
        case BASIC_ID:
            // do nothing here, basic has no internal data
            break;
    }
}


