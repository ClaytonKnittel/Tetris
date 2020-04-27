#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <color.h>

typedef struct drawable {
    GLuint vao, vbo;
    uint64_t size;
} drawable;


/*
 * initializes a static drawable (fixed shape, variable size and position).
 *
 * Expected data layout is as follows:
 *  +------+------+-------+
 *  |  vx  |  vy  | color |
 *  +------+------+-------+
 *
 * where color each element is of width 4 bytes
 */
int gl_load_static_monochrome_drawable(drawable *d, uint32_t *data,
        size_t n_vertices);

static void gl_unload_static_monochrome_drawable(drawable *d) {
    glDeleteVertexArrays(1, &d->vao);
    glDeleteBuffers(1, &d->vbo);
}


static void gl_draw(drawable *d) {
    glBindVertexArray(d->vao);
    glDrawArrays(GL_TRIANGLES, 0, d->size);
    glBindVertexArray(0);
}


