#include "inventory.h"

#include <math.h>
#include <SDL_keycode.h>

#include "common.h"
#include "engine.h"
#include "inter.h"
#include "main.h"
#include "game/game_ui.h"
#include "imgui/imgui_wrapper.h"

typedef struct {
    int start_x;
    int start_y;
    int columns;
    int cell_width;
    int cell_height;
} GridLayout;

static const int window_flags =
        IMGUI_WINDOW_FLAG_NO_TITLE_BAR | IMGUI_WINDOW_FLAG_NO_RESIZE | IMGUI_WINDOW_FLAG_NO_COLLAPSE |
        IMGUI_WINDOW_FLAG_NO_BACKGROUND | IMGUI_WINDOW_FLAG_NO_SCROLLBAR | IMGUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE;
static const GridLayout inventory_layout = {0, 0, 10, 34, 34};
static const GridLayout depot_layout = {GUI_SHOP_X + 6, GUI_SHOP_Y + 6, 8, 34, 34};

static int get_modifier_keys() {
    const int mods = imgui_get_key_mods();
    int keys = 0;

    if (mods & KMOD_SHIFT) keys |= 1;
    if (mods & KMOD_CTRL) keys |= 2;
    if (mods & KMOD_ALT) keys |= 4;
    return keys;
}

static void grid_slot_to_xy(const GridLayout *layout, const int slot, int *out_x, int *out_y) {
    const int col = slot % layout->columns;
    const int row = slot / layout->columns;

    *out_x = layout->start_x + col * layout->cell_width;
    *out_y = layout->start_y + row * layout->cell_height;
}

static void push_inventory_styles() {
    imgui_push_style_var_float(IMGUI_STYLE_VAR_WINDOW_BORDER_SIZE, 0);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_ITEM_SPACING, 0, 0);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_ITEM_INNER_SPACING, 0, 0);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_WINDOW_PADDING, 3, 3);
}

static void pop_inventory_styles() {
    imgui_pop_style_var(4);
}

static void push_scroll_styles() {
    imgui_push_style_var_float(IMGUI_STYLE_VAR_GRAB_MIN_SIZE, 0);
    imgui_push_style_var_float(IMGUI_STYLE_VAR_FRAME_BORDER_SIZE, 0);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_FRAME_PADDING, 0, 0);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_ITEM_INNER_SPACING, 0, 0);
    imgui_push_style_color_32(IMGUI_COL_SLIDER_GRAB, INVENTORY_SCROLL_GRABBER_COLOR);
    imgui_push_style_color_32(IMGUI_COL_SLIDER_GRAB_ACTIVE, INVENTORY_SCROLL_GRABBER_COLOR_ACTIVE);
    imgui_push_style_color_32(IMGUI_COL_TEXT, 0x00000000);
    imgui_push_style_color_32(IMGUI_COL_FRAME_BG, INVENTORY_SCROLL_FRAME_BACKGROUND_COLOR);
    imgui_push_style_color_32(IMGUI_COL_FRAME_BG_ACTIVE, INVENTORY_SCROLL_FRAME_BACKGROUND_COLOR);
    imgui_push_style_color_32(IMGUI_COL_FRAME_BG_HOVERED, INVENTORY_SCROLL_FRAME_BACKGROUND_COLOR);
}

static void pop_scroll_styles() {
    imgui_pop_style_var(4);
    imgui_pop_style_color(6);
}

static void on_inventory_hovered(const int slot) {
    const int keys = get_modifier_keys();

    if (keys >= 2) {
        if (pl.item_info[slot].sprite) {
            if (pl.citem) cursor_type = CT_NONE;
            else cursor_type = CT_TAKE;
        } else {
            if (pl.citem) cursor_type = CT_DROP;
            else cursor_type = CT_NONE;
        }
    } else if (keys == 1) {
        if (pl.item_info[slot].sprite) {
            if (pl.citem) cursor_type = CT_SWAP;
            else cursor_type = CT_TAKE;
        } else {
            if (pl.citem) cursor_type = CT_DROP;
            else cursor_type = CT_NONE;
        }
    } else if (keys == 0) {
        if (pl.item_info[slot].sprite) cursor_type = CT_USE;
        else cursor_type = CT_NONE;
    } else {
        cursor_type = CT_NONE;
    }
}

static void on_inventory_left_click(const int slot) {
    const int keys = get_modifier_keys();

    if (keys >= 2) {
        if (game_ui_state.open_shop && game_ui_state.open_shop != 110 && game_ui_state.open_shop != 111) {
            // Sell item from inventory
            cmd3(CL_CMD_QSHOP, shop.nr, slot, game_ui_state.open_depot_page);
        } else {
            // Push or pull item stacks
            cmd3(CL_CMD_INV, 3, slot, selected_char);
        }
    } else if (keys == 1) {
        cmd3(CL_CMD_INV, 0, slot, selected_char);
    } else if (keys == 0) {
        cmd3(CL_CMD_INV, 6, slot, selected_char);
    }
}

