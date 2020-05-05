#version 330 core


in vec2 fs_tex_coords;

uniform vec4 font_color;

// single large texture containing all characters in the font
uniform sampler2D font_atlas;

// pixel color to be drawn
out vec4 p_color;


void main() {
    float alpha = texture(font_atlas, fs_tex_coords).r;
    vec4 smp = vec4(1.f, 1.f, 1.f, alpha);
    p_color = font_color * smp;
}

