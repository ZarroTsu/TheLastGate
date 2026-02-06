//
// Created by james on 2/5/2026.
//

#include "nearest_to_mouse.h"

#include <stdbool.h>

int find_nearest(unsigned int type, IVec2 tile_pos, Vec2 mouse_world_pos, int screen_render_distance,
                 struct cmap *map, IVec2 *out_pos) {
    const Offset *rings[] = {ring0, ring1, ring2};
    const int ring_sizes[] = {
        (int) (sizeof(ring0) / sizeof(ring0[0])),
        (int) (sizeof(ring1) / sizeof(ring1[0])),
        (int) (sizeof(ring2) / sizeof(ring2[0]))
    };

    for (int ring = 0; ring < 3; ring++) {
        float best_distance_squared = 1e30F;
        bool found = false;
        int best_x = 0;
        int best_y = 0;

        for (int i = 0; i < ring_sizes[ring]; i++) {
            int tile_x = tile_pos.x + rings[ring][i].dx;
            int tile_y = tile_pos.y + rings[ring][i].dy;
            if (!(MAP_AT(tile_x, tile_y).flags & type)) continue;

            float distance_x = ((float)tile_x + 0.5F) - mouse_world_pos.x;
            float distance_y = ((float)tile_y + 0.5F) - mouse_world_pos.y;
            float distance_squared = (distance_x * distance_x) + (distance_y * distance_y);
            if (!found || distance_squared < best_distance_squared) {
                best_distance_squared = distance_squared;
                best_x = tile_x;
                best_y = tile_y;
                found = true;
            }
        }

        if (found) {
            out_pos->x = best_x;
            out_pos->y = best_y;
            return 1;
        }
    }

    return 0;
}
