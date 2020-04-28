#version 330 core

uniform mat3 trans;
uniform vec4 color_array[6];
uniform uint color_idx;

layout (location = 0) in vec2 position;
layout (location = 1) in uint shade_index;

out vec4 p_color;

void main() {
    vec3 pos = trans * vec3(position, 1.0);
    gl_Position.xyw = pos;
    gl_Position.z = 0.0;
    p_color = color_array[color_idx * 3u + shade_index];
}

