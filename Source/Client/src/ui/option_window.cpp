//
// Created by james on 12/2/2025.
//

#include "option_window.hpp"

#include <stdbool.h>

#include "imgui/imgui_wrapper.h"
#include "config/keybindings.h"
#include "ui_common.h"
#include "engine.h"
#include "inter.h"
#include "ui.h"
#include "widgets.hpp"
#include "config/config.h"
#include "game/game_input.h"
#include "game/game_ui.h"
#include "graphics/sdl.h"
#include "util/math_util.h"

extern int do_darkmode;

typedef struct {
    Vec2 window_position;
    Vec2 window_size;
    Vec2 window_padding;
    Vec2 tab_button_size;
    Vec2 close_button_size;
    float bottom_bar_height;

    /* Keybind Options */
    float keybind_label_width;
    Vec2 keybind_cell_padding;
    Vec2 keybind_button_size;
    Vec2 keybind_reset_button_size;
    Vec2 keybind_reset_all_button_size;
} OptionWindowLayout;

static const OptionWindowLayout LAYOUT = {
    .window_position = {GUI_SHOP_X, GUI_SHOP_Y},
    .window_size = {320, 320},
    .window_padding = {10, 10},
    .tab_button_size = {90, 30},
    .close_button_size = {80, 20},
    .bottom_bar_height = 35,

    /* Keybind Options */
    .keybind_label_width = 120,
    .keybind_cell_padding = {2, 2},
    .keybind_button_size = {80, 20},
    .keybind_reset_button_size = {52, 20},
    .keybind_reset_all_button_size = {200, 20}
};

static const OptionWindowLayout *get_layout(void) {
    return &LAYOUT;
}

static float get_table_width(const OptionWindowLayout *layout) {
    const float cell_count = 3;
    return layout->keybind_cell_padding.x * cell_count * 2 + layout->keybind_label_width + layout->
           keybind_reset_button_size.x + layout->keybind_button_size.x;
}

bool is_resetting_keybinds = false;

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

