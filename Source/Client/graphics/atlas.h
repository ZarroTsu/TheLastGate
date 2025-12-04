#pragma once

#define ATLAS_SIZE_X 6400
#define ATLAS_SIZE_Y 6400
#include "sprite_data.h"
#include "glad/glad.h"

typedef struct {
    int x, y;
    int row_height;
} AtlasCursor;

typedef struct {
    char name[32];
    unsigned int texture_id;
    AtlasCursor cursor;
} Atlas;

void init_atlases(void);

unsigned int add_to_atlas(SpriteData *sprite_data);
