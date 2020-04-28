#include <stdio.h>
#include <stdlib.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <gl/shader.h>

#define MAX_FILE_SIZE 4096

static int gl_load_shader(GLuint * s, const char * file_path, int type) {
    GLint n_read, status;
    GLchar *buf;
    FILE * f = fopen(file_path, "r");

    if (f == NULL) {
        fprintf(stderr, "Could not open shader file \"%s\"\n", file_path);
        return -1;
    }

    buf = (GLchar*) malloc(MAX_FILE_SIZE * sizeof(GLchar));
    if (buf == NULL) {
        fprintf(stderr, "Unable to allocate %d bytes for shader program "
                "compilation\n", MAX_FILE_SIZE);
        return -1;
    }

    n_read = (GLint) fread((char*) buf, 1, MAX_FILE_SIZE, f);

    fclose(f);

    if (n_read == MAX_FILE_SIZE) {
        fprintf(stderr, "Shader file \"%s\" too large\n", file_path);
        free(buf);
        return -1;
    }

    *s = glCreateShader(type);
    glShaderSource(*s, 1, (const GLchar**) &buf, &n_read);

    free(buf);

    glCompileShader(*s);

    // check compilation status
    glGetShaderiv(*s, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLchar infoLog[512];
        glGetShaderInfoLog(*s, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER) {
            fprintf(stderr, "Vertex shader compilation error: %s\n", infoLog);
        }
        else {
            fprintf(stderr, "Fragment shader compilation error: %s\n", infoLog);
        }
    }

    return 0;
}


int gl_load_program(program * p, const char * restrict vert_path,
        const char * restrict frag_path) {
    GLint status;
    GLuint vert, frag;

    int res = gl_load_shader(&vert, vert_path, GL_VERTEX_SHADER);
    res = res || gl_load_shader(&frag, frag_path, GL_FRAGMENT_SHADER);

    if (res != 0) {
        return res;
    }

    p->self = glCreateProgram();
    glAttachShader(p->self, vert);
    glAttachShader(p->self, frag);
    glLinkProgram(p->self);

    // check linking status
    glGetProgramiv(p->self, GL_LINK_STATUS, &status);
    if (!status) {
        GLchar infoLog[512];
        glGetShaderInfoLog(p->self, 512, NULL, infoLog);
        fprintf(stderr, "Shader program linking error: %s\n", infoLog);
        res = -1;
    }

    glDetachShader(p->self, vert);
    glDetachShader(p->self, frag);

    glDeleteShader(vert);
    glDeleteShader(frag);
    return res;
}


void gl_use_program(program * p) {
    glUseProgram(p->self);
}


void gl_unload_program(program * p) {
    glDeleteProgram(p->self);
}

