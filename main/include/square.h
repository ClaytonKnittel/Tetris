#ifndef _SQUARE_H
#define _SQUARE_H

#include <shape.h>


/*
 * makes a square of dimension 1x1, centered at (0.5, 0.5), width a rim
 * of width rim_w
 *
 * lrim is color of top and left sides of square (light rim), and
 * drim is color of bottom and right sides of square (dark rim)
 */
void square_init(shape *s, float rim_w, program * p);

#endif /* _SQUARE_H */
