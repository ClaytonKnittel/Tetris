#ifndef _MGL_H
#define _MGL_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <gl/color.h>

typedef union wh {
    // width and height of the window
    struct {
        GLint w, h;
    };
    // width and height variable in one 64-bit integer to make
    // modifying their values atomic
    uint64_t wh;
} width_height;

typedef struct gl_context {
    GLFWwindow * window;
    width_height wh;

    // key callback function, forwarded in OpenGL key listener callback
    void (*key_callback)(struct gl_context*, int, int, int, int);

    // for user to specify, will never be modified or read by these methods
    void * user_data;
} gl_context;


int gl_init(gl_context *context, GLint width, GLint height);

void gl_exit(gl_context *context);


static void gl_set_bg_color(color_t color) {
    glClearColor(color_r(color), color_g(color),
                 color_b(color), color_a(color));
}


void _gl_key_callback_proxy(GLFWwindow *w, int key, int action, int scancode,
        int mods);

static void gl_register_key_callback(gl_context *c,
        void (*callback)(gl_context*, int key, int action, int scancode,
            int mods)) {

    c->key_callback = callback;
    glfwSetKeyCallback(c->window, &_gl_key_callback_proxy);
}


static void gl_clear(gl_context *c) {
    glClear(GL_COLOR_BUFFER_BIT);
    width_height wh = c->wh;
    glViewport(0, 0, wh.w, wh.h);
}


static void gl_render(gl_context *c) {
    // Swap buffers
    glfwSwapBuffers(c->window);
}


static int gl_should_exit(gl_context *c) {
    return glfwGetKey(c->window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
        glfwWindowShouldClose(c->window);
}

#endif /* _MGL_H */
