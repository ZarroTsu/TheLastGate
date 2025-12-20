#include "launcher.h"

#include <fcntl.h>
#include <io.h>
#include <stddef.h>

#include "inter.h"
#include "main.h"
#include "options.h"
#include "tinyfiledialogs.h"
#include "config/config.h"
#include "graphics/sdl.h"
#include "log/log.h"
#include "ui/ui.h"
#include "ui/imgui/imgui_wrapper.h"

static const int window_flags =
        IMGUI_WINDOW_FLAG_NO_COLLAPSE | IMGUI_WINDOW_FLAG_NO_RESIZE | IMGUI_WINDOW_FLAG_NO_MOVE |
        IMGUI_WINDOW_FLAG_NO_TITLE_BAR | IMGUI_WINDOW_FLAG_NO_SCROLLBAR | IMGUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE;

static SwitchCharacterConfirmState switch_confirm = {0};
static char simple_popover_text[250];

static const char *classes[] = {
    "Templar",
    "Mercenary",
    "Harakim",
    "Arch-Templar",
    "Skald",
    "Warrior",
    "Seyan",
    "Sorcerer",
    "Summoner",
    "Arch-Harakim",
    "Braver",
    "Lycanthrope"
};

static const char *genders[] = {
    "Male",
    "Female"
};

static const char *alphas[] = {
    "No Alpha",
    "Partial Alpha",
    "Full Alpha"
};

static const char *window_modes[] = {
    "Fullscreen",
    "Windowed"
};

static int current_class = 0;
static int current_gender = 0;

static char password[15] = "";

static const char *filterPatterns[1] = {"*.moa"};

static void set_race_gender() {
    switch (okey.race) {
        case 4: // templar M
            current_class = 0;
            current_gender = 0;
            break;
        case 5: // templar F
            current_class = 0;
            current_gender = 1;
            break;

        case 6: // mercenary M
            current_class = 1;
            current_gender = 0;
            break;
        case 7: // mercenary F
            current_class = 1;
            current_gender = 1;
            break;

        case 8: // harakim M
            current_class = 2;
            current_gender = 0;
            break;
        case 9: // harakim F
            current_class = 2;
            current_gender = 1;
            break;

        case 10: // seyan M
            current_class = 6;
            current_gender = 0;
            break;
        case 11: // seyan F
            current_class = 6;
            current_gender = 1;
            break;


        case 12: // arch templar M
            current_class = 3;
            current_gender = 0;
            break;
        case 13: // arch templar F
            current_class = 3;
            current_gender = 1;
            break;

        case 14: // pugilist M
            current_class = 4;
            current_gender = 0;
            break;
        case 15: // pugilist F
            current_class = 4;
            current_gender = 1;
            break;

        case 16: // warrior M
            current_class = 5;
            current_gender = 0;
            break;
        case 17: // warrior F
            current_class = 5;
            current_gender = 1;
            break;

        case 18: // sorcerer M
            current_class = 7;
            current_gender = 0;
            break;
        case 19: // sorcerer F
            current_class = 7;
            current_gender = 1;
            break;

        case 20: // summoner M
            current_class = 8;
            current_gender = 0;
            break;
        case 21: // summoner F
            current_class = 8;
            current_gender = 1;
            break;

        case 22: // arch harakim M
            current_class = 9;
            current_gender = 0;
            break;
        case 23: // arch harakim F
            current_class = 9;
            current_gender = 1;
            break;

        //

        case 2:
            current_class = 10;
            current_gender = 0;
            break; // braver M
        case 3:
            current_class = 10;
            current_gender = 1;
            break; // braver F

        case 1554: current_class = 11;
            current_gender = 0;
            break;

        default: current_class = 0;
            current_gender = 0;
            break;
    }
}

static void start_load_switch_character_confirmation(const char *file_path) {
    strncpy(switch_confirm.file_path, file_path, sizeof(switch_confirm.file_path) - 1);
    switch_confirm.show_first_confirmation = true;
}

