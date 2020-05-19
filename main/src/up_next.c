
#include <assert.h>
#include <up_next.h>
#include <piece.h>


extern const float aspect_ratio;

// percentage of window for queue (remaining percentage is for text)
#define QUEUE_PERC .87f
#define PADDING .05f
#define TEXT_PERC (1.f - QUEUE_PERC - PADDING)


int up_next_init(up_next_t *u, uint32_t queue_size, float x, float y,
        float w, float h, font_t *font, tetris_t *t) {

    assert(queue_size > 0);

    uint32_t b_w = PIECE_BB_W;
    uint32_t b_h = PIECE_BB_H * queue_size;

    float board_h = QUEUE_PERC * h;
    float board_w = aspect_ratio * (board_h * b_w) / b_h;

    float board_x = x + (w - board_w) / 2;
    float board_y = y;

    board_init(&u->board, b_w, b_h);
    board_set_pos(&u->board, board_x, board_y);
    board_set_xscale(&u->board, board_w);
    board_set_yscale(&u->board, board_h);

    u->size = queue_size;
    u->font = font;
    u->t = t;

    u->text_x = x;
    u->text_y = y + board_h + (PADDING * h);
    u->text_w = w;
    u->text_h = TEXT_PERC * h;

    return 0;
}


void up_next_destroy(up_next_t *u) {
    board_destroy(&u->board);
}


void up_next_set(up_next_t *u, uint8_t *pieces) {
    board_clear(&u->board);

    piece_t p = {
        .orientation = 0,
        .board_x = 0
    };

    for (uint32_t i = 0; i < u->size; i++) {
        p.piece_idx = pieces[i];
        p.board_y = (u->size - 1 - i) * PIECE_BB_H;
        board_place_piece(&u->board, p);
    }
}


void up_next_draw(up_next_t *u) {
    // update contents of up next
    up_next_set(u, tetris_get_up_next(u->t));

    board_draw(&u->board);

    font_set_color(u->font, gen_color(255, 255, 255, 255));
    font_render(u->font, "Next", u->text_x, u->text_y, u->text_w, u->text_h);
}


