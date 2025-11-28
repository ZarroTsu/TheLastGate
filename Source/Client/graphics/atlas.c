#include "atlas.h"

#include <stdio.h>

#include "../log.h"

unsigned int tile_atlas;

AtlasCursor atlas_cursor = {0, 0, 0};

void init_atlas(void) {
    if (tile_atlas) {
        LOG("Tile atlas already initialized\n");
        return;
    }

    glGenTextures(1, &tile_atlas);
    glBindTexture(GL_TEXTURE_2D, tile_atlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_SIZE_X, ATLAS_SIZE_Y, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

unsigned int add_to_atlas(SpriteData *sprite_data) {
    if (sprite_data->loaded_in_atlas) {
        LOG("Sprite already loaded into atlas.\n");
        return tile_atlas;
    }

    LOG("Surface format: %s\n",
    SDL_GetPixelFormatName(sprite_data->surface->format->format));

    int w = sprite_data->surface->w;
    int h = sprite_data->surface->h;

    // Move to next row if sprite doesn't fit horizontally
    if (atlas_cursor.x + w + 2 > ATLAS_SIZE_X) {
        atlas_cursor.x = 0;
        atlas_cursor.y += atlas_cursor.row_height + 2;
        atlas_cursor.row_height = 0;
    }

    // Check for vertical overflow
    if (atlas_cursor.y + h + 2 > ATLAS_SIZE_Y) {
        LOG("ERROR: Atlas overflow! Sprite %dx%d won't fit at position (%d, %d)\n",
               w, h, atlas_cursor.x, atlas_cursor.y);
        LOG("Atlas cursor: x=%d, y=%d, row_height=%d\n",
               atlas_cursor.x, atlas_cursor.y, atlas_cursor.row_height);
        exit(1); // TODO: Gracefully die, but for now if this happens there is a big problem.
        return 0;
    }

    int ax = atlas_cursor.x + 1;
    int ay = atlas_cursor.y + 1;

    atlas_cursor.x += w + 1;

    if (h > atlas_cursor.row_height) atlas_cursor.row_height = h + 1;

    glBindTexture(GL_TEXTURE_2D, tile_atlas);
    glTexSubImage2D(GL_TEXTURE_2D, 0, ax, ay, w, h, GL_BGRA, GL_UNSIGNED_BYTE, sprite_data->surface->pixels);

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOG("ERROR: OpenGL error during glTexSubImage2D: 0x%x\n", err);
        return -1;
    }

    sprite_data->uv0 = (UV){ax / (float) ATLAS_SIZE_X, ay / (float) ATLAS_SIZE_Y};
    sprite_data->uv1 = (UV){
        (ax + w) / (float) ATLAS_SIZE_X,
        (ay + h) / (float) ATLAS_SIZE_Y
    };

    LOG("Added sprite to atlas at (%d, %d), size %dx%d, UV: (%.4f, %.4f) to (%.4f, %.4f)\n",
           ax, ay, w, h, sprite_data->uv0.u, sprite_data->uv0.v,
           sprite_data->uv1.u, sprite_data->uv1.v);

    sprite_data->loaded_in_atlas = true;
    return tile_atlas;
}
