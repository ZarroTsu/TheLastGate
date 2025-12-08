//
// Created by james on 12/2/2025.
//

#include "option_window.h"
#include "../inter.h"

#include <stdbool.h>

#include "imgui_wrapper.h"
#include "ui_common.h"
#include "../engine.h"
#include "../main.h"
#include "../render.h"
#include "../sound.h"
#include "../graphics/sdl.h"
#include "widgets.h"

extern int do_darkmode;

typedef struct {
    int x;
    int y;
} UiPosition;

static const int SECTION_MARGIN = 10;
static const int SECTION_HEIGHT = 21;

static const UiPosition GENERAL_SETTINGS_LABEL = {GUI_SHOP_X + 15, GUI_SHOP_Y + 15};

static const UiPosition ESCAPE_CLOSES_WINDOWS_SECTION = { GUI_SHOP_X + 15, GUI_SHOP_Y + 35 };
static const UiPosition ESCAPE_CLOSES_WINDOWS_CHECKBOX = {ESCAPE_CLOSES_WINDOWS_SECTION.x, ESCAPE_CLOSES_WINDOWS_SECTION.y + 4};
static const UiPosition ESCAPE_CLOSES_WINDOWS_LABEL1 = { ESCAPE_CLOSES_WINDOWS_SECTION.x + 22, ESCAPE_CLOSES_WINDOWS_SECTION.y };
static const UiPosition ESCAPE_CLOSES_WINDOWS_LABEL2 = { ESCAPE_CLOSES_WINDOWS_SECTION.x + 22, ESCAPE_CLOSES_WINDOWS_SECTION.y + 13};

static const UiPosition COST_HELPER_SECTION = {GUI_SHOP_X + 15, ESCAPE_CLOSES_WINDOWS_SECTION.y + SECTION_HEIGHT + SECTION_MARGIN};
static const UiPosition COST_HELPER_CHECKBOX = {COST_HELPER_SECTION.x, COST_HELPER_SECTION.y + 4};
static const UiPosition COST_HELPER_LABEL1 = { COST_HELPER_SECTION.x + 22, COST_HELPER_SECTION.y };
static const UiPosition COST_HELPER_LABEL2 = { COST_HELPER_SECTION.x + 22, COST_HELPER_SECTION.y + 13 };

static const UiPosition GIVE_MORE_SECTION = {GUI_SHOP_X + 15, COST_HELPER_SECTION.y + SECTION_HEIGHT + SECTION_MARGIN };
static const UiPosition GIVE_MORE_CHECKBOX = { GIVE_MORE_SECTION.x, GIVE_MORE_SECTION.y + 4 };
static const UiPosition GIVE_MORE_LABEL_1 = { GIVE_MORE_SECTION.x + 22, GIVE_MORE_SECTION.y};
static const UiPosition GIVE_MORE_LABEL_2 = { GIVE_MORE_SECTION.x + 22, GIVE_MORE_SECTION.y + 13};

static const UiPosition USE_QUEUE_SECTION = { GUI_SHOP_X + 15, GIVE_MORE_SECTION.y + SECTION_HEIGHT + SECTION_MARGIN };
static const UiPosition USE_QUEUE_CHECKBOX = { USE_QUEUE_SECTION.x, USE_QUEUE_SECTION.y + 4 };
static const UiPosition USE_QUEUE_LABEL_1 = { USE_QUEUE_SECTION.x + 22, USE_QUEUE_SECTION.y};
static const UiPosition USE_QUEUE_LABEL_2 = { USE_QUEUE_SECTION.x + 22, USE_QUEUE_SECTION.y + 13};

static const UiPosition AUDIO_VOLUME_LABEL = {GUI_SHOP_X + 15, GUI_SHOP_Y + 235};
static const UiPosition AUDIO_VOLUME_LOWER_BUTTON = {GUI_SHOP_X + 15, GUI_SHOP_Y + 250};
static const UiPosition AUDIO_VOLUME_BAR = {GUI_SHOP_X + 35, GUI_SHOP_Y + 251};
static const UiPosition AUDIO_VOLUME_RAISE_BUTTON = {GUI_SHOP_X + 143, GUI_SHOP_Y + 250};
static const UiPosition AUDIO_VOLUME_ENABLED_LABEL = {GUI_SHOP_X + 163, GUI_SHOP_Y + 252};
static const UiPosition AUDIO_VOLUME_ENABLED_BUTTON = {GUI_SHOP_X + 217, GUI_SHOP_Y + 250};

