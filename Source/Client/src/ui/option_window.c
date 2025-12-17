//
// Created by james on 12/2/2025.
//

#include "option_window.h"

#include <stdbool.h>

#include "imgui/imgui_wrapper.h"
#include "config/keybindings.h"
#include "ui_common.h"
#include "engine.h"
#include "ui.h"
#include "widgets.h"
#include "config/config.h"
#include "graphics/sdl.h"

extern int do_darkmode;

bool in_button(int mouse_x, int mouse_y, int start_x, int start_y, int width, int height) {
    return mouse_x >= start_x && mouse_x <= start_x + width && mouse_y >= start_y && mouse_y <= start_y + height;
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

static void general_settings_tab() {
    /* Customize checkbox appearance */
    imgui_push_style_color(IMGUI_COL_FRAME_BG, 0.2f, 0.2f, 0.2f, 1.0f); /* Background */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_HOVERED, 0.3f, 0.3f, 0.3f, 1.0f); /* Hover */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_ACTIVE, 0.4f, 0.4f, 0.4f, 1.0f); /* Active */
    imgui_push_style_color(IMGUI_COL_CHECK_MARK, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);
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


    imgui_spacing();
    imgui_push_font(font_sizes.large);
    imgui_push_style_color(IMGUI_COL_TEXT, BLUE_FONT_COLOR[0], BLUE_FONT_COLOR[1], BLUE_FONT_COLOR[2], 1.0f);
    imgui_text("Stubborn Actions"); imgui_same_line_gap();
    imgui_pop_font();
    imgui_pop_style_color(1);
    imgui_push_style_color(IMGUI_COL_TEXT, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);
    imgui_text("?");
    imgui_pop_style_color(1);
    if (imgui_is_item_hovered()) {
        ui_tooltip("Actions that are interrupted will be retried, up to 10 times, until they succeed. Progress will reset attempts.");
    }
    imgui_spacing();
    togglable_option("##6", &g_config.gameplay.stubborn_use, "Stubborn Use");
    togglable_option("##7", &g_config.gameplay.stubborn_drop, "Stubborn Drop");
    togglable_option("##8", &g_config.gameplay.stubborn_pickup, "Stubborn Pickup");
    togglable_option("##9", &g_config.gameplay.stubborn_give, "Stubborn Give");
    togglable_option("##10", &g_config.gameplay.stubborn_move, "Stubborn Move");


    imgui_pop_style_color(4); /* Pop the 4 checkbox colors */
}

static void audio_settings_tab() {
    imgui_push_style_color(IMGUI_COL_TEXT, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);

    /* Customize checkbox appearance */
    imgui_push_style_color(IMGUI_COL_FRAME_BG, 0.2f, 0.2f, 0.2f, 1.0f); /* Background */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_HOVERED, 0.3f, 0.3f, 0.3f, 1.0f); /* Hover */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_ACTIVE, 0.4f, 0.4f, 0.4f, 1.0f); /* Active */
    imgui_push_style_color(IMGUI_COL_CHECK_MARK, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);
    /* Checkmark */

    imgui_checkbox("Audio Enabled?", &g_config.audio.sound_enabled);

    imgui_pop_style_color(4); /* Pop the 4 checkbox colors */

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

