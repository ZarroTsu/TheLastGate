//
// Created by james on 12/2/2025.
//

#include "option_window.h"
#include "../inter.h"

#include <stdbool.h>

#include "../engine.h"
#include "../main.h"
#include "../render.h"
#include "../sound.h"

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

void options_window_render() {
    int x = GUI_SHOP_X, y = GUI_SHOP_Y;
    copyspritex(do_darkmode ? 18042 : 42, x, y, 0);

    x += 15; // Left Margin
    int row_y = y + 15; // Top Margin

    puttext(GENERAL_SETTINGS_LABEL.x, GENERAL_SETTINGS_LABEL.y, 3, "General Settings");
    row_y += 15;

    // Row 1
    copyspritex(18127, ESCAPE_CLOSES_WINDOWS_CHECKBOX.x, ESCAPE_CLOSES_WINDOWS_CHECKBOX.y, 0);
    if (app_state.escape_closes_menus_first) {
        showbar(ESCAPE_CLOSES_WINDOWS_CHECKBOX.x + 1, ESCAPE_CLOSES_WINDOWS_CHECKBOX.y + 1, 11, 11, GREEN);
    }
    puttext(ESCAPE_CLOSES_WINDOWS_LABEL1.x, ESCAPE_CLOSES_WINDOWS_LABEL1.y, 1, "Escape closes windows before canceling");
    puttext(ESCAPE_CLOSES_WINDOWS_LABEL2.x, ESCAPE_CLOSES_WINDOWS_LABEL2.y, 1, "    movement.");
    row_y += 30;

    // Row 2
    copyspritex(18127, COST_HELPER_CHECKBOX.x, COST_HELPER_CHECKBOX.y, 0);
    if (app_state.cost_helper) {
        showbar(COST_HELPER_CHECKBOX.x + 1, COST_HELPER_CHECKBOX.y + 1, 11, 11, GREEN);
    }
    puttext(COST_HELPER_LABEL1.x, COST_HELPER_LABEL1.y, 1, "Show attribute (B,W,I,A,S) when cheaper");
    puttext(COST_HELPER_LABEL2.x, COST_HELPER_LABEL2.y, 1, "    than skill.");

    // Row 3
    copyspritex(18127, GIVE_MORE_CHECKBOX.x, GIVE_MORE_CHECKBOX.y, 0);
    if (app_state.give_more) {
        showbar(GIVE_MORE_CHECKBOX.x + 1, GIVE_MORE_CHECKBOX.y + 1, 11, 11, GREEN);
    }
    puttext(GIVE_MORE_LABEL_1.x, GIVE_MORE_LABEL_1.y, 1, "Holding shift when giving an item");
    puttext(GIVE_MORE_LABEL_2.x, GIVE_MORE_LABEL_2.y, 1, "  will attempt to reload that item.");

    // Row 4
    copyspritex(18127, USE_QUEUE_CHECKBOX.x, USE_QUEUE_CHECKBOX.y, 0);
    if (app_state.use_queue) {
        showbar(USE_QUEUE_CHECKBOX.x + 1, USE_QUEUE_CHECKBOX.y + 1, 11, 11, GREEN);
    }
    puttext(USE_QUEUE_LABEL_1.x, USE_QUEUE_LABEL_1.y, 1, "Holding ctrl when issuing using");
    puttext(USE_QUEUE_LABEL_2.x, USE_QUEUE_LABEL_2.y, 1, "  will queue that use. Limit 5");


    puttext(AUDIO_VOLUME_LABEL.x, AUDIO_VOLUME_LABEL.y, 3, "Audio Volume");

    // Audio Volume Slider
    copyspritex(18125, AUDIO_VOLUME_LOWER_BUTTON.x, AUDIO_VOLUME_LOWER_BUTTON.y, 0);
    showbar(AUDIO_VOLUME_BAR.x, AUDIO_VOLUME_BAR.y, 1, 11, 0x7348);
    showbar(AUDIO_VOLUME_BAR.x, AUDIO_VOLUME_BAR.y + 5, 99, 1, 0x7348);
    showbar(AUDIO_VOLUME_BAR.x + 99, AUDIO_VOLUME_BAR.y, 1, 11, 0x7348);
    copyspritex(18126, AUDIO_VOLUME_RAISE_BUTTON.x, AUDIO_VOLUME_RAISE_BUTTON.y, 0);
    showbar(AUDIO_VOLUME_BAR.x + 1 + 9 * app_state.volume_level, AUDIO_VOLUME_BAR.y + 1, 9, 9, GREEN);

    puttext(AUDIO_VOLUME_ENABLED_LABEL.x, AUDIO_VOLUME_ENABLED_LABEL.y, 1, "Enable?");
    copyspritex(18127, AUDIO_VOLUME_ENABLED_BUTTON.x, AUDIO_VOLUME_ENABLED_BUTTON.y, 0);
    if (dosound) {
        showbar(AUDIO_VOLUME_ENABLED_BUTTON.x + 1, AUDIO_VOLUME_ENABLED_BUTTON.y + 1, 11, 11, GREEN);
    }
}

