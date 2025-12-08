#include "widgets.h"

#include "imgui_wrapper.h"
#include "ui_common.h"
#include "graphics/sdl.h"

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
        (void*)(intptr_t)button_texture,
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
    const char* button_text = label;
    float text_width, text_height;
    imgui_calc_text_size_simple(&text_width, &text_height, button_text);

    float text_x = min_x + ((max_x - min_x) - text_width) / 2.0f;
    float text_y = min_y + ((max_y - min_y) - text_height) / 2.0f;

    imgui_draw_list_add_text(draw_list, text_x, text_y, text_color, button_text);

    return clicked;
}

bool tab_button(const char* label, bool is_active, float width) {
    void* draw_list = imgui_get_window_draw_list();

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
        (void*)(intptr_t)button_texture,
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
    unsigned int text_color = is_active ?
        imgui_color_convert_float4_to_u32(BLUE_FONT_COLOR[0], BLUE_FONT_COLOR[1], BLUE_FONT_COLOR[2], 1.0f) :
        imgui_color_convert_float4_to_u32(GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);

    imgui_draw_list_add_text(draw_list, text_x, text_y, text_color, label);

    return clicked;
}

bool keybind(const char *keybind_label) {
    /* Calculate total width of text + button to center them together */
    float text_width, text_height;
    imgui_calc_text_size_simple(&text_width, &text_height, keybind_label);
    float spacing_x, spacing_y;
    imgui_get_style_item_spacing(&spacing_x, &spacing_y);

    float button_width = 50.0f;
    float button_height = 20.0f;
    float total_width = text_width + spacing_x + button_width;

    /* Center the text+button pair horizontally */
    imgui_center_next_item(total_width);

    /* Save the starting Y position */
    float start_y = imgui_get_cursor_pos_y();

    /* Calculate vertical offset to center text with button */
    float text_offset = (button_height - text_height) / 2.0f;

    /* Render text vertically centered with button */
    imgui_push_style_color(IMGUI_COL_TEXT, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1.0f);
    imgui_set_cursor_pos_y(start_y + text_offset);
    imgui_text(keybind_label);

    /* Position button at the starting Y (buttons are typically taller) */
    imgui_same_line_gap();
    imgui_set_cursor_pos_y(start_y);
    ui_button("Ctrl+1", button_width, button_height);
    imgui_pop_style_color(1);
}