static void keybind_settings_tab() {
    int keybind_id = 0;
    float text_width;
    imgui_calc_text_size_simple(&text_width, NULL, "Spell Hotkeys");
    imgui_push_style_color(IMGUI_COL_TEXT, BLUE_FONT_COLOR[0], BLUE_FONT_COLOR[1], BLUE_FONT_COLOR[2], 1.0f);
    imgui_center_next_item(text_width);
    imgui_text("Spell Hotkeys");
    imgui_pop_style_color(1);

    imgui_spacing();

    keybind("Spell 1", &g_config.keybind.spell_hotkeys[0], keybind_id++);
    keybind("Spell 2", &g_config.keybind.spell_hotkeys[1], keybind_id++);
    keybind("Spell 3", &g_config.keybind.spell_hotkeys[2], keybind_id++);
    keybind("Spell 4", &g_config.keybind.spell_hotkeys[3], keybind_id++);
    keybind("Spell 5", &g_config.keybind.spell_hotkeys[4], keybind_id++);
    keybind("Spell 6", &g_config.keybind.spell_hotkeys[5], keybind_id++);
    keybind("Spell 7", &g_config.keybind.spell_hotkeys[6], keybind_id++);
    keybind("Spell 8", &g_config.keybind.spell_hotkeys[7], keybind_id++);
    keybind("Spell 9", &g_config.keybind.spell_hotkeys[8], keybind_id++);
    keybind("Spell 10", &g_config.keybind.spell_hotkeys[9], keybind_id++);
    keybind("Spell 11", &g_config.keybind.spell_hotkeys[10], keybind_id++);
    keybind("Spell 12", &g_config.keybind.spell_hotkeys[11], keybind_id++);
    keybind("Spell 13", &g_config.keybind.spell_hotkeys[12], keybind_id++);
    keybind("Spell 14", &g_config.keybind.spell_hotkeys[13], keybind_id++);
    keybind("Spell 15", &g_config.keybind.spell_hotkeys[14], keybind_id++);
    keybind("Spell 16", &g_config.keybind.spell_hotkeys[15], keybind_id++);
    keybind("Spell 17", &g_config.keybind.spell_hotkeys[16], keybind_id++);
    keybind("Spell 18", &g_config.keybind.spell_hotkeys[17], keybind_id++);
    keybind("Spell 19", &g_config.keybind.spell_hotkeys[18], keybind_id++);
    keybind("Spell 20", &g_config.keybind.spell_hotkeys[19], keybind_id++);

    imgui_spacing();
    imgui_separator();

    imgui_calc_text_size_simple(&text_width, NULL, "Speed Hotkeys");
    imgui_push_style_color(IMGUI_COL_TEXT, BLUE_FONT_COLOR[0], BLUE_FONT_COLOR[1], BLUE_FONT_COLOR[2], 1.0f);
    imgui_center_next_item(text_width);
    imgui_text("Speed Hotkeys");
    imgui_pop_style_color(1);

    imgui_spacing();

    keybind("Fast Mode", &g_config.keybind.general_hotkeys[SPEED_FAST_HOTKEY], keybind_id++);
    keybind("Normal Mode", &g_config.keybind.general_hotkeys[SPEED_NORMAL_HOTKEY], keybind_id++);
    keybind("Slow Mode", &g_config.keybind.general_hotkeys[SPEED_SLOW_HOTKEY], keybind_id++);

    imgui_spacing();
    imgui_separator();

    imgui_calc_text_size_simple(&text_width, NULL, "World Hotkeys");
    imgui_push_style_color(IMGUI_COL_TEXT, BLUE_FONT_COLOR[0], BLUE_FONT_COLOR[1], BLUE_FONT_COLOR[2], 1.0f);
    imgui_center_next_item(text_width);
    imgui_text("World Hotkeys");
    imgui_pop_style_color(1);

    imgui_spacing();

    keybind("Toggle Health Percent", &g_config.keybind.general_hotkeys[TOGGLE_PERCENT_HOTKEY], keybind_id++);
    keybind("Toggle Stat Bases", &g_config.keybind.general_hotkeys[TOGGLE_STAT_BASE_HOTKEY], keybind_id++);
    keybind("Toggle Hide Sprites", &g_config.keybind.general_hotkeys[TOGGLE_HIDE_SPRITE_HOTKEY], keybind_id++);
    keybind("Toggle Names", &g_config.keybind.general_hotkeys[TOGGLE_NAMES_HOTKEY], keybind_id++);
    keybind("Toggle Health Bars", &g_config.keybind.general_hotkeys[TOGGLE_HEALTH_BARS_HOTKEY], keybind_id++);

    imgui_spacing();
    imgui_separator();

    imgui_calc_text_size_simple(&text_width, NULL, "Window Hotkeys");
    imgui_push_style_color(IMGUI_COL_TEXT, BLUE_FONT_COLOR[0], BLUE_FONT_COLOR[1], BLUE_FONT_COLOR[2], 1.0f);
    imgui_center_next_item(text_width);
    imgui_text("Window Hotkeys");
    imgui_pop_style_color(1);

    imgui_spacing();

    keybind("Options Window", &g_config.keybind.general_hotkeys[TOGGLE_OPTIONS_HOTKEY], keybind_id++);
    keybind("Toggle Gamma", &g_config.keybind.general_hotkeys[TOGGLE_GAMMA_HOTKEY], keybind_id++);
    keybind("Reset Window Size", &g_config.keybind.general_hotkeys[RESET_WINDOW_SIZE_HOTKEY], keybind_id++);

    imgui_spacing();
    imgui_separator();

    imgui_calc_text_size_simple(&text_width, NULL, "General Hotkeys");
    imgui_push_style_color(IMGUI_COL_TEXT, BLUE_FONT_COLOR[0], BLUE_FONT_COLOR[1], BLUE_FONT_COLOR[2], 1.0f);
    imgui_center_next_item(text_width);
    imgui_text("General Hotkeys");
    imgui_pop_style_color(1);

    imgui_spacing();

    keybind("Toggle Fightback", &g_config.keybind.general_hotkeys[FIGHTBACK_HOTKEY], keybind_id++);
    keybind("Swap Gearset", &g_config.keybind.general_hotkeys[SWAP_GEAR_HOTKEY], keybind_id++);
    keybind("Swap Position", &g_config.keybind.general_hotkeys[SWAP_POSITION_HOTKEY], keybind_id++);
    keybind("Display Debug Info", &g_config.keybind.general_hotkeys[DISPLAY_DEBUG_INFORMATION_HOTKEY], keybind_id++);
    keybind("Exit", &g_config.keybind.general_hotkeys[EXIT_HOTKEY], keybind_id++);

    imgui_spacing();
    imgui_separator();

    imgui_calc_text_size_simple(&text_width, NULL, "GC Commands");
    imgui_push_style_color(IMGUI_COL_TEXT, BLUE_FONT_COLOR[0], BLUE_FONT_COLOR[1], BLUE_FONT_COLOR[2], 1.0f);
    imgui_center_next_item(text_width);
    imgui_text("GC Commands");
    imgui_pop_style_color(1);

    imgui_spacing();

    keybind("Offense Mode", &g_config.keybind.general_hotkeys[GC_OFFENSE_HOTKEY], keybind_id++);
    keybind("Defense Mode", &g_config.keybind.general_hotkeys[GC_DEFENSE_HOTKEY], keybind_id++);
    keybind("Passive Mode", &g_config.keybind.general_hotkeys[GC_PASSIVE_HOTKEY], keybind_id++);
    keybind("Wait", &g_config.keybind.general_hotkeys[GC_WAIT_HOTKEY], keybind_id++);
    keybind("Follow", &g_config.keybind.general_hotkeys[GC_FOLLOW_HOTKEY], keybind_id++);
    keybind("Move", &g_config.keybind.general_hotkeys[GC_MOVE_HOTKEY], keybind_id++);
    keybind("Buffs", &g_config.keybind.general_hotkeys[GC_BUFFS_HOTKEY], keybind_id++);
}