static bool keybind_label(const OptionWindowLayout *layout, const char *label, BindingDescriptor *binding, int index) {
    /* Static state to track which keybind is being set */
    static int active_keybind_index = -1; /* -1 = none active */

    imgui_table_next_column();
    /* Calculate layout */
    float button_width = layout->keybind_button_size.x;
    float button_height = layout->keybind_button_size.y;
    float text_width;
    imgui_calc_text_size_simple(&text_width, NULL, label);
    float start_y = imgui_get_cursor_pos_y();
    float text_offset_y = (button_height - imgui_get_text_line_height()) / 2.0f;
    float text_offset_x = layout->keybind_label_width - text_width;

    /* Render label */
    imgui_push_style_color(IMGUI_COL_TEXT, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);

    imgui_set_cursor_pos_y(start_y + text_offset_y);
    imgui_set_cursor_pos_x(text_offset_x);
    imgui_text_wrapped(label);

    /* Determine button text */
    char button_text[32];
    bool is_active = (active_keybind_index == index);

    if (is_active) {
        snprintf(button_text, sizeof(button_text), "Press key...");
    } else {
        keybinding_to_string(binding->keybinding, button_text, sizeof(button_text));
    }

    /* Draw the button */
    imgui_push_id_int(2 * index);
    imgui_table_next_column();
    bool clicked = ui_button(button_text, button_width, button_height);
    if (is_active && imgui_is_item_hovered()) {
        ui_tooltip("ESC to cancel. DEL to un-bind.");
    }
    imgui_pop_id();
    imgui_push_id_int((2 * index) + 1);
    imgui_table_next_column();
    if (ui_button("Reset", layout->keybind_reset_button_size.x, layout->keybind_reset_button_size.y)) {
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
            if (has_shift) mod_flags = (KeybindModifier)(mod_flags | KEYBIND_MOD_SHIFT);
            if (has_ctrl) mod_flags = (KeybindModifier)(mod_flags | KEYBIND_MOD_CTRL);
            if (has_alt) mod_flags = (KeybindModifier)(mod_flags | KEYBIND_MOD_ALT);

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

static bool keybind(const OptionWindowLayout *layout, BindingDescriptor *binding, const int index) {
    return keybind_label(layout, binding->name, binding, index);
}

static void togglable_option(const char *checkbox_label, bool *value, const char *label) {
    float start_x = imgui_get_cursor_pos_x();
    float start_y = imgui_get_cursor_pos_y();

    /* Get layout metrics */
    float checkbox_height = imgui_get_frame_height();
    float pad_x, pad_y;
    imgui_get_style_frame_padding(&pad_x, &pad_y);
    float spacing_x, spacing_y;
    imgui_get_style_item_spacing(&spacing_x, &spacing_y);

    /* Checkbox width is approximately frame height (square checkbox) + label */
    float checkbox_label_width, checkbox_label_height;
    imgui_calc_text_size_simple(&checkbox_label_width, &checkbox_label_height, checkbox_label);
    float checkbox_total_width = checkbox_height + (checkbox_label_width > 0 ? spacing_x + checkbox_label_width : 0);

    /* Calculate available width for text wrapping */
    float avail_width;
    imgui_get_content_region_avail(&avail_width, NULL);
    float text_wrap_width = avail_width - checkbox_total_width - spacing_x;

    /* Calculate text height with wrapping */
    float text_width, text_height;
    imgui_calc_text_size(&text_width, &text_height, label, NULL, false, text_wrap_width);

    /* Calculate offset to center checkbox with wrapped text */
    float offset_y = (text_height - checkbox_height) / 2.0f;
    if (offset_y < 0) offset_y = 0;

    /* Render checkbox at centered position */
    imgui_set_cursor_pos(start_x, start_y + offset_y);
    imgui_checkbox(checkbox_label, value);

    /* Use SameLine to properly advance cursor with spacing, then get position */
    imgui_same_line_gap();
    float text_x = imgui_get_cursor_pos_x();

    /* Render text at top position */
    imgui_set_cursor_pos(text_x, start_y);
    imgui_push_text_wrap_pos(imgui_get_cursor_pos_x() + text_wrap_width);
    imgui_push_style_color(IMGUI_COL_TEXT, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);
    imgui_text_wrapped(label);
    imgui_pop_style_color(1);
    imgui_pop_text_wrap_pos();
}

static void push_checkbox_style(void) {
    imgui_push_style_color(IMGUI_COL_FRAME_BG, 0.2f, 0.2f, 0.2f, 1.0f); /* Background */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_HOVERED, 0.3f, 0.3f, 0.3f, 1.0f); /* Hover */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_ACTIVE, 0.4f, 0.4f, 0.4f, 1.0f); /* Active */
    imgui_push_style_color(IMGUI_COL_CHECK_MARK, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);
}

static void pop_checkbox_style(void) {
    imgui_pop_style_color(4);
}

static void general_settings_tab() {

    push_checkbox_style();
    /* Checkmark */
    togglable_option("##1", &g_config.ui.escape_closes_menu_first,
                     "Escape closes windows before canceling movement.");
    togglable_option("##2", &g_config.ui.cost_helper, "Show attribute (B,W,I,A,S) when cheaper than the skill.");
    togglable_option("##3", &g_config.gameplay.give_more,
                     "Holding shift when giving an item will attempt to reload that item.");
    togglable_option("##4", &g_config.gameplay.use_queue,
                     "Holding ctrl when issuing using will queue that use. Limit 5");
    togglable_option("##5", &g_config.ui.enter_to_talk,
                     "Press Enter to activate chat mode before typing. Press Enter again to send, or ESC to cancel.");
    togglable_option("##11", &g_config.ui.expanded_quick_stats,
                     "Show Hit/Parry Score with Weapon/Armor value.");


    imgui_spacing();
    imgui_push_font(font_sizes.normal);
    imgui_push_style_color(IMGUI_COL_TEXT, BLUE_FONT_COLOR[0], BLUE_FONT_COLOR[1], BLUE_FONT_COLOR[2], 1.0f);
    imgui_text("Stubborn Actions");
    imgui_same_line_gap();
    imgui_pop_font();
    imgui_pop_style_color(1);
    imgui_push_style_color(IMGUI_COL_TEXT, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);
    imgui_text("?");
    imgui_pop_style_color(1);
    if (imgui_is_item_hovered()) {
        ui_tooltip(
            "Actions that are interrupted will be retried, up to 10 times, until they succeed. Progress will reset attempts.");
    }
    imgui_spacing();
    togglable_option("##6", &g_config.gameplay.stubborn_use, "Stubborn Use");
    togglable_option("##7", &g_config.gameplay.stubborn_drop, "Stubborn Drop");
    togglable_option("##8", &g_config.gameplay.stubborn_pickup, "Stubborn Pickup");
    togglable_option("##9", &g_config.gameplay.stubborn_give, "Stubborn Give");
    togglable_option("##10", &g_config.gameplay.stubborn_move, "Stubborn Move");


    pop_checkbox_style(); /* Pop the 4 checkbox colors */
}

