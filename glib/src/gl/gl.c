
#include <stdio.h>


#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include <gl/gl.h>


static gl_context * get_window_context(GLFWwindow *w) {
    return (gl_context*) glfwGetWindowUserPointer(w);
}


static void window_resize_cb(GLFWwindow *w, int width, int height) {
    gl_context * c = get_window_context(w);
    glfwGetFramebufferSize(w, &width, &height);

    width_height wh = {
        .w = width,
        .h = height
    };
    c->wh = wh;
}


int gl_init(gl_context *context, GLint width, GLint height) {
    GLFWwindow * window;

    if (!glfwInit()) {
        fprintf(stderr, "GLFW window could not be initialized\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(width, height, "Game", NULL, NULL);

    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        return -1;
    }

    glfwGetFramebufferSize(window, &width, &height);

    glfwMakeContextCurrent(window); // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glfwSetWindowSizeCallback(window, &window_resize_cb);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    context->window = window;

    width_height wh = {
        .w = width,
        .h = height
    };
    context->wh = wh;

    glfwSetWindowUserPointer(window, context);

    return 0;
}

void gl_exit(gl_context *context) {
    glfwDestroyWindow(context->window);
    glfwTerminate();
}



void _gl_key_callback_proxy(GLFWwindow *w, int key, int action, int scancode,
        int mods) {

    gl_context * c = get_window_context(w);
    c->key_callback(c, key, action, scancode, mods);
}

