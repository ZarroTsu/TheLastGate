#include "launcher_ui.hpp"
#include <imgui.h>

#include "inter.h"
#include "main.h"
#include "options.h"
#include "tinyfiledialogs.h"
#include "ui.h"
#include "config/config.h"
#include "graphics/sdl.h"
#include "launcher/character_manager.h"
#include "launcher/launcher.h"
#include "log/log.h"
#include "ui_utils/imgui_helpers.hpp"
#include "ui_utils/imgui_style_builder.hpp"

typedef struct {
    ImU32 text_gold_color;
    ImU32 text_black_color;
    ImU32 subheader_text_color;
    ImU32 warning_text_color;
    ImU32 important_text_color;
    ImU32 border_color;
    ImU32 popover_background_color;
    ImU32 popover_title_color;
    ImU32 button_color;
    ImU32 button_text_color;
    ImU32 button_hovered_color;
    ImU32 button_active_color;
    ImU32 button_disabled_color;
    ImU32 input_background_color;
    ImU32 input_background_hovered_color;
    ImU32 input_background_active_color;
    ImU32 dropdown_selected_color;
    ImU32 dropdown_hovered_color;
    ImU32 dropdown_active_color;
    ImU32 previous_characters_background_color;
    ImU32 checkmark_color;

    ImVec2 window_size;
    ImVec2 window_padding;
    int background_sprite;

    ImVec2 main_button_size;
    float main_button_spacing;

    float section_header_spacing;

    float character_select_label_column_width;
    float character_select_input_column_width;
    float character_select_field_min_height;
    ImVec2 character_select_desc_size;

    ImVec2 previous_character_size;
    ImVec2 previous_character_button_size;

    ImVec2 popover_position;
    ImVec2 popover_size;
    ImVec2 popover_window_padding;
    ImVec2 popover_frame_padding;
    ImVec2 popover_inner_size;
    float popover_border_size;
    ImVec2 popover_button_size;
    float popover_button_spacing;

    float button_border_size;
} LauncherLayout;

static constexpr LauncherLayout LAUNCHER_LAYOUT = {
    .text_gold_color = 0xFF6ADBF3,
    .text_black_color = 0xFF000000,
    .subheader_text_color = 0xFF4AABCE,
    .warning_text_color = 0xFFAFD8E9,
    .important_text_color = 0xFFAB811E,
    .border_color = 0xFF446A75,
    .popover_background_color = 0xFF210004,
    .popover_title_color = 0xFF446A75,
    .button_color = 0xFF4BABCE,
    .button_text_color = 0xFF000000,
    .button_hovered_color = 0xFF3693BC,
    .button_active_color = 0xFF2C74A2,
    .button_disabled_color = 0xFF0D353E,
    .input_background_color = 0xFFAFD8E9,
    .input_background_hovered_color = 0xFF71B6D8,
    .input_background_active_color = 0xFF509FCF,
    .dropdown_selected_color = 0xFF7F434C,
    .dropdown_hovered_color = 0xFF6F4046,
    .dropdown_active_color = 0x7F412529,
    .previous_characters_background_color = 0xFF210004,
    .checkmark_color = 0xFF6F4046,

    .window_size = ImVec2(SCREEN_WIDTH, SCREEN_HEIGHT),
    .window_padding = ImVec2(110, 72),
    .background_sprite = 18130,

    .main_button_size = ImVec2(120, 30),
    .main_button_spacing = 10,

    .section_header_spacing = 30,

    .character_select_label_column_width = 150,
    .character_select_input_column_width = 240,
    .character_select_field_min_height = 20,
    .character_select_desc_size = ImVec2(-1, 80),

    .previous_character_size = ImVec2(-1, 200),
    .previous_character_button_size = ImVec2(240, 24),

    .popover_position = ImVec2(UI_CENTER_X - 200, UI_CENTER_Y - 100),
    .popover_size = ImVec2(400, 200),
    .popover_window_padding = ImVec2(4, 4),
    .popover_frame_padding = ImVec2(4, 4),
    .popover_inner_size = ImVec2(350, 100),
    .popover_border_size = 2,
    .popover_button_size = ImVec2(120, 0),
    .popover_button_spacing = 10,

    .button_border_size = 2,
};

