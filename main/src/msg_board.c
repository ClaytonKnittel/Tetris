
#include <msg_board.h>



int msg_board_init(msg_board_t *m, font_t *f, float x, float y,
        float w, float h) {

    m->f = f;

    shape_set_pos(&m->base, x, y);
    shape_set_xscale(&m->base, w);
    shape_set_yscale(&m->base, h);

    m->msg[0] = '\0';
    // null-terminate the end, as we will never write to it
    m->msg[MAX_MSG_LEN] = '\0';
    m->timer = 0;

    m->msg[0] = 'c';
    m->msg[1] = 'l';
    m->msg[2] = 'a';
    m->msg[3] = 'y';
    m->msg[4] = '\0';
    m->timer = MSG_LIFETIME;

    return 0;

}

void msg_board_destroy(msg_board_t *m) {
    return;
}


void msg_board_post(msg_board_t *m, char *msg) {
    strncpy(m->msg, msg, MAX_MSG_LEN);
    // reset timer
    m->timer = MSG_LIFETIME;
}


void msg_board_draw(msg_board_t *m) {
    uint8_t alpha;

    uint32_t timer = m->timer;

    if (timer != 0) {
        timer--;
        if (timer >= MSG_FADEOUT_TIME) {
            alpha = 255U;
        }
        else {
            float delta = ((float) timer) / ((float) MSG_FADEOUT_TIME);
            alpha = (uint8_t) (255.f * delta);
        }
        m->timer = timer;

        font_set_color(m->f, gen_color(255, 255, 255, alpha));
        font_render(m->f, m->msg, m->base.pos.x,
                m->base.pos.y,
                m->base.xscale,
                m->base.yscale);
    }
}