static void start_new_switch_character_confirmation() {
    switch_confirm.new_character = true;
    switch_confirm.show_first_confirmation = true;
}

static void push_launcher_styles() {
    imgui_push_style_color_32(IMGUI_COL_TEXT, GOLD_COLOR_32);
    imgui_push_font(font_sizes.subheader);
}

static void pop_launcher_styles() {
    imgui_pop_style_color(1);
    imgui_pop_font();
}

static void push_header_styles() {
    imgui_push_font(font_sizes.big_header);
    imgui_push_style_color_32(IMGUI_COL_TEXT, HEADER_TEXT_COLOR_32);
    imgui_push_font(font_sizes_bold.big_header);
}

static void pop_header_styles() {
    imgui_pop_font();
    imgui_pop_style_color(1);
    imgui_pop_font();
}

static void push_sub_header_styles() {
    imgui_push_font(font_sizes.header);
    imgui_push_style_color_32(IMGUI_COL_TEXT, GOLD_COLOR_32);
}

static void pop_sub_header_styles() {
    imgui_pop_style_color(1);
    imgui_pop_font();
}

static void push_previous_characters_styles() {
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_WINDOW_PADDING, 4, 4);
    imgui_push_style_var_float(IMGUI_STYLE_VAR_CHILD_BORDER_SIZE, 2);
    imgui_push_style_color_32(IMGUI_COL_CHILD_BG, INNER_WINDOW_32);
    imgui_push_style_color_32(IMGUI_COL_BORDER, BORDERS_32);
}

static void pop_previous_characters_styles() {
    imgui_pop_style_color(2);
    imgui_pop_style_var(2);
}

static void push_button_styles() {
    imgui_push_style_var_float(IMGUI_STYLE_VAR_FRAME_BORDER_SIZE, 2);
    imgui_push_style_color_32(IMGUI_COL_BORDER, BORDERS_32);
    imgui_push_style_color_32(IMGUI_COL_BUTTON, BUTTON_COLOR_32);
    imgui_push_style_color_32(IMGUI_COL_BUTTON_HOVERED, BUTTON_COLOR_HOVERED_32);
    imgui_push_style_color_32(IMGUI_COL_BUTTON_ACTIVE, BUTTON_COLOR_ACTIVE_32);
    imgui_push_style_color_32(IMGUI_COL_TEXT, 0xFF000000);
    imgui_push_font(font_sizes_bold.subheader);
}

static void pop_button_styles() {
    imgui_pop_font();
    imgui_pop_style_color(5);
    imgui_pop_style_var(1);
}

static void push_input_styles() {
    imgui_push_style_var_float(IMGUI_STYLE_VAR_FRAME_BORDER_SIZE, 2);
    imgui_push_style_color_32(IMGUI_COL_BORDER, BORDERS_32);

    imgui_push_style_color_32(IMGUI_COL_FRAME_BG, BEIGE_COLOR_32);
    imgui_push_style_color_32(IMGUI_COL_TEXT, 0xFF000000);

    imgui_push_style_color_32(IMGUI_COL_INPUT_TEXT_CURSOR, 0xFF000000);
}

static void pop_input_styles() {
    imgui_pop_style_color(4);
    imgui_pop_style_var(1);
}

static void push_checkbox_styles() {
    imgui_push_style_var_float(IMGUI_STYLE_VAR_FRAME_BORDER_SIZE, 2);
    imgui_push_style_color_32(IMGUI_COL_BORDER, BORDERS_32);

    imgui_push_style_color_32(IMGUI_COL_FRAME_BG, BEIGE_COLOR_32);
    imgui_push_style_color_32(IMGUI_COL_FRAME_BG_HOVERED, BEIGE_COLOR_HOVERED_32);
    imgui_push_style_color_32(IMGUI_COL_FRAME_BG_ACTIVE, BEIGE_COLOR_ACTIVE_32);

    imgui_push_style_color_32(IMGUI_COL_CHECK_MARK, DROPDOWN_HIGHLIGHT_32);
}