static const LauncherLayout *get_layout() {
    return &LAUNCHER_LAYOUT;
}

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

static constexpr ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

static std::function<void()> create_button_style(const LauncherLayout *layout) {
    return ImGuiStyleBuilder()
            .Var(ImGuiStyleVar_FrameBorderSize, layout->button_border_size)
            .Color(ImGuiCol_Border, layout->border_color)
            .Color(ImGuiCol_Button, layout->button_color)
            .Color(ImGuiCol_ButtonHovered, layout->button_hovered_color)
            .Color(ImGuiCol_ButtonActive, layout->button_active_color)
            .Color(ImGuiCol_Text, layout->button_text_color)
            .Font(static_cast<ImFont *>(font_sizes_bold.normal))
            .Build();
}

static std::function<void()> create_input_style(const LauncherLayout *layout) {
    return ImGuiStyleBuilder()
            .Var(ImGuiStyleVar_FrameBorderSize, layout->button_border_size)
            .Color(ImGuiCol_Border, layout->border_color)
            .Color(ImGuiCol_FrameBg, layout->input_background_color)
            .Color(ImGuiCol_FrameBgHovered, layout->input_background_hovered_color)
            .Color(ImGuiCol_FrameBgActive, layout->input_background_active_color)
            .Color(ImGuiCol_Text, layout->text_black_color)
            .Color(ImGuiCol_InputTextCursor, layout->text_black_color)
            .Color(ImGuiCol_CheckMark, layout->checkmark_color)
            .Build();
}

static const char *filterPatterns[1] = {"*.moa"};

static void start_load_switch_character_confirmation(const char *file_path) {
    strncpy(launcher_state.switch_character_confirm_state.file_path, file_path,
            sizeof(launcher_state.switch_character_confirm_state.file_path) - 1);
    launcher_state.switch_character_confirm_state.show_first_confirmation = true;
}

static void start_new_switch_character_confirmation() {
    launcher_state.switch_character_confirm_state.new_character = true;
    launcher_state.switch_character_confirm_state.show_first_confirmation = true;
}