int options_window_input(int x, int y, int state) {
    int x_off = GUI_SHOP_X, y_off = GUI_SHOP_Y;

    // Not in the window, or window not visible.
    if (show_opts == 0 || x < x_off || y < y_off || x > x_off + 297 || y > y_off + 317) return 0;

    // Close Window (main button)
    if (in_button(x, y, x_off + 115, y_off + 289, 52, 19) && state == MS_LB_UP) {
        show_opts = 0;
    }

    // Close Window (x)
    if (in_button(x, y, x_off + 282, y_off, 15, 15) && state == MS_LB_UP) {
        show_opts = 0;
    }

    // Escape Closes Window Checkbox
    if (in_button(x, y, ESCAPE_CLOSES_WINDOWS_CHECKBOX.x, ESCAPE_CLOSES_WINDOWS_CHECKBOX.y, 13, 13) && state == MS_LB_UP) {
        app_state.escape_closes_menus_first = !app_state.escape_closes_menus_first;
    }

    // Cost Helper Button
    if (in_button(x, y, COST_HELPER_CHECKBOX.x, COST_HELPER_CHECKBOX.y, 13, 13) && state == MS_LB_UP) {
        app_state.cost_helper = !app_state.cost_helper;
    }

    // Give More Button
    if (in_button(x, y, GIVE_MORE_CHECKBOX.x, GIVE_MORE_CHECKBOX.y, 13, 13) && state == MS_LB_UP) {
        app_state.give_more = !app_state.give_more;
    }

    // Use Queue Button
    if (in_button(x, y, USE_QUEUE_CHECKBOX.x, USE_QUEUE_CHECKBOX.y, 13, 13) && state == MS_LB_UP) {
        app_state.use_queue = !app_state.use_queue;
    }

    // Lower Volume Button
    if (in_button(x, y, AUDIO_VOLUME_LOWER_BUTTON.x, AUDIO_VOLUME_LOWER_BUTTON.y, 13, 13) && state == MS_LB_UP) {
        if (app_state.volume_level > 0) app_state.volume_level--;
    }

    // Raise Volume Button
    if (in_button(x, y, AUDIO_VOLUME_RAISE_BUTTON.x, AUDIO_VOLUME_RAISE_BUTTON.y, 13, 13) && state == MS_LB_UP) {
        if (app_state.volume_level < 10) app_state.volume_level++;
    }

    // Enable Audio Button
    if (in_button(x, y, AUDIO_VOLUME_ENABLED_BUTTON.x, AUDIO_VOLUME_ENABLED_BUTTON.y, 13, 13) && state == MS_LB_UP) {
        dosound = !dosound;
    }

    return 1;
}

void options_window_scroll(int x, int y, int delta) {
    if (x >= AUDIO_VOLUME_BAR.x && x <= AUDIO_VOLUME_BAR.x + 99 && y >= AUDIO_VOLUME_BAR.y && y <= AUDIO_VOLUME_BAR.y + 15) {
        if (delta > 0 && app_state.volume_level < 10) app_state.volume_level++;
        else if (delta < 0 && app_state.volume_level > 0) app_state.volume_level--;
    }
}
