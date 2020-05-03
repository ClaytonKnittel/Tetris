#ifndef _GL_FONT_H
#define _GL_FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <gl/shader.h>


// maximum number of characters that can be drawn in 1 draw call
// (have to make multiple if need to make more
#define FONT_MAX_STR_LEN 32


/*
 * all given in pixels, give the location of the glyph in the texture
 * atlas
 */
typedef struct glyph {
    // x offset of lower left corner of glyph
    uint32_t x_off;
    // y offset of lower left corner of glyph
    uint32_t y_off;

    // width of glyph
    uint32_t w;
    // height of glyph
    uint32_t h;
} glyph;


typedef struct font {
    FT_Library library;
    FT_Face face;

    // GL key for texture generated in init
    GLuint texture;

    // width and height of full texture
    uint32_t tex_width;
    uint32_t tex_height;

    // uniform location of font color
    GLuint font_color_loc;

    // shader program
    program p;

    // vao and vbo for font
    GLuint vao, vbo;

    // list of x offsets of each character (indexed by char index) in texture
    // atlas
    glyph *glyphs;
} font_t;


/*
 * initializes given font, with height in pixels of font_height
 */
int font_init(font_t *f, const char * font_path, uint64_t font_height);

void font_destroy(font_t *f);

/*
 * renders text starting at position (x_pos, y_pos) (top left of first letter),
 * in text box of fixed width and given line height (height of each character)
 */
void font_render(font_t *f, const char * text, float x_pos, float y_pos,
        float width, float line_height);



#endif /* _GL_FONT_H */