bool in_button(int mouse_x, int mouse_y, int start_x, int start_y, int width, int height) {
    return mouse_x >= start_x && mouse_x <= start_x + width && mouse_y >= start_y && mouse_y <= start_y + height;
}

static void togglable_option(const char *checkbox_label, bool* value, const char *label) {
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
    imgui_push_style_color(IMGUI_COL_FRAME_BG, 0.2f, 0.2f, 0.2f, 1.0f);          /* Background */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_HOVERED, 0.3f, 0.3f, 0.3f, 1.0f);  /* Hover */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_ACTIVE, 0.4f, 0.4f, 0.4f, 1.0f);   /* Active */
    imgui_push_style_color(IMGUI_COL_CHECK_MARK, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);  /* Checkmark */

    togglable_option("##1", &app_state.escape_closes_menus_first,
                         "Escape closes windows before canceling movement.");
    togglable_option("##2", &app_state.cost_helper, "Show attribute (B,W,I,A,S) when cheaper than the skill.");
    togglable_option("##3", &app_state.give_more,
                     "Holding shift when giving an item will attempt to reload that item.");
    togglable_option("##4", &app_state.use_queue, "Holding ctrl when issuing using will queue that use. Limit 5");

    imgui_pop_style_color(4);  /* Pop the 4 checkbox colors */
}

static void audio_settings_tab() {

    imgui_push_style_color(IMGUI_COL_TEXT, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);

    /* Customize checkbox appearance */
    imgui_push_style_color(IMGUI_COL_FRAME_BG, 0.2f, 0.2f, 0.2f, 1.0f);          /* Background */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_HOVERED, 0.3f, 0.3f, 0.3f, 1.0f);  /* Hover */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_ACTIVE, 0.4f, 0.4f, 0.4f, 1.0f);   /* Active */
    imgui_push_style_color(IMGUI_COL_CHECK_MARK, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);  /* Checkmark */

    imgui_checkbox("Audio Enabled?", &dosound);

    imgui_pop_style_color(4);  /* Pop the 4 checkbox colors */

    /* Customize slider appearance */
    imgui_push_style_color(IMGUI_COL_FRAME_BG, 0.2f, 0.2f, 0.2f, 1.0f);          /* Background */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_HOVERED, 0.3f, 0.3f, 0.3f, 1.0f);  /* Hover */
    imgui_push_style_color(IMGUI_COL_FRAME_BG_ACTIVE, 0.4f, 0.4f, 0.4f, 1.0f);   /* Active */
    imgui_push_style_color(IMGUI_COL_SLIDER_GRAB, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);  /* Slider knob */
    imgui_push_style_color(IMGUI_COL_SLIDER_GRAB_ACTIVE, GOLD_FONT_COLOR[0] * 0.8f, GOLD_FONT_COLOR[1] * 0.8f, GOLD_FONT_COLOR[2] * 0.8f, 1.0f);  /* Knob when dragging */

    imgui_slider_int("Audio Volume", &app_state.volume_level, 0, 10);

    imgui_pop_style_color(5);  /* Pop the 5 slider colors */
    imgui_pop_style_color(1);  /* Pop the text color */
}

static void keybind_settings_tab() {
    float text_width;
    imgui_calc_text_size_simple(&text_width, NULL, "Spell Hotkeys");
    imgui_push_style_color(IMGUI_COL_TEXT, BLUE_FONT_COLOR[0], BLUE_FONT_COLOR[1], BLUE_FONT_COLOR[2], 1.0f);
    imgui_center_next_item(text_width);
    imgui_text("Spell Hotkeys");
    imgui_pop_style_color(1);

    imgui_spacing();

    keybind("Spell 1", 0);
    keybind("Spell 2", 1);
    keybind("Spell 3", 2);
    keybind("Spell 4", 3);
    keybind("Spell 5", 4);
    keybind("Spell 6", 5);
    keybind("Spell 7", 6);
    keybind("Spell 8", 7);
    keybind("Spell 9", 8);
    keybind("Spell 10", 9);
    keybind("Spell 11", 10);
    keybind("Spell 12", 11);
    keybind("Spell 13", 12);
    keybind("Spell 14", 13);
    keybind("Spell 15", 14);
    keybind("Spell 16", 15);
    keybind("Spell 17", 16);
    keybind("Spell 18", 17);
    keybind("Spell 19", 18);
    keybind("Spell 20", 19);
}