void options_window_render() {
    static int active_tab = 0; /* Track which tab is active: 0 = General, 1 = Audio */


    imgui_set_next_window_pos(GUI_SHOP_X, GUI_SHOP_Y);
    imgui_set_next_windows_size(320.0f, 320.0f);

    if (imgui_begin("My Window", NULL,
                    IMGUI_WINDOW_FLAG_NO_COLLAPSE | IMGUI_WINDOW_FLAG_NO_MOVE | IMGUI_WINDOW_FLAG_NO_RESIZE |
                    IMGUI_WINDOW_FLAG_NO_TITLE_BAR | IMGUI_WINDOW_FLAG_NO_SCROLLBAR |
                    IMGUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE)) {
        /* Custom tab buttons - fixed at top */
        if (tab_button("General", active_tab == 0, 90.0f)) {
            active_tab = 0;
        }
        imgui_same_line_gap();
        if (tab_button("Audio", active_tab == 1, 90.0f)) {
            active_tab = 1;
        }
        imgui_same_line_gap();
        if (tab_button("Keybinds", active_tab == 2, 90.0f)) {
            active_tab = 2;
        }

        imgui_spacing();
        imgui_separator();
        imgui_spacing();

        /* Scrollable region for tab content - use 0 for height to auto-calculate space between tabs and close button */
        if (imgui_begin_child("tab_content", 0.0f, -35.0f, false, IMGUI_WINDOW_FLAG_NONE)) {
            /* Render active tab content */
            if (active_tab == 0) {
                general_settings_tab();
            } else if (active_tab == 1) {
                audio_settings_tab();
            } else if (active_tab == 2) {
                keybind_settings_tab();
            }
        }
        imgui_end_child();

        imgui_spacing();
        imgui_separator();
        imgui_spacing();

        /* Calculate actual button width: text + padding */
        float text_width, text_height;
        imgui_calc_text_size_simple(&text_width, &text_height, "Close");
        float button_width = text_width + 20.0f; /* Add padding for the button */

        imgui_center_next_item(button_width);

        if (ui_button("Close", button_width, 15.0f)) {
            show_opts = false;
            apply_config_changes();
        }
    }
    imgui_end();
}