static void audio_settings_tab() {
    imgui_push_style_color(IMGUI_COL_TEXT, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);


    push_checkbox_style();
    /* Checkmark */
    imgui_checkbox("Audio Enabled?", &g_config.audio.sound_enabled);

    pop_checkbox_style();

    /* Customize slider appearance */
    imgui_push_style_color(IMGUI_COL_FRAME_BG, 0.2f, 0.2f, 0.2f, 1.0f); /* Background */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_HOVERED, 0.3f, 0.3f, 0.3f, 1.0f); /* Hover */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_ACTIVE, 0.4f, 0.4f, 0.4f, 1.0f); /* Active */
    imgui_push_style_color(IMGUI_COL_SLIDER_GRAB, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);
    /* Slider knob */
    imgui_push_style_color(IMGUI_COL_SLIDER_GRAB_ACTIVE, GOLD_FONT_COLOR[0] * 0.8f, GOLD_FONT_COLOR[1] * 0.8f,
                           GOLD_FONT_COLOR[2] * 0.8f, 1.0f); /* Knob when dragging */

    imgui_slider_int("Audio Volume", &g_config.audio.sound_volume, 0, 10);

    imgui_pop_style_color(5); /* Pop the 5 slider colors */
    imgui_pop_style_color(1); /* Pop the text color */
}

static void keybind_settings_tab(const OptionWindowLayout *layout) {
    int keybind_id = 0;
    float text_width;
    BindingCategory categories[] = {
        BINDING_CATEGORY_SPELL,
        BINDING_CATEGORY_SPEED,
        BINDING_CATEGORY_WORLD,
        BINDING_CATEGORY_WINDOW,
        BINDING_CATEGORY_GENERAL,
        BINDING_CATEGORY_GC
    };

    const char *category_names[] = {
        "Spell Hotkeys",
        "Speed Hotkeys",
        "World Hotkeys",
        "Window Hotkeys",
        "General Hotkeys",
        "GC Commands"
    };

    int num_categories = sizeof(categories) / sizeof(categories[0]);
    int spell_slot_index = 0; // Used for hinting spell bindings
    for (int i = 0; i < num_categories; i++) {
        BindingCategory category = categories[i];

        // Padding above header for everything after the first category
        if (i > 0) {
            imgui_spacing();
            imgui_separator();
            imgui_spacing();
        }

        imgui_calc_text_size_simple(&text_width, NULL, category_names[i]);
        imgui_push_style_color(IMGUI_COL_TEXT, BLUE_FONT_COLOR[0], BLUE_FONT_COLOR[1], BLUE_FONT_COLOR[2], 1.0f);
        imgui_center_next_item(text_width);
        imgui_text(category_names[i]);
        imgui_pop_style_color(1);

        imgui_spacing();

        imgui_center_next_item(get_table_width(layout));
        if (imgui_begin_table("Keybinding Table", 3, IMGUI_TABLE_FLAG_SIZING_FIXED_FIT)) {
            imgui_table_setup_column("Label", 0, layout->keybind_label_width, 0);
            imgui_table_setup_column("Binding", 0, layout->keybind_button_size.x, 1);
            imgui_table_setup_column("Reset", 0, layout->keybind_reset_button_size.x, 2);
            for (int j = 0; j < g_config.keybind.num_bindings; j++) {
                if (g_config.keybind.bindings[j].category == category) {
                    if (category == BINDING_CATEGORY_SPELL) {
                        if (pdata.xbutton[spell_slot_index].skill_nr != -1) {
                            char spell_label[64];
                            snprintf(spell_label, sizeof(spell_label), "%s (%s)", g_config.keybind.bindings[j].name,
                                     pdata.xbutton[spell_slot_index].name);
                            keybind_label(layout, spell_label, &g_config.keybind.bindings[j], keybind_id++);
                        }
                        spell_slot_index++;
                    } else {
                        keybind(layout, &g_config.keybind.bindings[j], keybind_id++);
                    }
                }
            }
            imgui_end_table();
        }
    }

    imgui_spacing();
    imgui_separator();
    imgui_spacing();
    imgui_center_next_item(layout->keybind_reset_all_button_size.x);
    if (ui_button(is_resetting_keybinds ? "Confirm?" : "Reset Keybinds to Defaults",
                  layout->keybind_reset_all_button_size.x, layout->keybind_reset_all_button_size.y)) {
        if (is_resetting_keybinds) {
            xlog(2, "Keybinds have been set to their defaults.");
            keybindings_init();
            is_resetting_keybinds = false;
        } else {
            is_resetting_keybinds = true;
        }
    }
}

