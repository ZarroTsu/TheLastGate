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
    unsigned int texture_id;
    AtlasCursor cursor;
} Atlas;

void init_atlas(void);

unsigned int add_to_atlas(SpriteData *sprite_data);
