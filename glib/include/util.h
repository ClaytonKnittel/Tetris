
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdint.h>
#include <stdio.h>


// reinterpret cast of int to float (keep bits same)
static float int_to_float(uint32_t i) {
    union {
        uint32_t ival;
        float fval;
    } __v = {
        .ival = i
    };
    return __v.fval;
}


#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// align must be power of two
#define ALIGN_UP(a, align) \
    (((a) + ((align) - 1)) & ~((align) - 1))


static GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        const char* error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        printf("%s | %s (%d)\n", error, file, line);
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)
