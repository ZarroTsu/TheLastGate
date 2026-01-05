#include "hotbar.hpp"

#include <imgui.h>
#include <stdio.h>

#include "common.h"
#include "inter.h"
#include "ui.h"
#include "ui_common.hpp"
#include "config/keybindings.h"
#include "ui_utils/imgui_style_builder.hpp"

static int current_slot = 0;

static const int usable_skills[26] = {
    SK_BLIND, SK_CLEAVE, SK_SHIELD, SK_LEAP, SK_RAGE, SK_TAUNT, SK_WEAKEN, SK_WARCRY, SK_BLAST, SK_BLESS, SK_CURSE,
    SK_DISPEL, SK_ENHANCE, SK_GHOST, SK_HASTE, SK_HEAL, SK_IDENT, SK_LETHARGY, SK_LIGHT, SK_MSHIELD, SK_POISON,
    SK_PROTECT, SK_PULSE, SK_RECALL, SK_SHADOW, SK_SLOW
};

static void get_skill_tab_info_from_id(const int skill_id, char *out_name, int *out_skill_tab_id) {
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
    char item[8];
    get_keybind_text_for_current_slot(current_slot, keybind_text, sizeof(keybind_text));
    if (skill_tab_id < 60) {
        if (pdata.xbutton[current_slot].skill_nr != skilltab[skill_tab_id].nr) {
            pdata.xbutton[current_slot].skill_nr = skilltab[skill_tab_id].nr;
            snprintf(pdata.xbutton[current_slot].name, 7, "%s", skilltab[skill_tab_id].name);
            xlog(1, "%s is now %s.", keybind_text, skilltab[skill_tab_id].name);
        } else {
            pdata.xbutton[current_slot].skill_nr = -1;
            xlog(1, "%s is now unassigned.", keybind_text);
        }
    } else if (skill_tab_id >= 200) {
        // Technically Equipment
        static char gear_name[20][8] = {
            "*Helmet", "*Neckla", "*Armor", "*Gloves", "*Belt",
            "*Tarot1", "*Boots", "*Offhan", "*Weapon", "*Cloak",
            "*L-Ring", "*R-Ring", "*Tarot2", "?", "?",
            "?", "?", "?", "?", "?"
        };

        if (pdata.xbutton[current_slot].skill_nr == skill_tab_id) {
            pdata.xbutton[current_slot].skill_nr = -1;
            xlog(1, "%s is now unassigned.", keybind_text);
        } else {
            pdata.xbutton[current_slot].skill_nr = skill_tab_id;
            snprintf(item, 7, "%s", gear_name[skill_tab_id - 200]);;
            xlog(1, "%s is now %s", keybind_text, item);
            strncpy(pdata.xbutton[current_slot].name, item, 7);
            pdata.xbutton[current_slot].name[7] = 0;
        }
    } else if (skill_tab_id >= 100) {
        if (pdata.xbutton[current_slot].skill_nr == skill_tab_id) {
            pdata.xbutton[current_slot].skill_nr = -1;
            xlog(1, "%s is now unassigned.", keybind_text);
        } else {
            pdata.xbutton[current_slot].skill_nr = skill_tab_id;
            snprintf(item, 7, "Item %d", skill_tab_id - 100);
            xlog(1, "%s is now %s", keybind_text, item);
            strncpy(pdata.xbutton[current_slot].name, item, 7);
            pdata.xbutton[current_slot].name[7] = 0;
        }
    }
}

static void render_spell_popover() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
    if (ImGui::BeginPopup("SpellPopup1")) {
        if (ImGui::Selectable("-", false)) {
            pdata.xbutton[current_slot].skill_nr = -1;
        }
        for (int i = 0; i < 26; i++) {
            int skill_id = usable_skills[i];
            int skill_tab_id;
            char skill_name[32];
            if ((skill_id == 52 && !KNOW_IDENTIFY) || ((skill_id == 53 || skill_id == 54) && !IS_LYCANTH)) {
                continue;
            }
            get_skill_tab_info_from_id(skill_id, skill_name, &skill_tab_id);
            if (skill_tab_id == -1) continue;
            if (pl.skill[skill_tab_id][0] == 0) continue;
            if (ImGui::Selectable(skill_name, false)) {
                handle_spell_selection(skill_tab_id);
            }
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(1);
}


void spell_hud() {
    ImGui::SetNextWindowPos(ImVec2(1036, 597));
    ImGui::SetNextWindowSize(ImVec2(238, 58));
    auto pop_spell_hud_styles = ImGuiStyleBuilder()
            .Var(ImGuiStyleVar_WindowPadding, ImVec2(0, 0))
            .Var(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f))
            .Var(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 2.0f))
            .Var(ImGuiStyleVar_WindowBorderSize, 0.0f)
            .Font(static_cast<ImFont *>(font_sizes.ui))
            .Build();
    if (ImGui::Begin("##SPELLHUD", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar)) {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        float window_x = ImGui::GetWindowPos().x;
        float window_y = ImGui::GetWindowPos().y;
        auto pop_button_styles = ImGuiStyleBuilder()
                .Color(ImGuiCol_Text, GOLD_FONT_COLOR_32)
                .Color(ImGuiCol_Button, TRANSPARENT_COLOR_32)
                .Color(ImGuiCol_ButtonHovered, TRANSPARENT_COLOR_32)
                .Color(ImGuiCol_ButtonActive, TRANSPARENT_COLOR_32)
                .Build();
        for (int i = 0; i < 20; i++) {
            if (i > 0 && i % 5 != 0) {
                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2.0f);
            }

            if (i > 0 && i % 5 == 0) {
                ImGui::Dummy(ImVec2(0, 2.0f));
            }
            float x = window_x + ImGui::GetCursorPosX();
            float y = window_y + ImGui::GetCursorPosY();
            draw_list->AddRectFilled(
                ImVec2(x, y),
                ImVec2(x + 46, y + 13),
                BACKGROUND_COLOR_DARK_MODE,
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

            auto [keybind_width, keybind_height] = ImGui::CalcTextSize(binding_text);

            /* Center the keybind text within the button */
            float text_x = x + (46 - keybind_width);
            float text_y = y + (13 - keybind_height) / 2.0f;

            draw_list->AddText(ImVec2(text_x - 2, text_y + 1), HINT_GREY_FONT_COLOR_32, binding_text);
            draw_list->AddText(ImVec2(x + 2, text_y + 1), GOLD_FONT_COLOR_32, spell_text);

            if (ImGui::InvisibleButton(spell_key_id, ImVec2(46, 13))) {
                button_command(16 + i);
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                current_slot = i;
                if (last_skill == -1) {
                    ImGui::OpenPopup("SpellPopup1");
                } else {
                    handle_spell_selection(last_skill);
                }
            }
        }
        render_spell_popover();
        pop_button_styles();
    }
    ImGui::End();
    pop_spell_hud_styles();
}
