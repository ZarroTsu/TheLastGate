//
// Created by james on 1/6/2026.
//

#include "imgui_helpers.hpp"

static constexpr ImColor WHITE_TINT = 0xFFFFFFFF;

void imgui_draw_sprite(ImDrawList *draw_list, const SpriteData *data, const ImVec2 position) {
    if (!data->loaded_in_atlas) return;

    const ImTextureID texture = data->atlas_texture;
    draw_list->AddImage(texture,
                        position,
                        ImVec2(position.x + data->pixel_width, position.y + data->pixel_height),
                        ImVec2(data->uv0.u, data->uv0.v),
                        ImVec2(data->uv1.u, data->uv1.v),
                        WHITE_TINT);
}

void imgui_center_next_item(const float item_width) {
    const float avail_width = ImGui::GetContentRegionAvail().x;
    if (const float offset = (avail_width - item_width) * 0.5F; offset > 0) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    }
}

void imgui_center_next_text(const char *text) {
    const float text_width = ImGui::CalcTextSize(text).x;
    imgui_center_next_item(text_width);
}