//
// Created by james on 1/4/2026.
//

#include "imgui_style_builder.hpp"

ImGuiStyleBuilder &ImGuiStyleBuilder::Var(ImGuiStyleVar idx, float val) {
    ImGui::PushStyleVar(idx, val);
    ++vars_pushed;
    return *this;
}

ImGuiStyleBuilder &ImGuiStyleBuilder::Var(ImGuiStyleVar idx, const ImVec2 &vec) {
    ImGui::PushStyleVar(idx, vec);
    ++vars_pushed;
    return *this;
}

ImGuiStyleBuilder &ImGuiStyleBuilder::Color(ImGuiCol idx, ImU32 val) {
    ImGui::PushStyleColor(idx, val);
    ++colors_pushed;
    return *this;
}

ImGuiStyleBuilder &ImGuiStyleBuilder::Font(ImFont *font) {
    ImGui::PushFont(font);
    ++fonts_pushed;
    return *this;
}

std::function<void()> ImGuiStyleBuilder::Build() {
    int vars = vars_pushed;
    int colors = colors_pushed;
    int fonts = fonts_pushed;

    vars_pushed = 0;
    colors_pushed = 0;
    fonts_pushed = 0;

    return [vars, colors, fonts]() {
        if (vars > 0) {
            ImGui::PopStyleVar(vars);
        }

        if (colors > 0) {
            ImGui::PopStyleColor(colors);
        }

        if (fonts > 0) {
            ImGui::PopFont();
        }
    };
}
