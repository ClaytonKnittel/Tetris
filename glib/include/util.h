#ifndef _GLIB_UTIL_H
#define _GLIB_UTIL_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdint.h>
#include <stdio.h>


/*#include <syslog.h>

#define fprintf(file, ...) syslog(LOG_ALERT, __VA_ARGS__)
#define printf(...) syslog(LOG_ALERT, __VA_ARGS__)*/


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


// rounds up a to nearest multiple of mod
#define ROUND_UP(a, mod) \
    ((((a) + ((mod) - 1)) / (mod)) * (mod))

#define ROUND_DOWN(a, mod) \
    (((a) / (mod)) * (mod))


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



/*
 * given a relative URL, gives back the full URL to be used to open a file
 */
int sysdep_url_to_abs_url(char * restrict buf, size_t buf_len, const char * restrict url);


#endif /* _GLIB_UTIL_H */
