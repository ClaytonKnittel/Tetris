#ifndef _GL_FONT_H
#define _GL_FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <gl/shader.h>
#include <gl/color.h>


// maximum number of characters that can be drawn in 1 draw call
// (have to make multiple if need to make more
#define FONT_MAX_STR_LEN 32


/*
 * all given in pixels, give the location of the glyph in the texture
 * atlas
 */
typedef struct glyph {
    // x offset of lower left corner of glyph in atlas texture
    uint32_t x_off;
    // y offset of lower left corner of glyph in atlas texture
    uint32_t y_off;

    // width of glyph
    uint32_t w;
    // height of glyph
    uint32_t h;

    // x-offset of top left of bitmap from starting position
    int32_t bl;
    // y-offset of top left of bitmap from starting position
    int32_t bt;

    // amount to advance by in x direction for next character
    int32_t ax;
    // amount to advance by in y direction for next character
    int32_t ay;
} glyph;


typedef struct font {
    FT_Library library;
    FT_Face face;

    // width and height of full texture
    uint32_t tex_width;
    uint32_t tex_height;

    // maximum ax val for any digit (for monospace digit drawing)
    int32_t max_digit_ax;

    // GL key for texture generated in init
    GLuint texture;

    // uniform location of font color
    GLuint font_color_loc;
    color_t font_color;

    // shader program
    program p;

    // vao and vbo for font
    GLuint vao, vbo;

    // list of metadata of each character (indexed by char index) for texture
    // atlas
    glyph *glyphs;
} font_t;


/*
 * initializes given font, with height in pixels of font_height
 */
int font_init(font_t *f, const char * font_path, uint64_t font_height);

void font_destroy(font_t *f);

static void font_set_color(font_t *f, color_t color) {
    f->font_color = color;
}

/*
 * renders text starting at position (x_pos, y_pos) (top left of first letter),
 * in text box of fixed width and given line height (height of each character)
 */
void font_render(font_t *f, const char * text, float x_pos, float y_pos,
        float width, float line_height);

/*
 * same as font_render, but uses monospacing (uses line spacing of widest
 * digit)
 */
void font_render_mono_num(font_t *f, const char * text, float x_pos, float y_pos,
        float width, float line_height);



#endif /* _GL_FONT_H */
