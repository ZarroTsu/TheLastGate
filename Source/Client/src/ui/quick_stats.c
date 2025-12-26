#include "quick_stats.h"

#include <stddef.h>
#include <stdio.h>

#include "main.h"
#include "ui_common.h"
#include "config/config.h"
#include "graphics/sdl.h"
#include "imgui/imgui_wrapper.h"
#include "util/math_util.h"

typedef struct {
    Vec2 window_position;
    Vec2 window_size;
    Vec2 window_size_expanded;
    Vec2 window_padding;
    float column_label_width;
    float column_value_width;
    float row_normal_height;
    float row_gap_height;
    float row_progress_border_height;
    float row_experience_height;
    float experience_bar_width;
    int window_background_sprite;
    int window_background_sprite_expanded;
} QuickStatsLayout;

static const QuickStatsLayout LAYOUT_LIGHT = {
    .window_position = {1106, 256},
    .window_size = {169, 51},
    .window_size_expanded = {169, 79},
    .window_padding = {2, 2},
    .column_label_width = 81,
    .column_value_width = 75,
    .row_normal_height = 12,
    .row_gap_height = 2,
    .row_progress_border_height = 1,
    .row_experience_height = 6,
    .experience_bar_width = 165,
    .window_background_sprite = 3,
    .window_background_sprite_expanded = 2
};

static const QuickStatsLayout LAYOUT_DARK = {
    .window_position = {1106, 256},
    .window_size = {171, 53},
    .window_size_expanded = {171, 81},
    .window_padding = {3, 3},
    .column_label_width = 79,
    .column_value_width = 75,
    .row_normal_height = 12,
    .row_gap_height = 2,
    .row_progress_border_height = 1,
    .row_experience_height = 6,
    .experience_bar_width = 165,
    .window_background_sprite = 3,
    .window_background_sprite_expanded = 2
};

static const QuickStatsLayout* get_layout(void) {
    if (do_darkmode) return &LAYOUT_DARK;
    return &LAYOUT_LIGHT;
}

static void push_inventory_styles(const QuickStatsLayout *layout) {
    imgui_push_style_var_float(IMGUI_STYLE_VAR_WINDOW_BORDER_SIZE, 0);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_ITEM_SPACING, 0, 0);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_ITEM_INNER_SPACING, 0, 0);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_WINDOW_PADDING, layout->window_padding.x, layout->window_padding.y);
    imgui_push_style_color_32(IMGUI_COL_TEXT, GOLD_FONT_COLOR_32);
}

static void pop_inventory_styles() {
    imgui_pop_style_var(4);
    imgui_pop_style_color(1);
}

static void gap_row_render(const QuickStatsLayout *layout) {
    imgui_table_next_row(0, layout->row_gap_height);
    imgui_table_next_column();
}

static void value_row_render(const QuickStatsLayout *layout, const char *label, const char *value) {
    imgui_table_next_row(0, layout->row_normal_height);
    const float cursor_y = imgui_get_cursor_pos_y() + (layout->row_normal_height - imgui_get_text_line_height());
    float w;
    float tw;
    imgui_table_next_column();
    imgui_set_cursor_pos_y(cursor_y);
    imgui_text(label);
    imgui_table_next_column();
    imgui_set_cursor_pos_y(cursor_y);
    imgui_get_content_region_avail(&w, NULL);
    imgui_calc_text_size_simple(&tw, NULL, value);
    imgui_set_cursor_pos_x(imgui_get_cursor_pos_x() + w - tw);
    imgui_text(value);
}

static void bar_row_render(const QuickStatsLayout *layout, const float progress) {
    imgui_table_next_row(0, layout->row_experience_height);
    imgui_table_next_column();
    void *draw_list = imgui_get_window_draw_list();
    float win_x = imgui_get_window_pos_x();
    float win_y = imgui_get_window_pos_y();
    float x = win_x + imgui_get_cursor_pos_x() - layout->window_padding.x;
    float y = win_y + imgui_get_cursor_pos_y();
    float height = layout->row_experience_height;
    float width = layout->experience_bar_width;
    float progress_width = width * progress;


    imgui_draw_list_add_rect_filled(draw_list, x, y, x + width, y + height, 0xFF842d00, 0, 0);
    imgui_draw_list_add_rect_filled(draw_list, x, y, x + progress_width, y + height, 0xFF0061BD, 0, 0);
}

