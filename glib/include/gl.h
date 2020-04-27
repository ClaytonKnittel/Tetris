#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <color.h>

typedef struct gl_context {
    GLFWwindow * window;

    // width and height of the window
    GLint w, h;
} gl_context;


int gl_init(gl_context *context);


static void gl_set_bg_color(color_t color) {
    glClearColor(color_r(color), color_g(color),
            color_b(color), color_a(color));
}


static void gl_clear(gl_context *c) {
    glClear(GL_COLOR_BUFFER_BIT);
}


static void gl_render(gl_context *c) {
    // Swap buffers
    glfwSwapBuffers(c->window);
    glfwPollEvents();
}


static int gl_should_exit(gl_context *c) {
    return glfwGetKey(c->window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
        glfwWindowShouldClose(c->window);
}

