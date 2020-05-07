
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <util.h>
#include <gl/color.h>
#include <gl/font.h>


// number of floats sent to GPU per vertex being drawn
#define DATA_PER_VERT 4

// number of vertices needed for each character drawn
#define VERTS_PER_CHAR 6


#define NORMAL_SPACING 0
#define MONO_SPACING 1


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
        f->glyphs[idx].bl = face->glyph->bitmap_left;
        f->glyphs[idx].bt = face->glyph->bitmap_top;
        f->glyphs[idx].ax = face->glyph->advance.x;
        f->glyphs[idx].ay = face->glyph->advance.y;

        max_height = MAX(max_height, h);
        // place 1-pixel boundaries between adjacent letters (so no leakage
        // when drawing
        cum_width += w + 1;
    }
    // width of memory buffer must be multiple of 4 bytes
    f->tex_width = ALIGN_UP(cum_width, 4);
    f->tex_height = max_height;

    tex_buf = (uint8_t*) calloc(f->tex_width * f->tex_height, sizeof(uint8_t));
    if (tex_buf == NULL) {
        fprintf(stderr, "Could not malloc %lu bytes\n",
                f->tex_width * f->tex_height * sizeof(uint8_t));
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

        glyph * g = &f->glyphs[idx];
        uint8_t * b = (uint8_t *) face->glyph->bitmap.buffer;

        for (int row = 0; row < g->h; row++) {
            for (int col = 0; col < g->w; col++) {
                tex_buf[(row + g->y_off) * f->tex_width + (col + g->x_off)] =
                    b[(g->h - row - 1) * g->w + col];
            }
        }
    }

    // calculate max_digit_ax
    for (char dig = '0'; dig <= '9'; dig++) {
        FT_UInt glyph_idx = FT_Get_Char_Index(f->face, dig);
        glyph *g = &f->glyphs[glyph_idx];

        f->max_digit_ax = MAX(f->max_digit_ax, g->ax);
    }

    gl_load_program(&f->p, "glib/res/font.vs", "glib/res/font.fs");
    gl_use_program(&f->p);

    f->font_color_loc = gl_uniform_location(&f->p, "font_color");
    glCheckError();
    // black by default
    f->font_color = gen_color(0, 0, 0, 255);

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

    free(tex_buf);


    // now initialize VAO and VBO

    glGenVertexArrays(1, &f->vao);
    glBindVertexArray(f->vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // will be filled in render
    glGenBuffers(1, &f->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, f->vbo);
    glBufferData(GL_ARRAY_BUFFER,
            FONT_MAX_STR_LEN * VERTS_PER_CHAR * DATA_PER_VERT * sizeof(float),
            NULL, GL_DYNAMIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
            DATA_PER_VERT * sizeof(GL_FLOAT), (GLvoid*) 0);
    // texture coordinates attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
            DATA_PER_VERT * sizeof(GL_FLOAT), (GLvoid*) (2 * sizeof(float)));

    glBindVertexArray(0);

    return 0;
}


void font_destroy(font_t *f) {
    gl_unload_program(&f->p);
    free(f->glyphs);
    FT_Done_Face(f->face);
    FT_Done_FreeType(f->library);
}


