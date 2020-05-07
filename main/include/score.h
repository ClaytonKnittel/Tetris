#ifndef _SCORE_H
#define _SCORE_H

#include <gl/font.h>
#include <gl/shader.h>
#include <gl/font.h>

#include <shape.h>


typedef struct scoreboard {
    shape base;
    program p;
    font_t *f;

    int32_t score;
} scoreboard;



int scoreboard_init(scoreboard *s, font_t *f, float x, float y,
        float w, float h);


void scoreboard_destroy(scoreboard *s);


void scoreboard_set_score(scoreboard *s, uint32_t score);


void scoreboard_draw(scoreboard *s);


#endif /* _SCORE_H */