static void draw_sprite_imgui(void *draw_list, const SpriteData *data, float x, float y) {
    if (!data->loaded_in_atlas) return;

    /* Draw sprite_data using ImGui */
    void *texture = (void *) (uintptr_t) data->atlas_texture;
    imgui_draw_list_add_image(draw_list, texture,
                              x, y,
                              x + data->pixel_width, y + data->pixel_height,
                              data->uv0.u, data->uv0.v,
                              data->uv1.u, data->uv1.v,
                              0xFFFFFFFF);
}

static void draw_quick_stat_background(const QuickStatsLayout *layout) {
    void *draw_list = imgui_get_window_draw_list();
    SpriteData *data = sdl_get_ui_sprite_data(g_config.ui.expanded_quick_stats
                                                  ? layout->window_background_sprite_expanded
                                                  : layout->window_background_sprite);
    draw_sprite_imgui(draw_list, data, layout->window_position.x, layout->window_position.y);
}

typedef struct {
    float window_width;
    float window_height;
    float inner_width;
    float inner_height;
} QuickStatWindowSize;

QuickStatWindowSize calculate_window_sizes(const QuickStatsLayout *layout) {
    QuickStatWindowSize size = {0};
    if (g_config.ui.expanded_quick_stats) {
        size.window_width = layout->window_size_expanded.x;
        size.window_height = layout->window_size_expanded.y;
        size.inner_width = layout->window_size_expanded.x - layout->window_padding.x * 2;
        size.inner_height = layout->window_size_expanded.y - layout->window_padding.y * 2;
    } else {
        size.window_width = layout->window_size.x;
        size.window_height = layout->window_size.y;
        size.inner_width = layout->window_size.x - layout->window_padding.x * 2;
        size.inner_height = layout->window_size.y - layout->window_padding.y * 2;
    }
    return size;
}

void quick_stats_render(QuickStatState *state) {
    const QuickStatsLayout *layout = get_layout();
    char value[32];
    imgui_set_next_window_pos(layout->window_position.x, layout->window_position.y);
    const QuickStatWindowSize window_sizes = calculate_window_sizes(layout);
    imgui_set_next_windows_size(window_sizes.window_width, window_sizes.window_height);
    push_inventory_styles(layout);
    if (imgui_begin("Quick Stats", NULL,
                    IMGUI_WINDOW_FLAG_NO_RESIZE | IMGUI_WINDOW_FLAG_NO_MOVE | IMGUI_WINDOW_FLAG_NO_TITLE_BAR |
                    IMGUI_WINDOW_FLAG_NO_BACKGROUND)) {
        draw_quick_stat_background(layout);
        imgui_push_style_var_vec2(IMGUI_STYLE_VAR_WINDOW_PADDING, 0, 0);
        if (imgui_begin_child("Quick Stats Inner", window_sizes.inner_width, window_sizes.inner_height, false, 0)) {
            imgui_push_style_var_vec2(IMGUI_STYLE_VAR_CELL_PADDING, layout->window_padding.x, 0);
            imgui_push_style_var_vec2(IMGUI_STYLE_VAR_ITEM_SPACING, 0, 0);
            if (imgui_begin_table("Quick Stats Table", 2,
                                  IMGUI_TABLE_FLAG_SIZING_FIXED_FIT | IMGUI_TABLE_FLAG_PAD_OUTER_X |
                                  IMGUI_TABLE_FLAG_NO_HOST_EXTEND_X)) {
                imgui_table_setup_column("Label", IMGUI_TABLE_COLUMN_FLAG_WIDTH_FIXED, layout->column_label_width, 0);
                imgui_table_setup_column("Value", IMGUI_TABLE_COLUMN_FLAG_WIDTH_FIXED, layout->column_value_width, 1);

                snprintf(value, sizeof(value), "%d", state->weapon_value);
                value_row_render(layout, "Weapon Value", value);

                gap_row_render(layout);

                snprintf(value, sizeof(value), "%d", state->armor_value);
                value_row_render(layout, "Armor Value", value);

                gap_row_render(layout);

                if (g_config.ui.expanded_quick_stats) {
                    snprintf(value, sizeof(value), "%d", state->hit_score);
                    value_row_render(layout, "Hit Score", value);

                    gap_row_render(layout);

                    snprintf(value, sizeof(value), "%d", state->parry_score);
                    value_row_render(layout, "Parry Score", value);

                    gap_row_render(layout);
                }

                snprintf(value, sizeof(value), "%d", state->experience);
                value_row_render(layout, "Experience", value);

                imgui_table_next_row(0, layout->row_progress_border_height);
                imgui_table_next_column();

                bar_row_render(layout, state->experience_bar_progress);

                imgui_end_table();
            }
            imgui_pop_style_var(2);
            imgui_end_child();
            imgui_pop_style_var(1);
        }
    }
    imgui_end();
    pop_inventory_styles();
}
