
#include <assert.h>

#include <drawable.h>

#define STATIC_MONOCHROME_VSIZE 3

static_assert(sizeof(uint32_t) == sizeof(GLfloat), "GLfloat must be 32 bits");

int gl_load_static_monochrome_drawable(drawable *d, uint32_t *data,
        size_t n_vertices) {

    assert(n_vertices % 3 == 0);

    d->size = n_vertices;

    glGenVertexArrays(1, &d->vao);
    glBindVertexArray(d->vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &d->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, d->vbo);
    glBufferData(GL_ARRAY_BUFFER,
            sizeof(uint32_t) * STATIC_MONOCHROME_VSIZE * n_vertices,
            data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, STATIC_MONOCHROME_VSIZE *
            sizeof(uint32_t), (GLvoid*) 0);
    glVertexAttribIPointer(1, 4, GL_UNSIGNED_BYTE,
            STATIC_MONOCHROME_VSIZE * sizeof(uint32_t),
            (GLvoid*) (2 * sizeof(uint32_t)));

    glBindVertexArray(0);

    return 0;
}

