#pragma once
#include <stdint.h>

typedef enum {
    SPRITE_OVERLAY_SOULSTONE = 4496,
    SPRITE_OVERLAY_TALISMAN = 4497,
    SPRITE_OVERLAY_CORRUPTION = 6881,
    SPRITE_OVERLAY_STACK_BASE = 4000, /* + count (1-10) */
    SPRITE_OVERLAY_CATALYST_BASE = 6999, /* + catalyst_id */
} SpriteOverlayId;

typedef enum {
    LOCKABLE_ITEM_FLAG_NONE = 0,
    LOCKABLE_ITEM_FLAG_LOCKED = 1 << 0,
    LOCKABLE_ITEM_FLAG_SOULSTONE = 1 << 1,
    LOCKABLE_ITEM_FLAG_TALISMAN = 1 << 2,
    LOCKABLE_ITEM_FLAG_CORRUPTION = 1 << 3,
} LockableItemFlags;

typedef enum {
    ITEM_FLAG_NONE = 0,
    ITEM_FLAG_SOULSTONE = 1 << 0,
    ITEM_FLAG_TALISMAN = 1 << 1,
    ITEM_FLAG_CORRUPTION = 1 << 2
} ItemFlags;

typedef union {
    uint32_t raw;
    uint32_t worn_flags;
    uint32_t catalyst_id;
} ItemDisplayProperties;

/* Temporary struct for passing item data to rendering functions */
typedef struct {
    int sprite; /* Base sprite ID */
    ItemDisplayProperties properties; /* Catalyst ID for inventory, placement bits for worn, price/flags for shop */
    unsigned char stack; /* Stack size (0-10) */
    unsigned char flags; /* Lock/special flags bitfield */
} ItemDisplayInfo;

/* Helper to create ItemDisplayInfo from parallel arrays */
static inline ItemDisplayInfo make_item_display(int sprite, ItemDisplayProperties properties,
                                                unsigned char stack, unsigned char flags) {
    ItemDisplayInfo info;
    info.sprite = sprite;
    info.properties = properties;
    info.stack = stack;
    info.flags = flags;
    return info;
}

/* Render an item sprite with all overlays */
void render_lockable_item_display(const ItemDisplayInfo *item, int x, int y, int effect);

void render_worn_item_display(const ItemDisplayInfo *item, int x, int y, int effect);

void render_item_display(const ItemDisplayInfo *item, int x, int y, int effect);