static void pop_checkbox_styles() {
    imgui_pop_style_color(5);
    imgui_pop_style_var(1);
}

static void push_dropdown_styles() {
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_WINDOW_PADDING, 4, 4);
    imgui_push_style_color_32(IMGUI_COL_POPUP_BG, POPOVER_BACKGROUND_32);
}

static void pop_dropdown_styles() {
    imgui_pop_style_color(1);
    imgui_pop_style_var(1);
}

static void push_dropdown_input_styles() {
    imgui_push_style_var_float(IMGUI_STYLE_VAR_FRAME_BORDER_SIZE, 2);
    imgui_push_style_color_32(IMGUI_COL_BORDER, BORDERS_32);

    imgui_push_style_color_32(IMGUI_COL_FRAME_BG, BEIGE_COLOR_32);
    imgui_push_style_color_32(IMGUI_COL_FRAME_BG_HOVERED, BEIGE_COLOR_HOVERED_32);
    imgui_push_style_color_32(IMGUI_COL_FRAME_BG_ACTIVE, BEIGE_COLOR_ACTIVE_32);
    imgui_push_style_color_32(IMGUI_COL_TEXT, 0xFF000000);

    imgui_push_style_color_32(IMGUI_COL_INPUT_TEXT_CURSOR, 0xFF000000);
}

static void pop_dropdown_input_styles() {
    imgui_pop_style_color(6);
    imgui_pop_style_var(1);
}

static void push_dropdown_popover_styles() {
    imgui_push_style_var_float(IMGUI_STYLE_VAR_POPUP_BORDER_SIZE, 2);
    imgui_push_style_color_32(IMGUI_COL_HEADER, DROPDOWN_SELECTED_32);
    imgui_push_style_color_32(IMGUI_COL_HEADER_HOVERED, DROPDOWN_HIGHLIGHT_32);
    imgui_push_style_color_32(IMGUI_COL_HEADER_ACTIVE, DROPDOWN_ACTIVE_32);

    imgui_push_style_color_32(IMGUI_COL_BORDER, BORDERS_32);
}

static void pop_dropdown_popover_styles() {
    imgui_pop_style_color(4);
    imgui_pop_style_var(1);
}

static void push_slider_styles() {
    imgui_push_style_var_float(IMGUI_STYLE_VAR_FRAME_BORDER_SIZE, 2);
    imgui_push_style_color_32(IMGUI_COL_BORDER, BORDERS_32);

    imgui_push_style_color_32(IMGUI_COL_TEXT, INNER_WINDOW_32);

    imgui_push_style_color_32(IMGUI_COL_FRAME_BG, BEIGE_COLOR_32);
    imgui_push_style_color_32(IMGUI_COL_FRAME_BG_HOVERED, BEIGE_COLOR_HOVERED_32);
    imgui_push_style_color_32(IMGUI_COL_FRAME_BG_ACTIVE, BEIGE_COLOR_HOVERED_32);
    imgui_push_style_color_32(IMGUI_COL_SLIDER_GRAB, DROPDOWN_HIGHLIGHT_32);
    imgui_push_style_color_32(IMGUI_COL_SLIDER_GRAB_ACTIVE, DROPDOWN_HIGHLIGHT_32);
}

static void pop_slider_styles() {
    imgui_pop_style_color(7);
    imgui_pop_style_var(1);
}

static void shared_dropdown(const char *id, const char **options, int option_count, int *current_option) {
    push_dropdown_styles();

    push_dropdown_input_styles();
    bool opened = imgui_begin_combo(id, options[*current_option], IMGUI_COMBO_FLAG_NO_ARROW_BUTTON);
    pop_dropdown_input_styles();

    push_dropdown_popover_styles();
    if (opened) {
        for (int i = 0; i < option_count; i++) {
            bool selected = (*current_option == i);

            if (imgui_selectable(options[i], selected)) {
                *current_option = i;
            }

            if (selected) {
                imgui_set_item_default_focus();
            }
        }
        imgui_end_combo();
    }
    pop_dropdown_popover_styles();
    pop_dropdown_styles();
}

