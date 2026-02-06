#pragma once

#include "inter.h"
#include "math_util.h"

typedef struct {
    int dx;
    int dy;
} Offset;

static const Offset ring0[] = {
    {0, 0}
};

static const Offset ring1[] = {
    {-1, -1}, {0, -1}, {1, -1},
    {-1, 0}, {1, 0},
    {-1, 1}, {0, 1}, {1, 1}
};

static const Offset ring2[] = {
    {-2, -2}, {-1, -2}, {0, -2}, {1, -2}, {2, -2},
    {-2, -1}, {2, -1},
    {-2, 0}, {2, 0},
    {-2, 1}, {2, 1},
    {-2, 2}, {-1, 2}, {0, 2}, {1, 2}, {2, 2}
};

#define MAP_AT(x, y) (map[((y) * screen_render_distance) + (x)])

extern int find_nearest(unsigned int type, IVec2 tile_pos, Vec2 mouse_world_pos, int screen_render_distance, struct cmap *map, IVec2 *out_pos);