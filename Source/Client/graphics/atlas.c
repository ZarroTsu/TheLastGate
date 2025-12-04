#include "atlas.h"

#include <stdio.h>

#include "../log.h"
#include "../engine.h"

static Atlas tile_atlas; // 32x32
static Atlas character_atlas; // 64x64
static Atlas object_atlas; // 32x64
static Atlas mixed_atlas; // everything else

static int get_atlas_utilization(const Atlas atlas) {
    const int total_space = ATLAS_SIZE_X * ATLAS_SIZE_Y;
    const int used_space = atlas.cursor.y * ATLAS_SIZE_X + atlas.cursor.x;

    const int percentage = (int)((float)used_space * 100.0f / (float)total_space);
    return percentage;
}

static void init_atlas(Atlas *atlas, const char *name) {
    if (atlas->texture_id) {
        LOG("Atlas already initialized\n");
        return;
    }
    snprintf(atlas->name, sizeof(atlas->name), "%s", name);

    atlas->cursor.x = 0;
    atlas->cursor.y = 0;
    atlas->cursor.row_height = 0;

    glGenTextures(1, &atlas->texture_id);
    glBindTexture(GL_TEXTURE_2D, atlas->texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_SIZE_X, ATLAS_SIZE_Y, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void init_atlases(void) {
    init_atlas(&tile_atlas, "TileAtlas");
    init_atlas(&character_atlas, "CharacterAtlas");
    init_atlas(&object_atlas, "ObjectAtlas");
    init_atlas(&mixed_atlas, "MixedAtlas");
}

void log_atlas_debug_info(void) {
    xlog(2, "TileAtlas(Count=%d,Utilization=%d%)", tile_atlas.cursor.count, get_atlas_utilization(tile_atlas));
    xlog(2, "CharacterAtlas(Count=%d,Utilization=%d%)", character_atlas.cursor.count, get_atlas_utilization(character_atlas));
    xlog(2, "ObjectAtlas(Count=%d,Utilization=%d%)", object_atlas.cursor.count, get_atlas_utilization(object_atlas));
    xlog(2, "MixedAtlas(Count=%d,Utilization=%d%)", mixed_atlas.cursor.count, get_atlas_utilization(mixed_atlas));
}

static Atlas *get_atlas_by_size(const int width, const int height) {
    if (width == 32 && height == 32) return &tile_atlas;
    if (width == 32 && height == 64) return &object_atlas;
    if (width == 64 && height == 64) return &character_atlas;
    return &mixed_atlas;
}

unsigned int add_to_atlas(SpriteData *sprite_data) {
    if (sprite_data->loaded_in_atlas) {
        LOG("Sprite already loaded into atlas.\n");
        return sprite_data->atlas_texture;
    }

    int w = sprite_data->surface->w;
    int h = sprite_data->surface->h;

    Atlas *atlas = get_atlas_by_size(w, h);

    // Move to next row if sprite doesn't fit horizontally
    if (atlas->cursor.x + w + 2 > ATLAS_SIZE_X) {
        atlas->cursor.x = 0;
        atlas->cursor.y += atlas->cursor.row_height + 2;
        atlas->cursor.row_height = 0;
    }

    // Check for vertical overflow
    if (atlas->cursor.y + h + 2 > ATLAS_SIZE_Y) {
        LOG("ERROR: Atlas overflow! Sprite %dx%d won't fit at position (%d, %d)\n",
               w, h, atlas->cursor.x, atlas->cursor.y);
        LOG("Atlas cursor: x=%d, y=%d, row_height=%d\n",
               atlas->cursor.x, atlas->cursor.y, atlas->cursor.row_height);
        exit(1); // TODO: Gracefully die, but for now if this happens there is a big problem.
    }

    int ax = atlas->cursor.x + 1;
    int ay = atlas->cursor.y + 1;

    atlas->cursor.x += w + 1;

    if (h > atlas->cursor.row_height) atlas->cursor.row_height = h + 1;

    glBindTexture(GL_TEXTURE_2D, atlas->texture_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, ax, ay, w, h, GL_BGRA, GL_UNSIGNED_BYTE, sprite_data->surface->pixels);

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOG("ERROR: OpenGL error during glTexSubImage2D: 0x%x\n", err);
        return 0;
    }

    sprite_data->uv0 = (UV){ax / (float) ATLAS_SIZE_X, ay / (float) ATLAS_SIZE_Y};
    sprite_data->uv1 = (UV){
        (ax + w) / (float) ATLAS_SIZE_X,
        (ay + h) / (float) ATLAS_SIZE_Y
    };

    LOG("Added sprite to atlas (%s) at (%d, %d), size %dx%d, UV: (%.4f, %.4f) to (%.4f, %.4f)\n",
           atlas->name, ax, ay, w, h, sprite_data->uv0.u, sprite_data->uv0.v,
           sprite_data->uv1.u, sprite_data->uv1.v);

    atlas->cursor.count++;
    sprite_data->atlas_texture = atlas->texture_id;
    sprite_data->loaded_in_atlas = true;
    return atlas->texture_id;
}