static void on_inventory_right_click(const int slot) {
    static int first_right_click = 0;
    const int keys = get_modifier_keys();

    if (keys >= 2) {
        if (last_skill >= 60 && last_skill <= 64) {
            xlog(6, "Details panel now showing default.");
        }
        // Lock item where it is ;  TODO: fix /sort server-side before uncommenting
        //cmd3(CL_CMD_INV,4,nr+game_ui_state.inventory_scroll,selected_char);
        //pl.item_l[nr+game_ui_state.inventory_scroll] = 1-pl.item_l[nr+game_ui_state.inventory_scroll];
        if (last_skill == INVENTORY_HOTBAR_SKILL_OFFSET + slot) {
            last_skill = -1;
            xlog(6, "Inventory slot %d no longer selected for shortcut.", slot);
        } else {
            last_skill = INVENTORY_HOTBAR_SKILL_OFFSET + slot;
            if (first_right_click) {
                xlog(6, "Inventory slot %d selected for shortcut.", slot);
            } else {
                first_right_click++;
                xlog(
                    6,
                    "Inventory slot %d selected for shortcut. Right-click on one of the shortcut keys in the bottom right to set a shortcut.",
                    slot);
            }
        }
    } else if (keys == 1) {
        cmd3(CL_CMD_INV_LOOK, slot, 0, selected_char);
    } else if (keys == 0) {
        cmd3(CL_CMD_INV_LOOK, slot, 0, selected_char);
    }
}

void inventory_render() {
    static int scroll = 0;
    const int scroll_limit = inventory_max_scroll(inventory_layout.columns);
    push_inventory_styles();
    imgui_set_next_window_pos(INVENTORY_WINDOW_POSITION[0], INVENTORY_WINDOW_POSITION[1]);
    imgui_set_next_windows_size(INVENTORY_WINDOW_SIZE[0], INVENTORY_WINDOW_SIZE[1]);
    if (imgui_begin("Inventory", NULL, window_flags)) {
        if (imgui_begin_child("Inventory Grid", INVENTORY_GRID_SIZE[0], INVENTORY_GRID_SIZE[1], 0, 0)) {
            void *draw_list = imgui_get_window_draw_list();
            int win_x = imgui_get_window_pos_x();
            int win_y = imgui_get_window_pos_y();
            if (imgui_is_window_hovered()) {
                float wheel = imgui_get_mouse_wheel();
                if (wheel != 0.0f) {
                    scroll -= (int) (wheel);
                    if (scroll < 0) scroll = 0;
                    if (scroll > scroll_limit) scroll = scroll_limit;
                }
            }
            for (int i = 0; i < 30; i++) {
                imgui_push_id_int(i);
                int slot_x, slot_y;
                int inventory_slot = i + (scroll * inventory_layout.columns);
                grid_slot_to_xy(&inventory_layout, i, &slot_x, &slot_y);
                imgui_set_cursor_pos(slot_x, slot_y);
                if (imgui_invisible_button("##1", INVENTORY_BUTTON_SIZE[0], INVENTORY_BUTTON_SIZE[1])) {
                    on_inventory_left_click(inventory_slot);
                }
                if (imgui_is_item_hovered()) {
                    on_inventory_hovered(inventory_slot);
                    if (imgui_is_mouse_clicked(1)) {
                        on_inventory_right_click(inventory_slot);
                    }
                }
                if (pl.item_info[inventory_slot].sprite) {
                    bool highlighted = imgui_is_item_hovered();
                    render_lockable_item_display_imgui(draw_list, &pl.item_info[inventory_slot], win_x + slot_x,
                                                       win_y + slot_y, highlighted ? 16 : 0);

                }
                render_item_keybind_imgui(draw_list, INVENTORY_HOTBAR_SKILL_OFFSET + inventory_slot, win_x + slot_x, win_y + slot_y, INVENTORY_BUTTON_SIZE[0], INVENTORY_BUTTON_SIZE[1]);
                imgui_pop_id();
            }
            imgui_end_child();
        }
        imgui_same_line(0, 0);
        push_scroll_styles();
        imgui_vslider_int("##inv_scroll", INVENTORY_SCROLL_SIZE[0], INVENTORY_SCROLL_SIZE[1], &scroll, scroll_limit, 0);
        pop_scroll_styles();
    }
    imgui_end();
    pop_inventory_styles();
}
