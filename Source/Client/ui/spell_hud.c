#include "spell_hud.h"

#include "imgui_wrapper.h"
#include "inter.h"
#include "keybindings.h"
#include "main.h"
#include "ui_common.h"
#include "graphics/sdl.h"

static void handle_spell_selection(int current_slot, int skill_tab_id) {
    char keybind_text[32];
    keybinding_to_string(keybind_config.spell_hotkeys[current_slot], keybind_text, sizeof(keybind_text));
    if (pdata.xbutton[current_slot].skill_nr != skilltab[skill_tab_id].nr) {
        pdata.xbutton[current_slot].skill_nr = skilltab[skill_tab_id].nr;
        snprintf(pdata.xbutton[current_slot].name, 7, "%s", skilltab[skill_tab_id].name);
        xlog(1,"%s is now %s.", keybind_text, skilltab[skill_tab_id].name);
    } else {
        pdata.xbutton[current_slot].skill_nr = -1;
        xlog(1,"%s is now unassigned.", keybind_text);
    }
}

void spell_hud() {
    static int current_slot = 0;


    imgui_set_next_window_pos(1036, 597);
    imgui_set_next_windows_size(238, 58);
    // imgui_set_font_global_scale(scale_y);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_WINDOW_PADDING, 0.0f, 0.0f);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_ITEM_SPACING, 0.0f, 0.0f);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_FRAME_PADDING, 0.0f, 2.0f);
    imgui_push_style_var_float(IMGUI_STYLE_VAR_WINDOW_BORDER_SIZE, 0.0f);
    if (imgui_begin("##SPELLHUD", NULL, IMGUI_WINDOW_FLAG_NO_MOVE | IMGUI_WINDOW_FLAG_NO_COLLAPSE | IMGUI_WINDOW_FLAG_NO_RESIZE | IMGUI_WINDOW_FLAG_NO_BACKGROUND | IMGUI_WINDOW_FLAG_NO_TITLE_BAR)) {
        void *draw_list = imgui_get_window_draw_list();
        float window_x = imgui_get_window_pos_x();
        float window_y = imgui_get_window_pos_y();
        imgui_push_style_color(IMGUI_COL_TEXT, GOLD_FONT_COLOR[0], GOLD_FONT_COLOR[1], GOLD_FONT_COLOR[2], 1);
        imgui_push_style_color(IMGUI_COL_BUTTON, 1,1,1,0);
        imgui_push_style_color(IMGUI_COL_BUTTON_HOVERED, 1,1,1,0);
        imgui_push_style_color(IMGUI_COL_BUTTON_ACTIVE, 1,1,1,0);
        for (int i = 0; i < 20; i++) {
            if (i > 0 && i % 5 != 0) {
                imgui_same_line_gap();
                imgui_set_cursor_pos_x(imgui_get_cursor_pos_x() + 2.0f);
            }

            if (i > 0 && i % 5 == 0) {
                imgui_dummy(0, 2.0f);
            }
            float x = window_x + imgui_get_cursor_pos_x();
            float y = window_y + imgui_get_cursor_pos_y();
            imgui_draw_list_add_rect_filled(
                draw_list,
                x, y, x + 46, y + 13,
                0xFF120505,
                0.0f,
                0);
            char binding_text[4];
            char spell_text[8];
            char spell_key_id[32];

            sprintf(spell_key_id, "##SpellKey%d", i);
            keybinding_to_short_string(keybind_config.spell_hotkeys[i], binding_text, sizeof(binding_text));
            if (pdata.xbutton[i].skill_nr != -1) {
                sprintf(spell_text, "%s", pdata.xbutton[i].name);
            } else {
                sprintf(spell_text, "-");
            }

            float keybind_width, keybind_height;
            imgui_calc_text_size_simple(&keybind_width, &keybind_height, binding_text);

            /* Center the keybind text within the button */
            float text_x = x + (46 - keybind_width);
            float text_y = y + (13 - keybind_height) / 2.0f;

            imgui_draw_list_add_text(draw_list, text_x - 2, text_y + 1, HINT_GREY_FONT_COLOR_32, binding_text);
            imgui_draw_list_add_text(draw_list, x + 2, text_y + 1, GOLD_FONT_COLOR_32, spell_text);

            if (imgui_invisible_button(spell_key_id, 46, 13)) {
                button_command(16 + i);
            }
            // if (imgui_button_sized(buf, 46, 13)) {
            // }

            if (imgui_is_item_hovered() && imgui_is_mouse_clicked(1)) {
                current_slot = i;
                if (last_skill == -1) {
                    imgui_open_popup("SpellPopup1");
                } else {
                    handle_spell_selection(current_slot, last_skill);
                }
            }
            // imgui_dummy(46, 13);
        }
        if (imgui_begin_popup("SpellPopup1")) {
            if (imgui_selectable("-", 0)) {
                pdata.xbutton[current_slot].skill_nr = -1;
            }
            for (int i = 0; i < 55; i++) {
                if (skilltab[i].nr > 0) {
                    if (imgui_selectable(skilltab[i].name, 0)) {
                        handle_spell_selection(current_slot, i);
                    }
                }
            }
            imgui_end_popup();
        }
        imgui_pop_style_color(4);
    }
    imgui_end();
    imgui_pop_style_var(4);
}
