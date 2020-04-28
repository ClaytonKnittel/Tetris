
#include <assert.h>

#include <gl/drawable.h>


static_assert(sizeof(uint32_t) == sizeof(GLfloat), "GLfloat must be 32 bits");


int gl_load_static_indexed_drawable(drawable *d, uint32_t *data,
        size_t n_vertices) {

    // drawing triangles
    assert(n_vertices % 3 == 0);

    d->size = n_vertices;

    glGenVertexArrays(1, &d->vao);
    glBindVertexArray(d->vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &d->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, d->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uint32_t) * 3 * n_vertices, data,
            GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(uint32_t),
            (GLvoid*) 0);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 3 * sizeof(uint32_t),
            (GLvoid*) (2 * sizeof(uint32_t)));

    glBindVertexArray(0);

    return 0;
}


int gl_load_static_monochrome_drawable(drawable *d, uint32_t *data,
        size_t n_vertices) {

    // drawing triangles
    assert(n_vertices % 3 == 0);

    d->size = n_vertices;

    glGenVertexArrays(1, &d->vao);
    glBindVertexArray(d->vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &d->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, d->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uint32_t) * 3 * n_vertices, data,
            GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(uint32_t),
            (GLvoid*) 0);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE,
            3 * sizeof(uint32_t), (GLvoid*) (2 * sizeof(uint32_t)));

    glBindVertexArray(0);

    return 0;
}

