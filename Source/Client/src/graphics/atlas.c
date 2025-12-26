#include "atlas.h"

#include <stdio.h>

#include "sdl.h"
#include "log/log.h"
#include "engine.h"

static AtlasGroup tile_atlas_group; // 32x32
static AtlasGroup character_atlas_group; // 64x64
static AtlasGroup object_atlas_group; // 32x64
static AtlasGroup mixed_atlas_group; // everything else
static AtlasGroup ui_atlas_group; // everything UI related

static int get_atlas_utilization(const Atlas atlas) {
    const int total_space = ATLAS_SIZE_X * ATLAS_SIZE_Y;
    const int used_space = atlas.cursor.y * ATLAS_SIZE_X + atlas.cursor.x;

    const int percentage = (int) ((float) used_space * 100.0f / (float) total_space);
    return percentage;
}

static int get_sprite_count(const AtlasGroup group) {
    int total = 0;
    for (int i = 0; i < group.count; i++) {
        total += group.atlases[i].cursor.count;
    }
    return total;
}

static void init_atlas(Atlas *atlas) {
    if (atlas->texture_id) {
        log_debug("Atlas already initialized\n");
        return;
    }

    atlas->cursor.x = 0;
    atlas->cursor.y = 0;
    atlas->cursor.row_height = 0;

    create_gl_texture(ATLAS_SIZE_X, ATLAS_SIZE_Y, &atlas->texture_id, NULL);
}

static void init_atlas_group(AtlasGroup *atlas_group, const char *name) {
    snprintf(atlas_group->name, sizeof(atlas_group->name), "%s", name);
    atlas_group->count = 1;
    atlas_group->current = 0;

    init_atlas(&atlas_group->atlases[0]);
}

void init_atlas_groups(void) {
    init_atlas_group(&tile_atlas_group, "TileAtlas");
    init_atlas_group(&character_atlas_group, "CharacterAtlas");
    init_atlas_group(&object_atlas_group, "ObjectAtlas");
    init_atlas_group(&mixed_atlas_group, "MixedAtlas");
    init_atlas_group(&ui_atlas_group, "UiAtlas");
}

void log_atlas_debug_info(void) {
    xlog(3, "TileAtlasGroup");
    xlog(2, "AtlasCount=%d,SpriteCount=%d,CurrentUtil=%d%%", tile_atlas_group.count,
         get_sprite_count(tile_atlas_group), get_atlas_utilization(tile_atlas_group.atlases[tile_atlas_group.current]));

    xlog(3, "CharacterAtlasGroup");
    xlog(2, "AtlasCount=%d,SpriteCount=%d,CurrentUtil=%d%%", character_atlas_group.count,
         get_sprite_count(character_atlas_group),
         get_atlas_utilization(character_atlas_group.atlases[character_atlas_group.current]));

    xlog(3, "ObjectAtlasGroup");
    xlog(2, "AtlasCount=%d,SpriteCount=%d,CurrentUtil=%d%%", object_atlas_group.count,
         get_sprite_count(object_atlas_group),
         get_atlas_utilization(object_atlas_group.atlases[object_atlas_group.current]));

    xlog(3, "MixedAtlasGroup");
    xlog(2, "AtlasCount=%d,SpriteCount=%d,CurrentUtil=%d%%", mixed_atlas_group.count,
         get_sprite_count(mixed_atlas_group),
         get_atlas_utilization(mixed_atlas_group.atlases[mixed_atlas_group.current]));
}

static AtlasGroup *get_atlas_group_by_size(const int width, const int height) {
    if (width == 32 && height == 32) return &tile_atlas_group;
    if (width == 32 && height == 64) return &object_atlas_group;
    if (width == 64 && height == 64) return &character_atlas_group;
    return &mixed_atlas_group;
}

unsigned int add_to_atlas(SpriteData *sprite_data) {
    if (sprite_data->loaded_in_atlas) {
        log_debug("Sprite already loaded into atlas.\n");
        return sprite_data->atlas_texture;
    }

    int w = sprite_data->surface->w;
    int h = sprite_data->surface->h;


    AtlasGroup *atlasGroup;
    if (sprite_data->is_ui) {
        atlasGroup = &ui_atlas_group;
    } else {
        atlasGroup = get_atlas_group_by_size(w, h);
    }

    Atlas *atlas = &atlasGroup->atlases[atlasGroup->current];

    // Move to next row if sprite doesn't fit horizontally
    if (atlas->cursor.x + w + 2 > ATLAS_SIZE_X) {
        atlas->cursor.x = 0;
        atlas->cursor.y += atlas->cursor.row_height + 2;
        atlas->cursor.row_height = 0;
    }

    // Check for vertical overflow
    if (atlas->cursor.y + h + 2 > ATLAS_SIZE_Y) {
        atlasGroup->current++;
        atlasGroup->count++;
        init_atlas(&atlasGroup->atlases[atlasGroup->current]);
        atlas = &atlasGroup->atlases[atlasGroup->current];
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
        log_error("ERROR: OpenGL error during glTexSubImage2D: 0x%x\n", err);
        return 0;
    }
    float inset = 0.5f;

    sprite_data->uv0 = (UV){(ax + inset) / (float) ATLAS_SIZE_X, (ay + inset) / (float) ATLAS_SIZE_Y};
    sprite_data->uv1 = (UV){
        (ax + w) / (float) ATLAS_SIZE_X,
        (ay + h) / (float) ATLAS_SIZE_Y
    };

    log_debug("Added sprite to atlasGroup (%s:%d) at (%d, %d), size %dx%d, UV: (%.4f, %.4f) to (%.4f, %.4f)\n",
        atlasGroup->name, atlasGroup->current, ax, ay, w, h, sprite_data->uv0.u, sprite_data->uv0.v,
        sprite_data->uv1.u, sprite_data->uv1.v);

    atlas->cursor.count++;
    sprite_data->atlas_texture = atlas->texture_id;
    sprite_data->loaded_in_atlas = true;
    return atlas->texture_id;
}
