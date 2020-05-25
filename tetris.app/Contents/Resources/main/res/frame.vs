#version 330 core

// transformation to be applied to entire frame
uniform mat3 trans;

// array of the three color variants (normal, light, medium dark)
uniform vec4 colors[4];

// number of blocks across the entire screen (excluding columns on the
// border)
uniform uint width;
// number of blocks that make up a column
uniform uint height;


// 2D position of vertex in single block (normalized to fit unit square exactly)
layout (location = 0) in vec2 position;

// determines which of the three variants of the color to use
// (normal, light, medium, or dark)
layout (location = 1) in uint shade_index;


// color going to fragment shader
out vec4 p_color;

void main() {
    uint tile_idx = uint(gl_InstanceID);

    float x_i = float((1 + int(width)) * (int(tile_idx) / int(height)) - 1);
    float y_i = float(tile_idx % height);

    float fwid = float(width);
    float fhei = float(height);

    // transforms normalized square (fits unit square) to its tile position
    // within the unit square
    // note: in column-major order
    float dx = x_i / fwid;
    float dy = y_i / fhei;
    mat3 tile_trans = mat3(
            1.f / fwid, 0.f,        0.f,
            0.f,        1.f / fhei, 0.f,
            dx,         dy,         1.f);

    vec3 pos = trans * tile_trans * vec3(position, 1.f);
    gl_Position.xyw = pos;
    gl_Position.z = 0.f;

    // now find color of tile
    p_color = colors[shade_index];
}

