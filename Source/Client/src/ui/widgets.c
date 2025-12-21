#include "widgets.h"

#include "imgui/imgui_wrapper.h"
#include "ui_common.h"
#include "graphics/sdl.h"
#include "config/keybindings.h"
#include <stdio.h>

#include "engine.h"
#include "game/game_input.h"
#include "config/config.h"

bool ui_button(const char *label, float width, float height) {
    if (width < 52) width = 52;
    if (height < 19) height = 19;
    sdl_load_sprite(BUTTON_SPRITE_ID);
    GLuint button_texture = sprite_data[BUTTON_SPRITE_ID].gl_texture;

    /* Get draw list BEFORE creating widgets */
    void *draw_list = imgui_get_window_draw_list();

    /* Create invisible button */
    bool clicked = imgui_invisible_button(label, width, height);

    /* Get button rectangle */
    float min_x, min_y, max_x, max_y;
    imgui_get_item_rect_min(&min_x, &min_y);
    imgui_get_item_rect_max(&max_x, &max_y);

    /* Determine button state and tint color */
    unsigned int tint;
    unsigned int text_color;
    if (imgui_is_item_active()) {
        /* Clicked/pressed state - darker */
        tint = BUTTON_ACTIVE_TINT;
        text_color = BUTTON_TEXT_ACTIVE_COLOR;
    } else if (imgui_is_item_hovered()) {
        /* Hovered state - slightly darker */
        tint = BUTTON_HOVER_TINT;
        text_color = BUTTON_TEXT_HOVER_COLOR;
    } else {
        /* Normal state - full brightness */
        tint = BUTTON_TINT;
        text_color = BUTTON_TEXT_COLOR;
    }

    /* Draw the 9-slice button background */
    imgui_draw_list_add_image_9_slice(
        draw_list,
        (void *) (intptr_t) button_texture,
        min_x, min_y, max_x, max_y,
        BUTTON_SPRITE_BORDER_WIDTH,
        BUTTON_SPRITE_BORDER_WIDTH,
        BUTTON_SPRITE_BORDER_WIDTH,
        BUTTON_SPRITE_BORDER_WIDTH,
        sprite_data[BUTTON_SPRITE_ID].pixel_width,
        sprite_data[BUTTON_SPRITE_ID].pixel_height,
        sprite_data[BUTTON_SPRITE_ID].uv0.u,
        sprite_data[BUTTON_SPRITE_ID].uv0.v,
        sprite_data[BUTTON_SPRITE_ID].uv1.u,
        sprite_data[BUTTON_SPRITE_ID].uv1.v,
        tint
    );

    /* Draw centered text */
    const char *button_text = label;
    float text_width, text_height;
    imgui_calc_text_size_simple(&text_width, &text_height, button_text);

    float text_x = min_x + ((max_x - min_x) - text_width) / 2.0f;
    float text_y = min_y + ((max_y - min_y) - text_height) / 2.0f;

    imgui_draw_list_add_text(draw_list, text_x, text_y, text_color, button_text);

    return clicked;
}

bool tab_button(const char *label, bool is_active, float width) {
    void *draw_list = imgui_get_window_draw_list();

    /* Create invisible button for interaction */
    bool clicked = imgui_invisible_button(label, width, 30.0f);

    /* Get button rectangle */
    float min_x, min_y, max_x, max_y;
    imgui_get_item_rect_min(&min_x, &min_y);
    imgui_get_item_rect_max(&max_x, &max_y);

    /* Load button sprite */
    sdl_load_sprite(BUTTON_SPRITE_ID);
    GLuint button_texture = sprite_data[BUTTON_SPRITE_ID].gl_texture;

    /* Determine button state and tint color */
    unsigned int tint;
    if (is_active) {
        /* Active tab - brighter */
        tint = imgui_color_convert_float4_to_u32(1.0f, 1.0f, 1.0f, 1.0f);
    } else if (imgui_is_item_hovered()) {
        /* Hovered inactive tab */
        tint = imgui_color_convert_float4_to_u32(0.85f, 0.85f, 0.85f, 1.0f);
    } else {
        /* Inactive tab - darker */
        tint = imgui_color_convert_float4_to_u32(0.7f, 0.7f, 0.7f, 1.0f);
    }

    /* Draw the 9-slice button background */
    imgui_draw_list_add_image_9_slice(
        draw_list,
        (void *) (intptr_t) button_texture,
        min_x, min_y, max_x, max_y,
        BUTTON_SPRITE_BORDER_WIDTH,
        BUTTON_SPRITE_BORDER_WIDTH,
        BUTTON_SPRITE_BORDER_WIDTH,
        BUTTON_SPRITE_BORDER_WIDTH,
        sprite_data[BUTTON_SPRITE_ID].pixel_width,
        sprite_data[BUTTON_SPRITE_ID].pixel_height,
        sprite_data[BUTTON_SPRITE_ID].uv0.u,
        sprite_data[BUTTON_SPRITE_ID].uv0.v,
        sprite_data[BUTTON_SPRITE_ID].uv1.u,
        sprite_data[BUTTON_SPRITE_ID].uv1.v,
        tint
    );

    /* Draw centered text */
    float text_width, text_height;
    imgui_calc_text_size_simple(&text_width, &text_height, label);

    float text_x = min_x + ((max_x - min_x) - text_width) / 2.0f;
    float text_y = min_y + ((max_y - min_y) - text_height) / 2.0f;

    /* Text color: blue if active, gold if inactive */
    unsigned int text_color = is_active
                                  ? imgui_color_convert_float4_to_u32(BLUE_FONT_COLOR[0], BLUE_FONT_COLOR[1],
                                                                      BLUE_FONT_COLOR[2], 1.0f)
                                  : imgui_color_convert_float4_to_u32(GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1],
                                                                      GOLD_FONT_COLOR[2], 1.0f);

    imgui_draw_list_add_text(draw_list, text_x, text_y, text_color, label);

    return clicked;
}