static void load() {
    char const *file_path = tinyfd_openFileDialog(
        "Select your MOA",
        "",
        1,
        filterPatterns, "Astonia Save File (.MOA)", 0);

    if (file_path != nullptr) {
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

    if (file_path != nullptr) {
        log_info("Saving MOA to %s", file_path);
        save_character_to_file(file_path);
    }
}

static std::function<void()> create_popover_styles(const LauncherLayout *layout) {
    return ImGuiStyleBuilder()
            .Var(ImGuiStyleVar_WindowPadding, layout->popover_window_padding)
            .Var(ImGuiStyleVar_FramePadding, layout->popover_frame_padding)
            .Var(ImGuiStyleVar_WindowBorderSize, layout->popover_border_size)
            .Color(ImGuiCol_PopupBg, layout->popover_background_color)
            .Color(ImGuiCol_TitleBgActive, layout->popover_title_color)
            .Color(ImGuiCol_Border, layout->border_color)
            .Build();
}

static void render_confirmation_popover(const LauncherLayout *layout) {
    if (launcher_state.switch_character_confirm_state.show_first_confirmation) {
        ImGui::OpenPopup("Are you sure?");
        launcher_state.switch_character_confirm_state.show_first_confirmation = false;
    }

    ImGui::SetNextWindowPos(layout->popover_position);
    ImGui::SetNextWindowSize(layout->popover_size);
    const auto pop_popover_styles = create_popover_styles(layout);

    if (ImGui::BeginPopupModal("Are you sure?", nullptr,
                               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoSavedSettings)) {
        imgui_center_next_item(layout->popover_inner_size.x);
        if (ImGui::BeginChild("text_container", layout->popover_inner_size, 0, ImGuiWindowFlags_NoScrollbar)) {
            if (launcher_state.switch_character_confirm_state.new_character) {
                static const auto *confirm_text = "Do you really want to create a new account?";
                imgui_center_next_text(confirm_text);
                ImGui::TextUnformatted(confirm_text);
            } else {
                static const auto *switch_accounts = "Do you really want to switch accounts?";
                imgui_center_next_text(switch_accounts);
                ImGui::TextUnformatted(switch_accounts);
            }
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, layout->warning_text_color);
            ImGui::TextWrapped("Your old account will no longer be accessible, unless you remembered to save it.");
            ImGui::PopStyleColor(1);

            static const auto *warning_text = "You did save it, didn't you?";
            imgui_center_next_text(warning_text);
            ImGui::TextColored(ImColor(layout->important_text_color), "%s", warning_text);
            ImGui::EndChild();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        imgui_center_next_item((layout->popover_button_size.x * 2) + layout->popover_button_spacing);
        const auto pop_button_styles = create_button_style(layout);
        if (ImGui::Button("Yes", layout->popover_button_size)) {
            ImGui::CloseCurrentPopup();
            launcher_state.switch_character_confirm_state.show_second_confirmation = true;
        }

        ImGui::SameLine(0, layout->popover_button_spacing);

        if (ImGui::Button("Cancel", layout->popover_button_size)) {
            ImGui::CloseCurrentPopup();
            launcher_state.switch_character_confirm_state.file_path[0] = '\0';
            launcher_state.switch_character_confirm_state.new_character = false;
        }
        pop_button_styles();
        ImGui::EndPopup();
    }

    if (launcher_state.switch_character_confirm_state.show_second_confirmation) {
        ImGui::OpenPopup("Are you really sure?");
        launcher_state.switch_character_confirm_state.show_second_confirmation = false;
    }

    ImGui::SetNextWindowPos(layout->popover_position);
    ImGui::SetNextWindowSize(layout->popover_size);
    if (ImGui::BeginPopupModal("Are you really sure?", nullptr,
                               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoSavedSettings)) {
        imgui_center_next_item(layout->popover_inner_size.x);
        if (ImGui::BeginChild("text_container", layout->popover_inner_size, 0, ImGuiWindowFlags_NoScrollbar)) {
            static const char *no_really_text = "No Really!";
            imgui_center_next_text(no_really_text);
            ImGui::TextColored(ImColor(layout->important_text_color), "%s", no_really_text);
            ImGui::Spacing();

            ImGui::TextWrapped("This will replace your current character.");
            static const char *no_undo = "This action cannot be undone.";
            imgui_center_next_text(no_undo);
            ImGui::TextUnformatted(no_undo);
            ImGui::EndChild();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        imgui_center_next_item((layout->popover_button_size.x * 2) + layout->popover_button_spacing);
        const auto pop_button_styles = create_button_style(layout);
        if (ImGui::Button("Yes", layout->popover_button_size)) {
            ImGui::CloseCurrentPopup();
            launcher_state.switch_character_confirm_state.show_second_confirmation = false;
            if (launcher_state.switch_character_confirm_state.new_character) {
                create_new_character();
                okey_to_class_gender(&launcher_state.current_class, &launcher_state.current_gender);
            } else {
                load_character_from_file(launcher_state.switch_character_confirm_state.file_path);
                okey_to_class_gender(&launcher_state.current_class, &launcher_state.current_gender);
            }
            launcher_state.switch_character_confirm_state.file_path[0] = '\0';
            launcher_state.switch_character_confirm_state.new_character = false;
        }

        ImGui::SameLine(0, layout->popover_button_spacing);

        if (ImGui::Button("Cancel", layout->popover_button_size)) {
            ImGui::CloseCurrentPopup();
            launcher_state.switch_character_confirm_state.file_path[0] = '\0';
            launcher_state.switch_character_confirm_state.new_character = false;
        }
        pop_button_styles();

        ImGui::EndPopup();
    }

    pop_popover_styles();
}

static void render_fyi_popover(const LauncherLayout *layout) {
    if (launcher_state.simple_popover_state.display) {
        ImGui::OpenPopup("FYI");
        launcher_state.simple_popover_state.display = false;
    }

    ImGui::SetNextWindowPos(layout->popover_position);
    ImGui::SetNextWindowSize(layout->popover_size);
    const auto pop_popover_styles = create_popover_styles(layout);
    if (ImGui::BeginPopupModal("FYI", nullptr,
                               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoSavedSettings)) {
        imgui_center_next_item(layout->popover_inner_size.x);
        if (ImGui::BeginChild("text_container", layout->popover_inner_size, 0, ImGuiWindowFlags_NoScrollbar)) {
            ImGui::TextWrapped("%s", launcher_state.simple_popover_state.text);
            ImGui::EndChild();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        imgui_center_next_item(layout->popover_button_size.x);
        const auto pop_button_styles = create_button_style(layout);
        if (ImGui::Button("Ok", layout->popover_button_size)) {
            ImGui::CloseCurrentPopup();
        }
        pop_button_styles();
        ImGui::EndPopup();
    }

    pop_popover_styles();
}

static void render_connection_popover(const LauncherLayout *layout) {
    ImGui::SetNextWindowPos(layout->popover_position);
    ImGui::SetNextWindowSize(layout->popover_size);
    const auto pop_popover_styles = create_popover_styles(layout);
    if (ImGui::BeginPopupModal("Connection", nullptr,
                               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoSavedSettings)) {
        imgui_center_next_item(layout->popover_inner_size.x);
        if (ImGui::BeginChild("text_container", layout->popover_inner_size, 0, ImGuiWindowFlags_NoScrollbar)) {
            ImGui::TextWrapped("%s", launcher_state.connection_status_text);
            ImGui::EndChild();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        imgui_center_next_item(layout->popover_button_size.x);
        ImGui::BeginDisabled(
            launcher_state.connection_status.state != CONNECTION_STATE_CONNECTED && launcher_state.connection_status.
            state != CONNECTION_STATE_ERROR);
        const auto pop_button_styles = create_button_style(layout);
        if (ImGui::Button("Ok", layout->popover_button_size)) {
            ImGui::CloseCurrentPopup();
        }
        pop_button_styles();
        ImGui::EndDisabled();
        ImGui::EndPopup();
    }

    pop_popover_styles();
}

static bool render_dropdown(const LauncherLayout *layout, const char *id, const char **options, const int option_count,
                            int *current_option) {
    bool changed = false;
    auto pop_dropdown_style = ImGuiStyleBuilder()
            .Var(ImGuiStyleVar_WindowPadding, layout->popover_frame_padding)
            .Color(ImGuiCol_PopupBg, layout->popover_background_color)
            .Build();

    const auto pop_dropdown_input_style = create_input_style(layout);
    bool opened = ImGui::BeginCombo(id, options[*current_option], ImGuiComboFlags_NoArrowButton);
    pop_dropdown_input_style();

    auto pop_dropdown_popover_styles = ImGuiStyleBuilder()
            .Var(ImGuiStyleVar_PopupBorderSize, layout->popover_border_size)
            .Color(ImGuiCol_Header, layout->dropdown_selected_color)
            .Color(ImGuiCol_HeaderHovered, layout->dropdown_hovered_color)
            .Color(ImGuiCol_HeaderActive, layout->dropdown_active_color)
            .Color(ImGuiCol_Border, layout->border_color)
            .Build();

    if (opened) {
        for (int i = 0; i < option_count; i++) {
            const bool selected = (*current_option == i);

            if (ImGui::Selectable(options[i], selected)) {
                changed = true;
                *current_option = i;
            }

            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    pop_dropdown_popover_styles();
    pop_dropdown_style();
    return changed;
}

static void render_previous_characters(const LauncherLayout *layout) {
    auto pop_previous_character_style = ImGuiStyleBuilder()
            .Var(ImGuiStyleVar_WindowPadding, layout->popover_window_padding)
            .Var(ImGuiStyleVar_ChildBorderSize, layout->popover_border_size)
            .Color(ImGuiCol_ChildBg, layout->previous_characters_background_color)
            .Color(ImGuiCol_Border, layout->border_color)
            .Build();
    if (ImGui::BeginChild("previous_characters", layout->previous_character_size, ImGuiChildFlags_Borders,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        const auto pop_sub_header_style = ImGuiStyleBuilder()
                .Font(static_cast<ImFont *>(font_sizes.subheader))
                .Color(ImGuiCol_Text, layout->text_gold_color)
                .Build();
        static const char *previous_character_header = "Previous Characters";
        imgui_center_next_text(previous_character_header);
        ImGui::TextUnformatted(previous_character_header);
        pop_sub_header_style();

        ImGui::Dummy(ImVec2(0, layout->section_header_spacing / 2));

        if (ImGui::BeginChild("previous_character_list", ImVec2(-1, -1), 0, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            for (int i = 0; i < get_previous_characters()->count; i++) {
                PreviousCharacterEntry entry = get_previous_characters()->entries[i];
                const auto pop_button_styles = create_button_style(layout);
                imgui_center_next_item(layout->previous_character_button_size.x);
                ImGui::PushID(entry.file_path);
                if (ImGui::Button(entry.name, layout->previous_character_button_size)) {
                    load_character_from_file(entry.file_path);
                    okey_to_class_gender(&launcher_state.current_class, &launcher_state.current_gender);
                }
                ImGui::PopID();
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary | ImGuiHoveredFlags_DelayNormal)) {
                    ImGui::PushStyleColor(ImGuiCol_Text, layout->text_gold_color);
                    ImGui::SetTooltip("%s", entry.file_path);
                    ImGui::PopStyleColor(1);
                }
                pop_button_styles();
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }
    pop_previous_character_style();
}

static std::function<void()> create_header_styles(const LauncherLayout *layout) {
    return ImGuiStyleBuilder()
            .Color(ImGuiCol_Text, layout->important_text_color)
            .Font(static_cast<ImFont *>(font_sizes.header))
            .Build();
}

static void render_character_select(const LauncherLayout *layout) {
    const auto pop_header_styles = create_header_styles(layout);
    static const char *character_select_text = "Character Selection";
    imgui_center_next_text(character_select_text);
    ImGui::TextUnformatted(character_select_text);
    pop_header_styles();
    ImGui::Dummy(ImVec2(0, layout->section_header_spacing));

    imgui_center_next_item(layout->character_select_label_column_width + layout->character_select_input_column_width);

    if (ImGui::BeginTable("current_account", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
        ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, layout->character_select_label_column_width,
                                0);
        ImGui::TableSetupColumn("input", ImGuiTableColumnFlags_WidthFixed, layout->character_select_input_column_width,
                                1);

        ImGui::TableNextRow(0, layout->character_select_field_min_height);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(okey.name);
        ImGui::TableNextColumn();

        const auto pop_button_styles = create_button_style(layout);
        if (ImGui::Button("Create New Account")) {
            start_new_switch_character_confirmation();
        }
        pop_button_styles();

        ImGui::EndTable();
    }

    imgui_center_next_item(layout->character_select_label_column_width + layout->character_select_input_column_width);
    if (ImGui::BeginTable("character_fields", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX)) {
        ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, layout->character_select_label_column_width,
                                0);
        ImGui::TableSetupColumn("input", ImGuiTableColumnFlags_WidthFixed, layout->character_select_input_column_width,
                                1);

        /* Name */
        ImGui::TableNextRow(0, layout->character_select_field_min_height);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Name");
        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-1);
        auto pop_input_styles = create_input_style(layout);
        ImGui::InputText("##name", pdata.cname, sizeof(pdata.cname));
        pop_input_styles();
        ImGui::PopItemWidth();

        /* Password */
        ImGui::TableNextRow(0, layout->character_select_field_min_height);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Password");
        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-1);
        pop_input_styles = create_input_style(layout);
        ImGui::InputText("##password", launcher_state.password, sizeof(launcher_state.password));
        pop_input_styles();
        ImGui::PopItemWidth();

        /* Description */
        ImGui::TableNextRow(0, layout->character_select_field_min_height);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Description");
        ImGui::TableNextColumn();
        pop_input_styles = create_input_style(layout);
        if (ImGui::InputTextMultiline("##description", pdata.desc, sizeof(pdata.desc),
                                      layout->character_select_desc_size, ImGuiInputTextFlags_WordWrap)) {
            pdata.changed = 1;
        }
        pop_input_styles();


        ImGui::BeginDisabled(okey.usnr > 0);
        /* Class */
        ImGui::TableNextRow(0, layout->character_select_field_min_height);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Class");
        ImGui::TableNextColumn();
        render_dropdown(layout, "##class", classes, 3, &launcher_state.current_class);

        /* Gender */
        ImGui::TableNextRow(0, layout->character_select_field_min_height);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Gender");
        ImGui::TableNextColumn();
        render_dropdown(layout, "##gender", genders, 2, &launcher_state.current_gender);

        ImGui::EndDisabled();


        ImGui::EndTable();
    }

    ImGui::Dummy(ImVec2(0, layout->section_header_spacing));

    render_previous_characters(layout);
}

static void render_options(const LauncherLayout *layout) {
    const auto pop_header_styles = create_button_style(layout);
    static const char *options_header = "Options";
    imgui_center_next_text(options_header);
    ImGui::TextUnformatted(options_header);
    pop_header_styles();
    ImGui::Dummy(ImVec2(0, layout->section_header_spacing));

    imgui_center_next_item(layout->character_select_label_column_width + layout->character_select_input_column_width);
    if (ImGui::BeginTable("Options", 2,
                          ImGuiTableFlags_SizingFixedFit |
                          ImGuiTableFlags_NoHostExtendX)) {
        ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, layout->character_select_label_column_width,
                                0);
        ImGui::TableSetupColumn("input", ImGuiTableColumnFlags_WidthFixed, layout->character_select_input_column_width,
                                1);

        ImGui::TableNextRow(0, layout->character_select_field_min_height);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Alpha");
        ImGui::TableNextColumn();
        render_dropdown(layout, "##alphas", alphas, 3, &do_alpha);

        ImGui::TableNextRow(0, layout->character_select_field_min_height);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Window Type");
        ImGui::TableNextColumn();
        const bool prev_windowed = g_config.video.windowed;
        int window_mode = g_config.video.windowed ? 1 : 0;
        if (render_dropdown(layout, "##window_modes", window_modes, 2, &window_mode)) {
            g_config.video.windowed = (window_mode == 1);
        }
        if (prev_windowed != g_config.video.windowed) {
            sdl_set_fullscreen(g_config.video.windowed);
        }

        ImGui::TableNextRow(0, layout->character_select_field_min_height);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Shadows");
        ImGui::TableNextColumn();
        auto pop_checkbox_styles = create_input_style(layout);
        ImGui::Checkbox("##shadow", &do_shadow);
        pop_checkbox_styles();

        ImGui::TableNextRow(0, layout->character_select_field_min_height);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Sounds Enabled?");
        ImGui::TableNextColumn();
        pop_checkbox_styles = create_input_style(layout);
        ImGui::Checkbox("##do_sound", &g_config.audio.sound_enabled);
        pop_checkbox_styles();

        ImGui::TableNextRow(0, layout->character_select_field_min_height);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Volume Level");
        ImGui::TableNextColumn();
        auto pop_slider_styles = ImGuiStyleBuilder()
                .Var(ImGuiStyleVar_FrameBorderSize, 2)
                .Color(ImGuiCol_Border, layout->border_color)
                .Color(ImGuiCol_Text, layout->previous_characters_background_color)
                .Color(ImGuiCol_FrameBg, layout->input_background_color)
                .Color(ImGuiCol_FrameBgHovered, layout->input_background_hovered_color)
                .Color(ImGuiCol_FrameBgActive, layout->input_background_active_color)
                .Color(ImGuiCol_SliderGrab, layout->dropdown_hovered_color)
                .Color(ImGuiCol_SliderGrabActive, layout->dropdown_hovered_color)
                .Build();
        ImGui::SliderInt("##volume_level", &g_config.audio.sound_volume, 0, MAX_VOLUME_LEVEL);
        pop_slider_styles();

        ImGui::TableNextRow(0, layout->character_select_field_min_height);
        ImGui::TableNextColumn();
        ImGui::TextUnformatted("Dark GUI");
        ImGui::TableNextColumn();
        pop_checkbox_styles = create_input_style(layout);

        ImGui::Checkbox("##dark_ui", &do_darkmode);
        pop_checkbox_styles();

        ImGui::EndTable();
    }
}

void launcher_ui_render(void) {
    const LauncherLayout *layout = get_layout();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(layout->window_size);
    sdl_load_sprite(layout->background_sprite);
    auto pop_launcher_styles = ImGuiStyleBuilder()
            .Var(ImGuiStyleVar_WindowPadding, layout->window_padding)
            .Color(ImGuiCol_Text, layout->text_gold_color)
            .Font(static_cast<ImFont *>(font_sizes.normal))
            .Build();
    if (ImGui::Begin("Launcher", nullptr, window_flags)) {
        imgui_draw_sprite(ImGui::GetWindowDrawList(), &sprite_data[layout->background_sprite], ImVec2(0, 0));

        render_confirmation_popover(layout);
        render_fyi_popover(layout);
        render_connection_popover(layout);

        if (ImGui::BeginTable("Launcher Layout", 2, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            render_character_select(layout);
            ImGui::TableNextColumn();
            render_options(layout);
            ImGui::BeginChild("filler", ImVec2(0, -1), 0, 0);
            ImGui::EndChild();
            ImGui::EndTable();
        }
    }

    static constexpr int BUTTON_COUNT = 5;
    imgui_center_next_item(
        (layout->main_button_size.x * BUTTON_COUNT) + (layout->main_button_spacing * BUTTON_COUNT - 1));
    const auto pop_button_styles = create_button_style(layout);
    if (ImGui::Button("Start", layout->main_button_size)) {
        ImGui::OpenPopup("Connection");
        launcher_connect();
    }
    ImGui::SameLine(0, layout->main_button_spacing);
    if (ImGui::Button("Load", layout->main_button_size)) {
        load();
    }
    ImGui::SameLine(0, layout->main_button_spacing);
    if (ImGui::Button("Save", layout->main_button_size)) {
        save();
    }
    ImGui::SameLine(0, layout->main_button_spacing);
    if (ImGui::Button("Discord", layout->main_button_size)) {
        SDL_OpenURL(MNEWS);
    }
    ImGui::SameLine(0, layout->main_button_spacing);
    if (ImGui::Button("Exit", layout->main_button_size)) {
        quit = 1;
    }
    pop_button_styles();
    ImGui::End();
    pop_launcher_styles();
}
