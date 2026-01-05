#pragma once

#include "common.h"

/* Used for binding hotbar slots */
#define INVENTORY_HOTBAR_SKILL_OFFSET 100

/* Managing scroll for inventory */
enum {
    INVENTORY_TOTAL_SLOTS = MAXITEMS,
    INVENTORY_VISIBLE_ROWS = 3,
};

static int inventory_max_scroll(int columns) {
    if (columns <= 0) return 0;

    int rows = INVENTORY_TOTAL_SLOTS / columns;
    int max_scroll = rows - INVENTORY_VISIBLE_ROWS;
    return max_scroll > 0 ? max_scroll : 0;
}

/* Style Constants */
static const uint32_t INVENTORY_SCROLL_GRABBER_COLOR = 0xFF006100;
static const uint32_t INVENTORY_SCROLL_GRABBER_COLOR_ACTIVE = 0xFF009400;
static const uint32_t INVENTORY_SCROLL_FRAME_BACKGROUND_COLOR = 0xFF050017;

static const float INVENTORY_WINDOW_POSITION[2] = {258, 3};
static const float INVENTORY_WINDOW_SIZE[2] = {357, 106};

static const float INVENTORY_WINDOW_PADDING[2] = {3, 3};

static const float INVENTORY_SCROLL_SIZE[2] = {11, 100};

static const float INVENTORY_GRID_SIZE[2] = {
    INVENTORY_WINDOW_SIZE[0] - INVENTORY_SCROLL_SIZE[0] - INVENTORY_WINDOW_PADDING[0] * 2,
    100
};

static const float INVENTORY_BUTTON_SIZE[2] = {32, 32};


void inventory_render(void);
