#pragma once

#define ATLAS_SIZE_X 4096
#define ATLAS_SIZE_Y 4096
#include "sprite_data.h"
#include "glad/glad.h"

typedef struct {
    int x, y;
    int row_height;
    int count;
} AtlasCursor;

typedef struct {
    unsigned int texture_id;
    AtlasCursor cursor;
} Atlas;

typedef struct {
    char name[32];
    Atlas atlases[10];
    int count;
    int current;
} AtlasGroup;

void init_atlas_groups(void);
void log_atlas_debug_info(void);

unsigned int add_to_atlas(SpriteData *sprite_data);