static void left_column() {
    push_header_styles();
    imgui_center_next_text("Character Selection");
    imgui_text("Character Selection");
    pop_header_styles();

    imgui_center_next_item(150 + 240);
    if (imgui_begin_table("CurrentAccount", 2,
                          IMGUI_TABLE_FLAG_BORDERS | IMGUI_TABLE_FLAG_SIZING_FIXED_FIT |
                          IMGUI_TABLE_FLAG_NO_HOST_EXTEND_X)) {
        imgui_table_setup_column("label", IMGUI_TABLE_COLUMN_FLAG_WIDTH_FIXED, 150, 0);
        imgui_table_setup_column("input", IMGUI_TABLE_COLUMN_FLAG_WIDTH_FIXED, 240, 1);

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text(okey.name);
        imgui_table_next_column();
        push_button_styles();
        if (imgui_button("Create New Account")) {
            start_new_switch_character_confirmation();
        }
        pop_button_styles();
        imgui_end_table();
    }

    imgui_center_next_item(150 + 240);
    if (imgui_begin_table("character_fields", 2,
                          IMGUI_TABLE_FLAG_BORDERS | IMGUI_TABLE_FLAG_SIZING_FIXED_FIT |
                          IMGUI_TABLE_FLAG_NO_HOST_EXTEND_X)) {
        imgui_table_setup_column("label", IMGUI_TABLE_COLUMN_FLAG_WIDTH_FIXED, 150, 0);
        imgui_table_setup_column("input", IMGUI_TABLE_COLUMN_FLAG_WIDTH_FIXED, 240, 1);

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Name");
        imgui_table_next_column();
        imgui_push_item_width(-1);
        push_input_styles();
        imgui_input_text("##name", pdata.cname, sizeof(pdata.cname));
        pop_input_styles();
        imgui_pop_item_width();

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Password");
        imgui_table_next_column();
        imgui_push_item_width(-1);
        push_input_styles();
        imgui_input_password("##password", password, sizeof(password));
        pop_input_styles();
        imgui_pop_item_width();

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Description");
        imgui_table_next_column();
        push_input_styles();
        imgui_input_text_area("##description", pdata.desc, sizeof(pdata.desc), -1, 80);
        pop_input_styles();

        imgui_begin_disabled(okey.usnr > 0);
        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Class");
        imgui_table_next_column();
        shared_dropdown("##class", classes, 3, &current_class);

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Gender");
        imgui_table_next_column();
        shared_dropdown("##gender", genders, 2, &current_gender);
        imgui_end_disabled();

        imgui_end_table();
    }


    push_previous_characters_styles();
    if (imgui_begin_child("previous_characters", -1, 200, true,
                          IMGUI_WINDOW_FLAG_NO_SCROLLBAR | IMGUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE)) {
        push_sub_header_styles();
        imgui_center_next_text("Previous Characters");
        imgui_text("Previous Characters");
        pop_sub_header_styles();
        if (imgui_begin_child("previous_character_list", -1, -1, false, IMGUI_WINDOW_FLAG_ALWAYS_VERTICAL_SCROLLBAR)) {
            for (int i = 0; i < 6; i++) {
                push_button_styles();
                imgui_center_next_item(240);
                imgui_button_sized("WorldofBiting", 240, 24);
                pop_button_styles();
            }
        }
        imgui_end_child();
    }
    imgui_end_child();
    pop_previous_characters_styles();
}

