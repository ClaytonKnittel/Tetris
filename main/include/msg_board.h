#ifndef _MSG_BOARD_H
#define _MSG_BOARD_H


#include <gl/font.h>
#include <gl/shader.h>

#include <shape.h>

#define MAX_MSG_LEN 31

// number of ticks to keep the message on screen before removing
#define MSG_LIFETIME 100

// number of ticks to fade the message out (happens so that message disappears
// right when lifetime expires)
#define MSG_FADEOUT_TIME 20

typedef struct msg_board {
    shape base;
    program p;
    font_t *f;

    uint32_t timer;
    char msg[MAX_MSG_LEN + 1];
} msg_board_t;


int msg_board_init(msg_board_t *m, font_t *f, float x, float y,
        float w, float h);

void msg_board_destroy(msg_board_t *m);


void msg_board_post(msg_board_t *m, char *msg);


void msg_board_draw(msg_board_t *m);


#endif /* _MSG_BOARD_H */
