#include "launcher.h"

#include <stddef.h>

#include "graphics/sdl.h"
#include "ui/ui.h"
#include "ui/imgui/imgui_wrapper.h"

static const int window_flags =
        IMGUI_WINDOW_FLAG_NO_COLLAPSE | IMGUI_WINDOW_FLAG_NO_RESIZE | IMGUI_WINDOW_FLAG_NO_MOVE |
        IMGUI_WINDOW_FLAG_NO_TITLE_BAR | IMGUI_WINDOW_FLAG_NO_SCROLLBAR | IMGUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE;

static const char *classes[] = {
    "Templar",
    "Mercenary",
    "Harakim"
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
    "Windowed",
    "Fullscreen"
};

static int current_class = 0;
static int current_gender = 0;
static int current_alpha = 0;
static int current_window_mode = 0;

static bool shadows = 0;
static bool do_sound = 0;
static int volume_level = 0;
static bool dark_ui = 0;

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
    static char name_buffer[32];
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
        imgui_text("WorldOfSlaying");
        imgui_table_next_column();
        push_button_styles();
        imgui_button("Create New Account");
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
        imgui_input_text("##name", name_buffer, sizeof(name_buffer));
        pop_input_styles();
        imgui_pop_item_width();

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Password");
        imgui_table_next_column();
        imgui_push_item_width(-1);
        push_input_styles();
        imgui_input_password("##password", name_buffer, sizeof(name_buffer));
        pop_input_styles();
        imgui_pop_item_width();

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Description");
        imgui_table_next_column();
        push_input_styles();
        imgui_input_text_area("##description", name_buffer, sizeof(name_buffer), -1, 80);
        pop_input_styles();

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
        shared_dropdown("##alphas", alphas, 3, &current_alpha);

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Window Type");
        imgui_table_next_column();
        shared_dropdown("##window_modes", window_modes, 2, &current_window_mode);

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Shadows");
        imgui_table_next_column();
        push_checkbox_styles();
        imgui_checkbox("##shadow", &shadows);
        pop_checkbox_styles();

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Sounds Enabled?");
        imgui_table_next_column();
        push_checkbox_styles();
        imgui_checkbox("##do_sound", &do_sound);
        pop_checkbox_styles();

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Volume Level");
        imgui_table_next_column();
        push_slider_styles();
        imgui_slider_int("##volume_level", &volume_level, 0, 10);
        pop_slider_styles();

        imgui_table_next_row(0, 20);
        imgui_table_next_column();
        imgui_text("Dark GUI");
        imgui_table_next_column();
        push_checkbox_styles();
        imgui_checkbox("##dark_ui", &dark_ui);
        pop_checkbox_styles();

        imgui_end_table();
    }
}

void launcher_render(void) {
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
        imgui_button_sized("Load", 120, 30);
        imgui_same_line(0, 10);
        imgui_button_sized("Save", 120, 30);
        imgui_same_line(0, 10);
        imgui_button_sized("Discord", 120, 30);
        imgui_same_line(0, 10);
        imgui_button_sized("Exit", 120, 30);
        pop_button_styles();
    }
    pop_launcher_styles();
    imgui_pop_style_var(1);
    imgui_end();
}
