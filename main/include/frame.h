#ifndef _FRAME_H
#define _FRAME_H

#include <shape.h>

// frame of the tetris board


typedef struct frame {
    // tile prototype shape (will be instanced)
    shape tile_prot;

    // number of tiles from bottom to top in one of te 2 columns
    uint32_t width, height;

    program p;
    // see frame.vs for descriptions of these
    GLuint colors_loc;
    GLuint width_loc, height_loc;
} frame_t;


// initialize frame around a tetris board of given width and height
int frame_init(frame_t *f, uint32_t width, uint32_t height);

void frame_destroy(frame_t *f);


static void frame_set_pos(frame_t *f, float x, float y) {
    shape_set_pos(&f->tile_prot, x, y);
}

static void frame_set_xscale(frame_t *f, float xscale) {
    shape_set_xscale(&f->tile_prot, xscale);
}

static void frame_set_yscale(frame_t *f, float yscale) {
    shape_set_yscale(&f->tile_prot, yscale);
}

void frame_draw(frame_t *f);


#endif /* _FRAME_H */
