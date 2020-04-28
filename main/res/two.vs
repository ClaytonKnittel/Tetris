#version 330 core

uniform mat3 trans;

layout (location = 0) in vec2 position;
layout (location = 1) in vec4 color;

out vec4 p_color;

void main() {
    vec3 pos = trans * vec3(position, 1.0);
    gl_Position.xyw = pos;
    gl_Position.z = 0.0;
    p_color = vec4(color) / 255.;
}

