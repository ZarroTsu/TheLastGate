#include "item_display.h"

#include "render.h"
#include "../../common.h"  /* For copyspritex and PL_* flags */

void render_item_display(const ItemDisplayInfo *item, int x, int y, int effect) {
    if (!item || !item->sprite) return;

    /* Draw base sprite */
    copyspritex(item->sprite, x, y, effect);

    /* Draw overlays in order */
    if (item->flags & ITEM_FLAG_SOULSTONE)
        copyspritex(SPRITE_OVERLAY_SOULSTONE, x, y, effect);

    if (item->flags & ITEM_FLAG_TALISMAN)
        copyspritex(SPRITE_OVERLAY_TALISMAN, x, y, effect);

    if (item->flags & ITEM_FLAG_CORRUPTION)
        copyspritex(SPRITE_OVERLAY_CORRUPTION, x, y, effect);

    /* Draw catalyst overlay (properties = catalyst ID) */
    if (item->properties > 0)
        copyspritex(SPRITE_OVERLAY_CATALYST_BASE + item->properties, x, y, effect);

    /* Draw stack count overlay */
    if (item->stack > 0 && item->stack <= 10)
        copyspritex(SPRITE_OVERLAY_STACK_BASE + item->stack, x, y, effect);
}