static void check_and_unbind_duplicate(Keybinding new_binding, BindingDescriptor *skip_hotkey) {
    char old_binding_str[64];
    for (int i = 0; i < g_config.keybind.num_bindings; i++) {
        BindingDescriptor *hk = &g_config.keybind.bindings[i];

        if (hk == skip_hotkey) continue;

        if (hk->keybinding.key == new_binding.key && hk->keybinding.modifier == new_binding.modifier) {
            keybinding_to_string(hk->keybinding, old_binding_str, sizeof(old_binding_str));
            hk->keybinding.key = SDLK_UNKNOWN;
            hk->keybinding.modifier = KEYBIND_MOD_NONE;
            xlog(0, "Unbound %s (%s) - key reassigned.", hk->name, old_binding_str);
            return;
        }
    }
}

bool keybind(BindingDescriptor *binding, int index) {
    /* Static state to track which keybind is being set */
    static int active_keybind_index = -1; /* -1 = none active */

    /* Calculate layout */
    float text_width, text_height;
    imgui_calc_text_size_simple(&text_width, &text_height, binding->name);
    float spacing_x, spacing_y;
    imgui_get_style_item_spacing(&spacing_x, &spacing_y);

    float button_width = 80.0f;
    float button_height = 20.0f;
    float total_width = text_width + spacing_x + button_width + spacing_x + 40.0f;

    /* Center the text+button pair */
    imgui_center_next_item(total_width);

    float start_y = imgui_get_cursor_pos_y();
    float text_offset = (button_height - text_height) / 2.0f;

    /* Render label */
    imgui_push_style_color(IMGUI_COL_TEXT, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);
    imgui_set_cursor_pos_y(start_y + text_offset);
    imgui_text(binding->name);

    /* Position button */
    imgui_same_line_gap();
    imgui_set_cursor_pos_y(start_y);

    /* Determine button text */
    char button_text[32];
    bool is_active = (active_keybind_index == index);

    if (is_active) {
        snprintf(button_text, sizeof(button_text), "Press key...");
    } else {
        keybinding_to_string(binding->keybinding, button_text, sizeof(button_text));
    }

    /* Draw the button */
    imgui_push_id_int(index);
    bool clicked = ui_button(button_text, button_width, button_height);
    if (is_active && imgui_is_item_hovered()) {
        ui_tooltip("ESC to cancel. DEL to un-bind.");
    }
    imgui_pop_id();
    imgui_same_line_gap();
    imgui_set_cursor_pos_y(start_y);
    imgui_push_id_int(index + 1000); // Lazy :)
    if (ui_button("Reset", 40.0f, button_height)) {
        binding->keybinding.key = binding->default_keybinding.key;
        binding->keybinding.modifier = binding->default_keybinding.modifier;
    }
    imgui_pop_id();

    /* Handle button click - enter "set mode" */
    if (clicked && !is_active) {
        active_keybind_index = index;
        waiting_for_keybind = true;
    }

    /* If in set mode for this keybind, capture input */
    if (is_active) {
        /* Check for ESC to cancel */
        if (imgui_is_key_pressed(SDLK_ESCAPE)) {
            active_keybind_index = -1; /* Cancel */
            waiting_for_keybind = false;
        }
        /* Check for DELETE to unbind */
        else if (imgui_is_key_pressed(SDLK_DELETE)) {
            binding->keybinding.key = SDLK_UNKNOWN;
            active_keybind_index = -1;
            waiting_for_keybind = false;
        }
        /* Capture any other key press */
        else {
            /* Get modifier state */
            int mods = imgui_get_key_mods();
            bool has_shift = (mods & KMOD_SHIFT) != 0;
            bool has_ctrl = (mods & KMOD_CTRL) != 0;
            bool has_alt = (mods & KMOD_ALT) != 0;

            /* Build modifier flags */
            KeybindModifier mod_flags = KEYBIND_MOD_NONE;
            if (has_shift) mod_flags |= KEYBIND_MOD_SHIFT;
            if (has_ctrl) mod_flags |= KEYBIND_MOD_CTRL;
            if (has_alt) mod_flags |= KEYBIND_MOD_ALT;

            /* Check for any key press (except ESC, DELETE, and ENTER) */
            int i;
            SDL_Keycode pressed_key = SDLK_UNKNOWN;

            /* Check printable keys: letters, numbers, symbols */
            for (i = SDLK_SPACE; i <= SDLK_z; i++) {
                if (i == SDLK_ESCAPE || i == SDLK_DELETE) continue; /* Skip reserved keys */
                if (imgui_is_key_pressed(i)) {
                    pressed_key = i;
                    break;
                }
            }

            /* Check function keys */
            if (pressed_key == SDLK_UNKNOWN) {
                for (i = SDLK_F1; i <= SDLK_F12; i++) {
                    if (imgui_is_key_pressed(i)) {
                        pressed_key = i;
                        break;
                    }
                }
            }

            /* Check arrow keys and other special keys */
            if (pressed_key == SDLK_UNKNOWN) {
                SDL_Keycode special_keys[] = {
                    SDLK_TAB, SDLK_BACKSPACE, SDLK_INSERT,
                    SDLK_HOME, SDLK_END, SDLK_PAGEUP, SDLK_PAGEDOWN,
                    SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT
                };
                int num_special_keys = sizeof(special_keys) / sizeof(special_keys[0]);
                for (i = 0; i < num_special_keys; i++) {
                    if (imgui_is_key_pressed(special_keys[i])) {
                        pressed_key = special_keys[i];
                        break;
                    }
                }
            }

            /* Check keypad keys */
            if (pressed_key == SDLK_UNKNOWN) {
                for (i = SDLK_KP_0; i <= SDLK_KP_EQUALS; i++) {
                    if (imgui_is_key_pressed(i)) {
                        pressed_key = i;
                        break;
                    }
                }
            }

            /* If valid key pressed (not ENTER, or /), update the keybinding */
            if (pressed_key != SDLK_UNKNOWN && pressed_key != SDLK_RETURN && pressed_key != SDLK_SLASH) {
                Keybinding new_binding;
                new_binding.key = pressed_key;
                new_binding.modifier = mod_flags;
                check_and_unbind_duplicate(new_binding, binding);
                binding->keybinding.key = pressed_key;
                binding->keybinding.modifier = mod_flags;
                active_keybind_index = -1; /* Exit set mode */
                waiting_for_keybind = false;
            }
            /* If ENTER was pressed, ignore it (reserved for chat) */
            else if (imgui_is_key_pressed(SDLK_RETURN) || imgui_is_key_pressed(SDLK_SLASH)) {
                /* Do nothing - ENTER is reserved */
            }
        }
    }

    imgui_pop_style_color(1);

    return clicked;
}

