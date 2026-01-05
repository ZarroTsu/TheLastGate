#pragma once

#include <imgui.h>
#include <functional>

class ImGuiStyleBuilder {
public:
    ImGuiStyleBuilder() = default;

    ImGuiStyleBuilder &Var(ImGuiStyleVar idx, float val);

    ImGuiStyleBuilder &Var(ImGuiStyleVar idx, const ImVec2 &vec);

    ImGuiStyleBuilder &Color(ImGuiCol idx, ImU32 val);

    ImGuiStyleBuilder &Color(ImGuiCol idx, const ImVec4 &val);

    ImGuiStyleBuilder &Font(ImFont *font);

    std::function<void()> Build();

private:
    int vars_pushed = 0;
    int colors_pushed = 0;
    int fonts_pushed = 0;
};
