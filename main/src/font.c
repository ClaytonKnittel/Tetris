
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <font.h>
#include <util.h>


// number of floats sent to GPU per vertex being drawn
#define DATA_PER_VERT 4

// number of vertices needed for each character drawn
#define VERTS_PER_CHAR 6


int font_init(font_t *f, const char * font_path, uint64_t font_height) {
    FT_Library library;
    FT_Face face;
    FT_Long num_glyphs;
    uint32_t cum_width;
    uint32_t max_height;
    uint8_t *tex_buf;

    int error = FT_Init_FreeType(&library);
    if (error) {
        fprintf(stderr, "Error initializing library\n");
        return error;
    }
    f->library = library;

    error = FT_New_Face(library, font_path, 0, &face);
    if (error == FT_Err_Unknown_File_Format) {
        fprintf(stderr, "Unknown file format %s\n", font_path);
        FT_Done_FreeType(library);
        return error;
    }
    else if (error) {
        fprintf(stderr, "Unable to initialize face for %s\n", font_path);
        FT_Done_FreeType(library);
        return error;
    }
    f->face = face;

    num_glyphs = face->num_glyphs;

    error |= FT_Set_Pixel_Sizes(face, 0, font_height);

    if (error) {
        fprintf(stderr, "Unable to set pixel size to %llu\n", font_height);
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return error;
    }

    f->glyphs = (glyph*) malloc(num_glyphs * sizeof(glyph));
    if (f->glyphs == NULL) {
        fprintf(stderr, "Could not allocate %lu bytes\n",
                num_glyphs * sizeof(glyph));
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return -1;
    }

    cum_width = 0;
    max_height = 0;
    for (FT_UInt idx = 0; idx < num_glyphs; idx++) {

        if (FT_Load_Glyph(face, idx, FT_LOAD_RENDER) != 0) {
            fprintf(stderr, "count not load glyph %u\n", idx);
            continue;
        }

        uint32_t w = face->glyph->bitmap.width;
        uint32_t h = face->glyph->bitmap.rows;

        f->glyphs[idx].x_off = cum_width;
        f->glyphs[idx].y_off = 0;
        f->glyphs[idx].w = w;
        f->glyphs[idx].h = h;

        max_height = MAX(max_height, h);
        cum_width += w;
    }
    f->tex_width = cum_width;
    f->tex_height = max_height;

    tex_buf = (uint8_t*) malloc(cum_width * max_height * sizeof(uint8_t));
    if (tex_buf == NULL) {
        fprintf(stderr, "Could not malloc %lu bytes\n",
                cum_width * max_height * sizeof(uint8_t));
        free(f->glyphs);
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return -1;
    }

    // now go through each texture again and write the bitmap buffer into
    // tex_buf
    for (FT_UInt idx = 0; idx < num_glyphs; idx++) {

        if (FT_Load_Glyph(face, idx, FT_LOAD_RENDER) != 0) {
            fprintf(stderr, "count not load glyph %u\n", idx);
            continue;
        }

        uint8_t * b = (uint8_t *) face->glyph->bitmap.buffer;

        for (uint32_t row = 0; row < max_height; row++) {
            __builtin_memcpy(&tex_buf[row * max_height + f->glyphs[idx].x_off],
                    b, f->glyphs[idx].w * sizeof(uint8_t));
        }
    }

    // now transfer the texture buffer over
    glGenTextures(1, &f->texture);
    glBindTexture(GL_TEXTURE_2D, f->texture);
    glTexImage2D(GL_TEXTURE_2D,
            0, GL_RED, f->tex_width, f->tex_height,
            0, GL_RED, GL_UNSIGNED_BYTE, tex_buf);

    // use linear filtering for minimized textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // used nearest-neighbor filtering for magnified textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    //FT_UInt idx = FT_Get_Char_Index(face, ca);

    free(tex_buf);


    // now initialize VAO and VBO

    glGenVertexArrays(1, &f->vao);
    glBindVertexArray(f->vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
            DATA_PER_VERT * sizeof(GL_FLOAT), (GLvoid*) 0);
    // texture coordinates attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            DATA_PER_VERT * sizeof(GL_FLOAT), (GLvoid*) (2 * sizeof(float)));

    // will be filled in render
    glGenBuffers(1, &f->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, f->vbo);
    glBufferData(GL_ARRAY_BUFFER,
            FONT_MAX_STR_LEN * VERTS_PER_CHAR * DATA_PER_VERT * sizeof(float),
            NULL, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

    return 0;
}


void font_destroy(font_t *f) {
    free(f->glyphs);
    FT_Done_Face(f->face);
    FT_Done_FreeType(f->library);
}


void font_render(font_t *f, const char * text, float x_pos, float y_pos,
        float width, float line_height) {

    // buffer which will hold data to be sent to GPU
    float buffer_data[FONT_MAX_STR_LEN * VERTS_PER_CHAR * DATA_PER_VERT];

    // track where we should currently be drawing to
    float pen_x = x_pos;
    float pen_y = y_pos;

    // right border of draw region
    float max_x = x_pos + width;

    // factor by which units in pixels convert to normalized screen coords
    float px_to_norm = line_height / (float) f->tex_height;

    for (uint32_t i = 0; text[i] != '\0'; i++) {

        FL_UInt glyph_idx = FT_Get_Char_index(f->face, text[i]);
        glyph *g = &f->glyphs[glyph_idx];

        float x1 = pen_x;
        float x2 = pen_x + (px_to_norm * f->glyphs[i].w);

        float y1 = pen_y - line_height;
        float y2 = pen_y;

        if (x2 > max_x && max_x != x_pos) {
            // only skip to next line if we will overflow outside the width
            // of the text box and this isn't the first character to be drawn
            // on this line
            pen_x = x_pos;
            pen_y -= line_height;
        }

        float *char_buf = &buffer_data[i * VERTS_PER_CHAR * DATA_PER_VERT];

        // (0, 0)
        char_buf[ 0] = x1;
        char_buf[ 1] = y1;
        char_buf[ 2] = g->x_off;
        char_buf[ 3] = g->y_off;

        // (1, 0)
        char_buf[ 4] = x2;
        char_buf[ 5] = y1;
        char_buf[ 6] = g->x_off + g->w;
        char_buf[ 7] = g->y_off;

        // (1, 1)
        char_buf[ 8] = x2;
        char_buf[ 9] = y2;
        char_buf[10] = g->x_off + g->w;
        char_buf[11] = g->y_off + g->h;

        // (1, 1)
        char_buf[12] = x2;
        char_buf[13] = y2;
        char_buf[14] = g->x_off + g->w;
        char_buf[15] = g->y_off + g->h;

        // (0, 1)
        char_buf[16] = x1;
        char_buf[17] = y2;
        char_buf[18] = g->x_off;
        char_buf[19] = g->y_off + g->h;

        // (0, 0)
        char_buf[20] = x1;
        char_buf[21] = y1;
        char_buf[22] = g->x_off;
        char_buf[23] = g->y_off;
    }

}