static void right_column() {
    push_header_styles();
    imgui_center_next_text("Options");
    imgui_text("Options");
    pop_header_styles();

    imgui_center_next_item(150 + 240);
    if (imgui_begin_table("Options", 2,
                          IMGUI_TABLE_FLAG_BORDERS | IMGUI_TABLE_FLAG_SIZING_FIXED_FIT |
                          IMGUI_TABLE_FLAG_NO_HOST_EXTEND_X)) {
        imgui_table_setup_column("label", IMGUI_TABLE_COLUMN_FLAG_WIDTH_FIXED, 150, 0);
        imgui_table_setup_column("input", IMGUI_TABLE_COLUMN_FLAG_WIDTH_FIXED, 240, 1);

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Alpha");
        imgui_table_next_column();
        shared_dropdown("##alphas", alphas, 3, &do_alpha);

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Window Type");
        imgui_table_next_column();
        shared_dropdown("##window_modes", window_modes, 2, &g_config.video.windowed);

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Shadows");
        imgui_table_next_column();
        push_checkbox_styles();
        imgui_checkbox("##shadow", &do_shadow);
        pop_checkbox_styles();

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Sounds Enabled?");
        imgui_table_next_column();
        push_checkbox_styles();
        imgui_checkbox("##do_sound", &g_config.audio.sound_enabled);
        pop_checkbox_styles();

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Volume Level");
        imgui_table_next_column();
        push_slider_styles();
        imgui_slider_int("##volume_level", &g_config.audio.sound_volume, 0, 10);
        pop_slider_styles();

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Dark GUI");
        imgui_table_next_column();
        push_checkbox_styles();
        imgui_checkbox("##dark_ui", &do_darkmode);
        pop_checkbox_styles();

        imgui_end_table();
    }
}

void launcher_init() {
    set_race_gender();
}

static void load_file() {
    int file = open(switch_confirm.file_path, O_RDONLY | O_BINARY);
    int flag = 0;
    if (file == -1) {
        log_error("Could not open file %s", switch_confirm.file_path);
        snprintf(simple_popover_text, sizeof(simple_popover_text), "Could not open file %s", switch_confirm.file_path);
        imgui_open_popup("FYI");
        return;
    }

    if (lseek(file, 0, SEEK_END) > (long) (sizeof(struct pdata) + sizeof(struct key))) flag = 1;
    lseek(file, 0, SEEK_SET);
    read(file, &okey, sizeof(struct key));
    if (read(file, &pdata, sizeof(struct pdata)) != sizeof(struct pdata) || flag) {
        pdata.hide = 1;
        pdata.show_names = 1;
        pdata.show_proz = 1;
        pdata.show_stats = 0;
        pdata.show_bars = 0;
        pdata.cname[0] = 0;
        pdata.ref[0] = 0;
        pdata.desc[0] = 0;
        pdata.changed = 0;

        for (int i = 0; i < 20; i++) {
            pdata.xbutton[i].skill_nr = -1;
            strcpy(pdata.xbutton[i].name, "-");
        }
    }

    close(file);
    pdata.changed = 1;
    set_race_gender();
}

static void save_file(const char *file_path) {
    int file = open(file_path, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, 0600);
    if (file == -1) {
        log_error("Could not save file %s", file_path);
        snprintf(simple_popover_text, sizeof(simple_popover_text), "Could not save file %s", file_path);
        imgui_open_popup("FYI");
        return;
    }

    write(file, &okey, sizeof(struct key));
    write(file, &pdata, sizeof(struct pdata));
    close(file);

    log_info("Saved file as %s", file_path);
    snprintf(simple_popover_text, sizeof(simple_popover_text), "Saved file as %s", file_path);
    imgui_open_popup("FYI");
}

static void load() {
    char const *file_path = tinyfd_openFileDialog(
        "Select your MOA",
        "",
        1,
        filterPatterns, "Astonia Save File (.MOA)", 0);

    if (file_path != NULL) {
        log_info("Loading MOA from %s", file_path);
        start_load_switch_character_confirmation(file_path);
    }
}

static void save() {
    char const *file_path = tinyfd_saveFileDialog(
        "Save your character",
        "",
        1,
        filterPatterns, "Astonia Save File (.MOA)");

    if (file_path != NULL) {
        log_info("Saving MOA to %s", file_path);
        save_file(file_path);
    }
}