static float get_button_spacing(const OptionWindowLayout *layout) {
    const float button_count = 3;
    const float total_width = layout->window_size.x - layout->window_padding.x * 2;
    const float empty_space = total_width - layout->tab_button_size.x * button_count;
    return empty_space / (button_count - 1);
}

void options_window_render() {
    static int active_tab = 0; /* Track which tab is active: 0 = General, 1 = Audio */
    const OptionWindowLayout *layout = get_layout();

    imgui_set_next_window_pos(layout->window_position.x, layout->window_position.y);
    imgui_set_next_windows_size(layout->window_size.x, layout->window_size.y);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_WINDOW_PADDING, layout->window_padding.x, layout->window_padding.y);

    float button_spacing = get_button_spacing(layout);
    if (imgui_begin("Options Window", NULL,
                    IMGUI_WINDOW_FLAG_NO_COLLAPSE | IMGUI_WINDOW_FLAG_NO_MOVE | IMGUI_WINDOW_FLAG_NO_RESIZE |
                    IMGUI_WINDOW_FLAG_NO_TITLE_BAR | IMGUI_WINDOW_FLAG_NO_SCROLLBAR |
                    IMGUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE)) {
        /* Custom tab buttons - fixed at top */
        if (tab_button("General", active_tab == 0, layout->tab_button_size.x)) {
            active_tab = 0;
        }
        imgui_same_line(0, button_spacing);
        if (tab_button("Audio", active_tab == 1, layout->tab_button_size.x)) {
            active_tab = 1;
        }
        imgui_same_line(0, button_spacing);
        if (tab_button("Keybinds", active_tab == 2, layout->tab_button_size.x)) {
            is_resetting_keybinds = false;
            active_tab = 2;
        }

        imgui_spacing();
        imgui_separator();
        imgui_spacing();

        /* Scrollable region for tab content - use 0 for height to auto-calculate space between tabs and close button */
        if (imgui_begin_child("tab_content", 0.0f, -layout->bottom_bar_height, false, IMGUI_WINDOW_FLAG_NONE)) {
            /* Render active tab content */
            if (active_tab == 0) {
                general_settings_tab();
            } else if (active_tab == 1) {
                audio_settings_tab();
            } else if (active_tab == 2) {
                keybind_settings_tab(layout);
            }
        }
        imgui_end_child();

        imgui_spacing();
        imgui_separator();
        imgui_spacing();

        /* Calculate actual button width: text + padding */

        imgui_center_next_item(layout->close_button_size.x);

        if (ui_button("Close", layout->close_button_size.x, layout->close_button_size.y)) {
            game_ui_state.show_options = false;
            apply_config_changes();
        }
    }
    imgui_end();
    imgui_pop_style_var(1);
}
