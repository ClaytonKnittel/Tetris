#version 330 core


// absolute position on screen (no need to transform)
layout (location = 0) in vec2 position;

// coordinates of texture to sample from
layout (location = 1) in vec2 tex_coords;


// tex_coords given to fragment shader
out vec2 fs_tex_coords;


void main() {
    gl_Position.xy = position;
    gl_Position.z = 0.f;
    gl_Position.w = 1.f;

    fs_tex_coords = tex_coords;
}