static void new_character() {
    okey.usnr = 0;
    okey.race = 0;
    okey.pass1 = 0;
    okey.pass2 = 0;
    pdata.changed = 1;
    strcpy(okey.name, "New Account");
    pdata.cname[0] = 0;
    set_race_gender();
}

static void render_popovers() {
    if (switch_confirm.show_first_confirmation) {
        imgui_open_popup("Are you sure?");
        switch_confirm.show_first_confirmation = false;
    }

    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_WINDOW_PADDING, 4, 4);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_FRAME_PADDING, 4, 4);
    imgui_push_style_var_float(IMGUI_STYLE_VAR_WINDOW_BORDER_SIZE, 2);
    imgui_push_style_color_32(IMGUI_COL_POPUP_BG, INNER_WINDOW_32);
    imgui_push_style_color_32(IMGUI_COL_TITLE_BG_ACTIVE, BORDERS_32);
    imgui_push_style_color_32(IMGUI_COL_BORDER, BORDERS_32);
    imgui_set_next_window_pos(UI_CENTER_X - 200, UI_CENTER_Y - 100);
    imgui_set_next_windows_size(400, 200);
    if (imgui_begin_popup_modal("Are you sure?", NULL,
                                IMGUI_WINDOW_FLAG_NO_MOVE | IMGUI_WINDOW_FLAG_NO_RESIZE |
                                IMGUI_WINDOW_FLAG_NO_SAVED_SETTINGS)) {
        // imgui_set_cursor_pos_y(imgui_get_cursor_pos_y() + 25);
        imgui_center_next_item(350);
        if (imgui_begin_child("text_container", 350, 100, false, IMGUI_WINDOW_FLAG_NO_SCROLLBAR)) {
            if (switch_confirm.new_character) {
                imgui_center_next_text("Do you really want to create a new account?");
                imgui_text("Do you really want to create a new account?");
            } else {
                imgui_center_next_text("Do you really want to switch accounts?");
                imgui_text("Do you really want to switch accounts?");
            }
            imgui_spacing();
            imgui_text_wrapped_colored_32(BEIGE_COLOR_32,
                                          "Your old account will no longer be accessible, unless you remembered to save it.");
            imgui_spacing();
            imgui_center_next_text("You did save it, didn't you");
            imgui_text_colored_32(HEADER_TEXT_COLOR_32, "You did save it, didn't you");
            imgui_end_child();
        }

        imgui_spacing();
        imgui_separator();
        imgui_spacing();

        imgui_center_next_item(250);
        push_button_styles();
        if (imgui_button_sized("Yes", 120, 0)) {
            imgui_close_current_popup();
            switch_confirm.show_second_confirmation = true;
        }

        imgui_same_line(0, 10);

        if (imgui_button_sized("Cancel", 120, 0)) {
            imgui_close_current_popup();
            switch_confirm.file_path[0] = '\0';
            switch_confirm.new_character = false;
        }
        pop_button_styles();

        imgui_end_popup();
    }

    if (switch_confirm.show_second_confirmation) {
        imgui_open_popup("Are you really sure?");
        switch_confirm.show_second_confirmation = false;
    }

    imgui_set_next_window_pos(UI_CENTER_X - 200, UI_CENTER_Y - 100);
    imgui_set_next_windows_size(400, 200);
    if (imgui_begin_popup_modal("Are you really sure?", NULL,
                                IMGUI_WINDOW_FLAG_NO_MOVE | IMGUI_WINDOW_FLAG_NO_RESIZE |
                                IMGUI_WINDOW_FLAG_NO_SAVED_SETTINGS)) {
        imgui_center_next_item(350);
        if (imgui_begin_child("text_container", 350, 100, false, IMGUI_WINDOW_FLAG_NO_SCROLLBAR)) {
            imgui_center_next_text("No Really!");
            imgui_text_colored(1.0f, 0.5f, 0.0f, 1.0f, "No Really!");
            imgui_spacing();

            imgui_text_wrapped("This will replace your current character.");
            imgui_center_next_text("This action cannot be undone.");
            imgui_text("This action cannot be undone.");
            imgui_end_child();
        }
        imgui_spacing();
        imgui_separator();
        imgui_spacing();
        imgui_center_next_item(250);
        push_button_styles();
        if (imgui_button_sized("Yes", 120, 0)) {
            imgui_close_current_popup();
            switch_confirm.show_second_confirmation = false;
            if (switch_confirm.new_character) {
                new_character();
            } else {
                load_file();
            }
            switch_confirm.file_path[0] = '\0';
            switch_confirm.new_character = false;
        }

        imgui_same_line(0, 10);

        if (imgui_button_sized("Cancel", 120, 0)) {
            imgui_close_current_popup();
            switch_confirm.file_path[0] = '\0';
            switch_confirm.new_character = false;
        }
        pop_button_styles();

        imgui_end_popup();
    }

    imgui_set_next_window_pos(UI_CENTER_X - 200, UI_CENTER_Y - 100);
    imgui_set_next_windows_size(400, 200);
    if (imgui_begin_popup_modal("FYI", NULL, IMGUI_WINDOW_FLAG_NO_MOVE | IMGUI_WINDOW_FLAG_NO_RESIZE |
                                             IMGUI_WINDOW_FLAG_NO_SAVED_SETTINGS)) {
        imgui_center_next_item(350);
        if (imgui_begin_child("text_container", 350, 100, false, IMGUI_WINDOW_FLAG_NO_SCROLLBAR)) {
            imgui_text_wrapped(simple_popover_text);
            imgui_end_child();
        }
        imgui_spacing();
        imgui_separator();
        imgui_spacing();

        imgui_center_next_item(120);
        push_button_styles();
        if (imgui_button_sized("Ok", 120, 0)) {
            imgui_close_current_popup();
        }
        pop_button_styles();

        imgui_end_popup();
    }
    imgui_pop_style_color(3);
    imgui_pop_style_var(3);
}