static void render_imgui_version() {
    static int active_tab = 0;  /* Track which tab is active: 0 = General, 1 = Audio */


    imgui_set_next_window_pos(GUI_SHOP_X, GUI_SHOP_Y);
    imgui_set_next_windows_size(320.0f, 320.0f);

    if (imgui_begin("My Window", NULL,
                    IMGUI_WINDOW_FLAG_NO_COLLAPSE | IMGUI_WINDOW_FLAG_NO_MOVE | IMGUI_WINDOW_FLAG_NO_RESIZE |
                    IMGUI_WINDOW_FLAG_NO_TITLE_BAR | IMGUI_WINDOW_FLAG_NO_SCROLLBAR | IMGUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE)) {

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
        float button_width = text_width + 20.0f;  /* Add padding for the button */

        imgui_center_next_item(button_width);

        if (ui_button("Close", button_width, 15.0f)) {
            show_opts = false;
        }
    }
    imgui_end();
}

void options_window_render() {
    render_imgui_version();
    // int x = GUI_SHOP_X, y = GUI_SHOP_Y;
    // copyspritex(do_darkmode ? 18042 : 42, x, y, 0);
    //
    // x += 15; // Left Margin
    // int row_y = y + 15; // Top Margin
    //
    // puttext(GENERAL_SETTINGS_LABEL.x, GENERAL_SETTINGS_LABEL.y, 3, "General Settings");
    // row_y += 15;
    //
    // // Row 1
    // copyspritex(18127, ESCAPE_CLOSES_WINDOWS_CHECKBOX.x, ESCAPE_CLOSES_WINDOWS_CHECKBOX.y, 0);
    // if (app_state.escape_closes_menus_first) {
    //     showbar(ESCAPE_CLOSES_WINDOWS_CHECKBOX.x + 1, ESCAPE_CLOSES_WINDOWS_CHECKBOX.y + 1, 11, 11, GREEN);
    // }
    // puttext(ESCAPE_CLOSES_WINDOWS_LABEL1.x, ESCAPE_CLOSES_WINDOWS_LABEL1.y, 1, "Escape closes windows before canceling");
    // puttext(ESCAPE_CLOSES_WINDOWS_LABEL2.x, ESCAPE_CLOSES_WINDOWS_LABEL2.y, 1, "    movement.");
    // row_y += 30;
    //
    // // Row 2
    // copyspritex(18127, COST_HELPER_CHECKBOX.x, COST_HELPER_CHECKBOX.y, 0);
    // if (app_state.cost_helper) {
    //     showbar(COST_HELPER_CHECKBOX.x + 1, COST_HELPER_CHECKBOX.y + 1, 11, 11, GREEN);
    // }
    // puttext(COST_HELPER_LABEL1.x, COST_HELPER_LABEL1.y, 1, "Show attribute (B,W,I,A,S) when cheaper");
    // puttext(COST_HELPER_LABEL2.x, COST_HELPER_LABEL2.y, 1, "    than skill.");
    //
    // // Row 3
    // copyspritex(18127, GIVE_MORE_CHECKBOX.x, GIVE_MORE_CHECKBOX.y, 0);
    // if (app_state.give_more) {
    //     showbar(GIVE_MORE_CHECKBOX.x + 1, GIVE_MORE_CHECKBOX.y + 1, 11, 11, GREEN);
    // }
    // puttext(GIVE_MORE_LABEL_1.x, GIVE_MORE_LABEL_1.y, 1, "Holding shift when giving an item");
    // puttext(GIVE_MORE_LABEL_2.x, GIVE_MORE_LABEL_2.y, 1, "  will attempt to reload that item.");
    //
    // // Row 4
    // copyspritex(18127, USE_QUEUE_CHECKBOX.x, USE_QUEUE_CHECKBOX.y, 0);
    // if (app_state.use_queue) {
    //     showbar(USE_QUEUE_CHECKBOX.x + 1, USE_QUEUE_CHECKBOX.y + 1, 11, 11, GREEN);
    // }
    // puttext(USE_QUEUE_LABEL_1.x, USE_QUEUE_LABEL_1.y, 1, "Holding ctrl when issuing using");
    // puttext(USE_QUEUE_LABEL_2.x, USE_QUEUE_LABEL_2.y, 1, "  will queue that use. Limit 5");
    //
    //
    // puttext(AUDIO_VOLUME_LABEL.x, AUDIO_VOLUME_LABEL.y, 3, "Audio Volume");
    //
    // // Audio Volume Slider
    // copyspritex(18125, AUDIO_VOLUME_LOWER_BUTTON.x, AUDIO_VOLUME_LOWER_BUTTON.y, 0);
    // showbar(AUDIO_VOLUME_BAR.x, AUDIO_VOLUME_BAR.y, 1, 11, 0x7348);
    // showbar(AUDIO_VOLUME_BAR.x, AUDIO_VOLUME_BAR.y + 5, 99, 1, 0x7348);
    // showbar(AUDIO_VOLUME_BAR.x + 99, AUDIO_VOLUME_BAR.y, 1, 11, 0x7348);
    // copyspritex(18126, AUDIO_VOLUME_RAISE_BUTTON.x, AUDIO_VOLUME_RAISE_BUTTON.y, 0);
    // showbar(AUDIO_VOLUME_BAR.x + 1 + 9 * app_state.volume_level, AUDIO_VOLUME_BAR.y + 1, 9, 9, GREEN);
    //
    // puttext(AUDIO_VOLUME_ENABLED_LABEL.x, AUDIO_VOLUME_ENABLED_LABEL.y, 1, "Enable?");
    // copyspritex(18127, AUDIO_VOLUME_ENABLED_BUTTON.x, AUDIO_VOLUME_ENABLED_BUTTON.y, 0);
    // if (dosound) {
    //     showbar(AUDIO_VOLUME_ENABLED_BUTTON.x + 1, AUDIO_VOLUME_ENABLED_BUTTON.y + 1, 11, 11, GREEN);
    // }
}