static void _font_render(font_t *f, const char * text, float x_pos, float y_pos,
        float width, float line_height, int spacing) {

    if (text[0] == '\0') {
        // ignore call to draw nothing
        return;
    }

    // buffer which will hold data to be sent to GPU
    float buffer_data[FONT_MAX_STR_LEN * VERTS_PER_CHAR * DATA_PER_VERT];

    // track where we should currently be drawing to
    float pen_x = x_pos;
    float pen_y = y_pos;

    // right border of draw region
    float max_x = x_pos + width;

    // factor by which units in pixels convert to normalized screen coords
    float px_to_norm = line_height / (float) f->tex_height;

    // make initializing render calls
    gl_use_program(&f->p);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, f->texture);
    glBindVertexArray(f->vao);

    // update color of font
    float color_vec[4] = {
        color_r(f->font_color),
        color_g(f->font_color),
        color_b(f->font_color),
        color_a(f->font_color)
    };
    glUniform4fv(f->font_color_loc, 1, color_vec);

    uint32_t i;

    while (1) {

        for (i = 0; i < FONT_MAX_STR_LEN && text[i] != '\0'; i++) {

            FT_UInt glyph_idx = FT_Get_Char_Index(f->face, text[i]);
            glyph *g = &f->glyphs[glyph_idx];

            if (glyph_idx == 0) {
                fprintf(stderr, "Unknown character \"%c\"\n", text[i]);
            }

            float x1;
            if (spacing == MONO_SPACING) {
                x1 = pen_x + (px_to_norm * f->max_digit_ax / 64.f) -
                    (px_to_norm * g->w) + (px_to_norm * g->bl);
            }
            else {
                x1 = pen_x + (px_to_norm * g->bl);
            }
            float x2 = x1 + (px_to_norm * g->w);

            float y2 = pen_y + (px_to_norm * g->bt);
            float y1 = y2 - (px_to_norm * g->h);

            float tx1 = (float) g->x_off / (float) f->tex_width;
            float tx2 = tx1 + ((float) g->w / (float) f->tex_width);

            float ty1 = (float) g->y_off / (float) f->tex_height;
            float ty2 = ty1 + ((float) g->h / (float) f->tex_height);

            if (x2 > max_x && pen_x != x_pos) {
                // only skip to next line if we will overflow outside the width
                // of the text box and this isn't the first character to be drawn
                // on this line
                pen_x = x_pos;
                pen_y -= line_height;

                // need to recalculate those values
                i--;
                continue;
            }
            else {
                if (spacing == MONO_SPACING) {
                    pen_x += (px_to_norm * f->max_digit_ax) / 64.f;
                }
                else {
                    pen_x += (px_to_norm * g->ax) / 64.f;
                }
                pen_y += (px_to_norm * g->ay) / 64.f;
            }

            float *char_buf = &buffer_data[i * VERTS_PER_CHAR * DATA_PER_VERT];

            // (0, 0)
            char_buf[ 0] = x1;
            char_buf[ 1] = y1;
            char_buf[ 2] = tx1;
            char_buf[ 3] = ty1;

            // (1, 0)
            char_buf[ 4] = x2;
            char_buf[ 5] = y1;
            char_buf[ 6] = tx2;
            char_buf[ 7] = ty1;

            // (1, 1)
            char_buf[ 8] = x2;
            char_buf[ 9] = y2;
            char_buf[10] = tx2;
            char_buf[11] = ty2;

            // (1, 1)
            char_buf[12] = x2;
            char_buf[13] = y2;
            char_buf[14] = tx2;
            char_buf[15] = ty2;

            // (0, 1)
            char_buf[16] = x1;
            char_buf[17] = y2;
            char_buf[18] = tx1;
            char_buf[19] = ty2;

            // (0, 0)
            char_buf[20] = x1;
            char_buf[21] = y1;
            char_buf[22] = tx1;
            char_buf[23] = ty1;

        }

        uint64_t n_verts = i * VERTS_PER_CHAR;
        uint64_t n_bytes = n_verts * DATA_PER_VERT * sizeof(float);

        glBindBuffer(GL_ARRAY_BUFFER, f->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, n_bytes, buffer_data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, n_verts);

        if (text[i] != '\0') {
            // make another iteration
            text += i;
            continue;
        }
        else {
            break;
        }
    }

    glBindVertexArray(0);
}


void font_render(font_t *f, const char * text, float x_pos, float y_pos,
        float width, float line_height) {
    _font_render(f, text, x_pos, y_pos, width, line_height, NORMAL_SPACING);
}

void font_render_mono_num(font_t *f, const char * text, float x_pos, float y_pos,
        float width, float line_height) {
    _font_render(f, text, x_pos, y_pos, width, line_height, MONO_SPACING);
}

