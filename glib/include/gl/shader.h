#ifndef _SHADER_H
#define _SHADER_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

typedef struct program {
    GLuint self;
} program;


int gl_load_program(program * p, const char * restrict vert_path,
        const char * restrict frag_path);

void gl_use_program(program * p);

static GLuint gl_uniform_location(program * p, const char * name) {
    GLuint uniform = glGetUniformLocation(p->self, name);

    if (uniform == -1) {
        fprintf(stderr, "\"%s\" not a uniform variable\n", name);
    }
    return uniform;
}

void gl_unload_program(program * p);


#endif /* _SHADER_H */
