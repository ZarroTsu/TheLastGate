#include "spell_hud.h"

#include "imgui/imgui_wrapper.h"
#include "inter.h"
#include "config/keybindings.h"
#include "main.h"
#include "ui_common.h"
#include "common.h"
#include "ui.h"
#include "config/config.h"

static int current_slot = 0;

static const int usable_skills[26] = {SK_BLIND, SK_CLEAVE, SK_SHIELD, SK_LEAP, SK_RAGE, SK_TAUNT, SK_WEAKEN, SK_WARCRY, SK_BLAST, SK_BLESS, SK_CURSE, SK_DISPEL, SK_ENHANCE, SK_GHOST, SK_HASTE, SK_HEAL, SK_IDENT, SK_LETHARGY, SK_LIGHT, SK_MSHIELD, SK_POISON, SK_PROTECT, SK_PULSE, SK_RECALL, SK_SHADOW, SK_SLOW};

static void get_skill_tab_info_from_id(const int skill_id, char *out_name, int *out_skill_tab_id) {
    int pl_flags = pl.worn[WN_FLAGS];
    int pl_flagb = pl.worn_p[WN_FLAGS];
    int skill_tab_id = -1;
    for (int i = 0; i < 55; i++) {
        if (skilltab[i].nr == skill_id) {
            skill_tab_id = i;
            break;
        }
    }
    if (skill_tab_id == -1) {
        *out_skill_tab_id = -1;
        snprintf(out_name, sizeof(out_name), "Invalid");
        return;
    }
    if ((skill_id == 11 && (pl_flagb & (1 << 10))) || // Magic Shield -> Magic Shell
        (skill_id == 19 && (pl_flags & (1 << 5))) || // Slow -> Greater Slow
        (skill_id == 20 && (pl_flags & (1 << 6))) || // Curse -> Greater Curse
        (skill_id == 24 && (pl_flags & (1 << 7))) || // Blast -> +Scorch
        (skill_id == 26 && (pl_flags & (1 << 14))) || // Heal -> Regen
        (skill_id == 37 && (pl_flagb & (1 << 11))) || // Blind -> Douse
        (skill_id == 40 && (pl_flags & (1 << 8))) || // Cleave -> +Aggravate
        (skill_id == 41 && (pl_flags & (1 << 10))) || // Weaken -> Greater Weaken
        (skill_id == 16 && (pl_flagb & (1 << 5))) || // Shield -> Shield Bash
        (skill_id == 43 && (pl_flagb & (1 << 6))) || // Pulse -> Healing Pulses
        (skill_id == 49 && (pl_flagb & (1 << 7))) || // Leap
        (skill_id == 35 && (pl_flagb & (1 << 12))) || // Warcry -> Rally
        (skill_id == 42 && (pl_flagb & (1 << 14))) || // Poison -> Venom
        (skill_id == 12 && (pl_flagb & (1 << 3))) || // Tactics invert
        (skill_id == 22 && IS_SHIFTED) // Rage -> Calm
    )
        snprintf(out_name, sizeof(out_name), "%s", skilltab[skill_tab_id].alt_a);
    else
        snprintf(out_name, sizeof(out_name), "%s", skilltab[skill_tab_id].name);

    *out_skill_tab_id = skill_tab_id;
}

static void get_keybind_text_for_current_slot(int slot, char *out, int size) {
    char binding_id[32];
    sprintf(binding_id, "spell_%d", slot + 1);
    BindingDescriptor *binding = binding_find_by_id(binding_id);
    if (binding) {
        keybinding_to_short_string(binding->keybinding, out, size);
    } else {
        sprintf(out, "UNK");
    }
}

static void handle_spell_selection(int skill_tab_id) {
    char keybind_text[32];
    get_keybind_text_for_current_slot(current_slot, keybind_text, sizeof(keybind_text));
    if (pdata.xbutton[current_slot].skill_nr != skilltab[skill_tab_id].nr) {
        pdata.xbutton[current_slot].skill_nr = skilltab[skill_tab_id].nr;
        snprintf(pdata.xbutton[current_slot].name, 7, "%s", skilltab[skill_tab_id].name);
        xlog(1,"%s is now %s.", keybind_text, skilltab[skill_tab_id].name);
    } else {
        pdata.xbutton[current_slot].skill_nr = -1;
        xlog(1,"%s is now unassigned.", keybind_text);
    }
}

static void render_spell_popover() {
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_WINDOW_PADDING, 8.0f, 4.0f);
    if (imgui_begin_popup("SpellPopup1")) {
        if (imgui_selectable("-", 0)) {
            pdata.xbutton[current_slot].skill_nr = -1;
        }
        for (int i = 0; i < 26; i++) {
            int skill_id = usable_skills[i];
            int skill_tab_id;
            const char *skill_name[32];
            if ((skill_id==52 && !KNOW_IDENTIFY) || ((skill_id==53||skill_id==54) && !IS_LYCANTH)) {
                continue;
            }
            get_skill_tab_info_from_id(skill_id, skill_name, &skill_tab_id);
            if (skill_tab_id == -1) continue;
            if (pl.skill[skill_tab_id][0] == 0) continue;
            if (imgui_selectable(skill_name, 0)) {
                handle_spell_selection(skill_tab_id);
            }
        }
        imgui_end_popup();
    }
    imgui_pop_style_var(1);
}



void spell_hud() {
    imgui_set_next_window_pos(1036, 597);
    imgui_set_next_windows_size(238, 58);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_WINDOW_PADDING, 0.0f, 0.0f);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_ITEM_SPACING, 0.0f, 0.0f);
    imgui_push_style_var_vec2(IMGUI_STYLE_VAR_FRAME_PADDING, 0.0f, 2.0f);
    imgui_push_style_var_float(IMGUI_STYLE_VAR_WINDOW_BORDER_SIZE, 0.0f);
    imgui_push_font(font_sizes.ui);
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
                0xFF050512,
                0.0f,
                0);
            char binding_text[4];
            char spell_text[8];
            char spell_key_id[32];

            sprintf(spell_key_id, "##SpellKey%d", i);
            get_keybind_text_for_current_slot(i, binding_text, sizeof(binding_text));
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

            if (imgui_is_item_hovered() && imgui_is_mouse_clicked(1)) {
                current_slot = i;
                if (last_skill == -1) {
                    imgui_open_popup("SpellPopup1");
                } else {
                    handle_spell_selection(last_skill);
                }
            }
        }
        render_spell_popover();
        imgui_pop_style_color(4);
    }
    imgui_pop_font();
    imgui_end();
    imgui_pop_style_var(4);
}