int options_window_input(int x, int y, int state) {
    // int x_off = GUI_SHOP_X, y_off = GUI_SHOP_Y;
    //
    // // Not in the window, or window not visible.
    // if (show_opts == 0 || x < x_off || y < y_off || x > x_off + 297 || y > y_off + 317) return 0;
    //
    // // Close Window (main button)
    // if (in_button(x, y, x_off + 115, y_off + 289, 52, 19) && state == MS_LB_UP) {
    //     show_opts = 0;
    // }
    //
    // // Close Window (x)
    // if (in_button(x, y, x_off + 282, y_off, 15, 15) && state == MS_LB_UP) {
    //     show_opts = 0;
    // }
    //
    // // Escape Closes Window Checkbox
    // if (in_button(x, y, ESCAPE_CLOSES_WINDOWS_CHECKBOX.x, ESCAPE_CLOSES_WINDOWS_CHECKBOX.y, 13, 13) && state == MS_LB_UP) {
    //     app_state.escape_closes_menus_first = !app_state.escape_closes_menus_first;
    // }
    //
    // // Cost Helper Button
    // if (in_button(x, y, COST_HELPER_CHECKBOX.x, COST_HELPER_CHECKBOX.y, 13, 13) && state == MS_LB_UP) {
    //     app_state.cost_helper = !app_state.cost_helper;
    // }
    //
    // // Give More Button
    // if (in_button(x, y, GIVE_MORE_CHECKBOX.x, GIVE_MORE_CHECKBOX.y, 13, 13) && state == MS_LB_UP) {
    //     app_state.give_more = !app_state.give_more;
    // }
    //
    // // Use Queue Button
    // if (in_button(x, y, USE_QUEUE_CHECKBOX.x, USE_QUEUE_CHECKBOX.y, 13, 13) && state == MS_LB_UP) {
    //     app_state.use_queue = !app_state.use_queue;
    // }
    //
    // // Lower Volume Button
    // if (in_button(x, y, AUDIO_VOLUME_LOWER_BUTTON.x, AUDIO_VOLUME_LOWER_BUTTON.y, 13, 13) && state == MS_LB_UP) {
    //     if (app_state.volume_level > 0) app_state.volume_level--;
    // }
    //
    // // Raise Volume Button
    // if (in_button(x, y, AUDIO_VOLUME_RAISE_BUTTON.x, AUDIO_VOLUME_RAISE_BUTTON.y, 13, 13) && state == MS_LB_UP) {
    //     if (app_state.volume_level < 10) app_state.volume_level++;
    // }
    //
    // // Enable Audio Button
    // if (in_button(x, y, AUDIO_VOLUME_ENABLED_BUTTON.x, AUDIO_VOLUME_ENABLED_BUTTON.y, 13, 13) && state == MS_LB_UP) {
    //     dosound = !dosound;
    // }

    return 0;
}

void options_window_scroll(int x, int y, int delta) {
    // if (x >= AUDIO_VOLUME_BAR.x && x <= AUDIO_VOLUME_BAR.x + 99 && y >= AUDIO_VOLUME_BAR.y && y <= AUDIO_VOLUME_BAR.y + 15) {
    //     if (delta > 0 && app_state.volume_level < 10) app_state.volume_level++;
    //     else if (delta < 0 && app_state.volume_level > 0) app_state.volume_level--;
    // }
}
