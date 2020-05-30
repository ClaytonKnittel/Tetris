#version 330 core

// expected width and height of board
#define WIDTH 10
#define HEIGHT 20

#define SIZEOF_UINT8 4U

// same as macros from board.h
#define N_STATES 16U
#define LOG_N_STATES 4U
#define COLOR_IDXS_PER_INT \
    ((SIZEOF_UINT8 * 8U) / LOG_N_STATES)
#define COLOR_IDX_MASK (N_STATES - 1U)


// transformation matrix to be applied to entire grid. When it's the identity,
// the board is drawn to exactly fit the unit square
uniform mat3 trans;


// relative indices of each of the four color variants
#define NORMAL 0U
#define LIGHT  1U
#define MED    2U
#define DARK   3U

// array of colors in quads (normal, light, med, dark), which is indexed into
// with the color index of each tile
uniform vec4 color_array[8 * 4];

// screen width and height
uniform uint width;
uniform uint height;

// set if the board is to be displayed in monochrome (grayed out)
uniform bool grayed;


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
    float dx = x_i / fwid;
    float dy = y_i / fhei;
    mat3 tile_trans = mat3(
            1.f / fwid, 0.f,        0.f,
            0.f,        1.f / fhei, 0.f,
            dx,         dy,         1.f);

    vec3 pos = trans * tile_trans * vec3(position, 1.f);
    gl_Position.xyw = pos;
    gl_Position.z = 0.f;

    // now find color_idx for tile
    uint state_idx = tile_idx / COLOR_IDXS_PER_INT;
    uint el_idx = tile_idx - (state_idx * COLOR_IDXS_PER_INT);
    uint state = (color_idxs[state_idx] >> (el_idx * LOG_N_STATES))
        & COLOR_IDX_MASK;

    uint color = (state & 0x7U);
    bool shadow = ((state >> 3U) & 1U) != 0U;

    vec4 col;
    if (shadow && (shade_index == NORMAL)) {
        // if this is a shadow tile, then don't color in the inside of the
        // tile
        col = vec4(0, 0, 0, 0);
    }
    else {
        col = color_array[color * 4U + shade_index];
    }

    if (grayed) {
        // if grayed out, calculate grayed color
        float avg = (col.x + col.y + col.z) / 3.f;
        p_color = vec4(avg, avg, avg, col.w);
    }
    else {
        p_color = col;
    }
}

