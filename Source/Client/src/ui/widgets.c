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