void ui_tooltip(const char *tooltip_text) {
    /* Custom styled tooltip with 9-slice background */
    float padding = 8.0f;
    float max_width = 200.0f;

    /* Calculate text size with padding */
    float text_width, text_height;
    imgui_calc_text_size(&text_width, &text_height, tooltip_text, NULL, false, max_width - padding * 2);

    /* Set padding for tooltip window */
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_WINDOW_PADDING, padding, padding);
    imgui_push_style_color(IMGUI_COL_POPUP_BG, 0.0f, 0.0f, 0.0f, 0.0f); /* Transparent background */

    imgui_begin_tooltip();

    /* Get window position and size for background */
    float win_x = imgui_get_window_pos_x();
    float win_y = imgui_get_window_pos_y();
    float win_width, win_height;
    imgui_get_content_region_avail(&win_width, &win_height);

    /* Calculate actual window bounds including padding */
    float min_x = win_x;
    float min_y = win_y;
    float max_x = win_x + text_width + padding * 2;
    float max_y = win_y + text_height + padding * 2;

    /* Load and draw 9-slice background behind everything */
    sdl_load_sprite(BUTTON_SPRITE_ID);
    GLuint button_texture = sprite_data[BUTTON_SPRITE_ID].gl_texture;
    void *draw_list = imgui_get_window_draw_list();

    imgui_draw_list_add_image_9_slice(
        draw_list,
        (void *) (intptr_t) button_texture,
        min_x, min_y, max_x, max_y,
        BUTTON_SPRITE_BORDER_WIDTH,
        BUTTON_SPRITE_BORDER_WIDTH,
        BUTTON_SPRITE_BORDER_WIDTH,
        BUTTON_SPRITE_BORDER_WIDTH,
        sprite_data[BUTTON_SPRITE_ID].pixel_width,
        sprite_data[BUTTON_SPRITE_ID].pixel_height,
        sprite_data[BUTTON_SPRITE_ID].uv0.u,
        sprite_data[BUTTON_SPRITE_ID].uv0.v,
        sprite_data[BUTTON_SPRITE_ID].uv1.u,
        sprite_data[BUTTON_SPRITE_ID].uv1.v,
        BUTTON_TINT
    );

    /* Draw text with natural ImGui layout */
    imgui_push_text_wrap_pos(imgui_get_cursor_pos_x() + max_width - padding * 2);
    imgui_push_style_color(IMGUI_COL_TEXT, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);
    imgui_text_wrapped(tooltip_text);
    imgui_pop_style_color(1);
    imgui_pop_text_wrap_pos();

    imgui_end_tooltip();
    imgui_pop_style_color(1);
    imgui_pop_style_var(1);
}
