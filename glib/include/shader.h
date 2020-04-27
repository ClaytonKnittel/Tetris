

typedef struct program {
    GLuint self;
} program;


int load_program(program * p, const char * restrict vert_path,
        const char * restrict frag_path);

void use_program(program * p);

void unload_program(program * p);

