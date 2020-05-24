
#include <hold.h>
#include <piece.h>


extern const float aspect_ratio;

// percentage of window for box (remaining percentage is for text)
#define BOX_PERC .80f
#define PADDING .07f
#define TEXT_PERC (1.f - BOX_PERC - PADDING)


int hold_init(hold_t *ph, float x, float y, float w, float h, font_t *font,
        tetris_state *s) {

    // board is just 4x4 square, will only ever hold 1 picee
    uint32_t b_w = PIECE_BB_W;
    uint32_t b_h = PIECE_BB_H;

    float board_h = BOX_PERC * h;
    float board_w = aspect_ratio * board_h;

    float board_x = x + (w - board_w) / 2;
    float board_y = y;

    board_init(&ph->board, b_w, b_h);
    board_set_pos(&ph->board, board_x, board_y);
    board_set_xscale(&ph->board, board_w);
    board_set_yscale(&ph->board, board_h);

    ph->font = font;
    ph->game_state = s;

    ph->text_x = x;
    ph->text_y = y + board_h + (PADDING * h);
    ph->text_w = w;
    ph->text_h = TEXT_PERC * h;

    return 0;
}


void hold_destroy(hold_t *h) {
    board_destroy(&h->board);
}


void hold_set(hold_t *h, uint8_t piece_idx) {
    board_clear(&h->board);

    // piece is going in default orientation in center of board
    piece_t p = {
        .piece_idx = piece_idx,
        .orientation = 0,
        .board_x = 0,
        .board_y = 0
    };
    board_place_piece(&h->board, p);
}


void hold_draw(hold_t *h) {
    // update piece in hold
    hold_set(h, h->game_state->hold.piece_idx);

    // set the board to grayed out if the up next piece in the tetris
    // game is stale
    if (piece_hold_is_stale(h->game_state)) {
        board_set_grayed(&h->board);
    }
    else {
        board_unset_grayed(&h->board);
    }

    board_draw(&h->board);

    font_set_color(h->font, gen_color(255, 255, 255, 255));
    font_render(h->font, "Hold", h->text_x, h->text_y, h->text_w, h->text_h);
}