void launcher_render() {
    imgui_set_next_window_pos(0, 0);
    imgui_set_next_windows_size(SCREEN_WIDTH, SCREEN_HEIGHT);
    sdl_load_sprite(LAUNCHER_BACKGROUND_SPRITE);
    const SpriteData *background_sprite_data = &sprite_data[LAUNCHER_BACKGROUND_SPRITE];
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_WINDOW_PADDING, 110, 72);
    push_launcher_styles();
    if (imgui_begin("Launcher", NULL, window_flags)) {
        void *draw_list = imgui_get_window_draw_list();

        imgui_draw_list_add_image(draw_list, background_sprite_data->gl_texture, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                                  background_sprite_data->uv0.u, background_sprite_data->uv0.v,
                                  background_sprite_data->uv1.u, background_sprite_data->uv1.v,
                                  imgui_color_convert_float4_to_u32(1, 1, 1, 1));


        render_popovers();

        if (imgui_begin_table("LauncherLayout", 2, IMGUI_TABLE_FLAG_SIZING_STRETCH_SAME | IMGUI_TABLE_FLAG_BORDERS)) {
            imgui_table_next_row(0, 40);
            imgui_table_next_column();
            left_column();
            imgui_table_next_column();
            right_column();
            imgui_begin_child("filler", 0, -1, false, 0);
            imgui_end_child();
            imgui_end_table();
        }

        imgui_center_next_item(640);
        push_button_styles();
        imgui_button_sized("Start", 120, 30);
        imgui_same_line(0, 10);
        if (imgui_button_sized("Load", 120, 30)) {
            load();
        }
        imgui_same_line(0, 10);
        if (imgui_button_sized("Save", 120, 30)) {
            save();
        }
        imgui_same_line(0, 10);
        if (imgui_button_sized("Discord", 120, 30)) {
            SDL_OpenURL(MNEWS);
        }
        imgui_same_line(0, 10);
        if (imgui_button_sized("Exit", 120, 30)) {
            quit = 1;
        }
        pop_button_styles();
    }
    pop_launcher_styles();
    imgui_pop_style_var(1);
    imgui_end();
}
