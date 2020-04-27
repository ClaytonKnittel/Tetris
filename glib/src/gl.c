
#include <stdio.h>


#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include <gl.h>


int gl_init(gl_context *context) {
    GLFWwindow * window;
    GLint width, height;

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

    window = glfwCreateWindow(1024, 768, "Game", NULL, NULL);

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
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    //glViewport(0, 0, width, height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    context->window = window;
    context->w = width;
    context->h = height;

    return 0;
}


