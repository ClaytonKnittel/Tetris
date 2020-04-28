#version 330 core

// expected width and height of board
#define WIDTH 10
#define HEIGHT 20

// transformation matrix to be applied to entire grid. When it's the identity,
// the board is drawn to exactly fit the unit square
uniform mat3 trans;

// array of colors in quads (normal, light, med, dark), which is indexed into
// with the color index of each tile
uniform vec4 color_array[8 * 4];

// screen width and height
uniform uint width;
uniform uint height;


// array of colors of each tile, sent over each frame
uniform uint color_idxs[WIDTH * HEIGHT];

// 2D position of vertex (normalized to fit unit square exactly)
layout (location = 0) in vec2 position;

// determines which of the three variants of the color to use
// (normal, light, or dark)
layout (location = 1) in uint shade_index;

// color going to fragment shader
out vec4 p_color;


void main() {
    uint tile_idx = uint(gl_InstanceID);

    float x_i = float(tile_idx % width);
    float y_i = float(tile_idx / width);

    float fwid = float(width);
    float fhei = float(height);

    // transforms normalized square (fits unit square) to its tile position
    // within the unit square
    // note: in column-major order
    float dx = (2.f * x_i / fwid) - 1.f;
    float dy = (2.f * y_i / fhei) - 1.f;
    mat3 tile_trans = mat3(
            2.f / fwid, 0.f,        0.f,
            0.f,        2.f / fhei, 0.f,
            dx,         dy,         1.f);

    vec3 pos = trans * tile_trans * vec3(position, 1.f);
    gl_Position.xyw = pos;
    gl_Position.z = 0.f;

    uint color_idx = color_idxs[tile_idx];
    p_color = color_array[color_idx * 4U + shade_index];
}

