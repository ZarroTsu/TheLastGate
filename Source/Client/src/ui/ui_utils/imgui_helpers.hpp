#pragma once
#include <imgui.h>

#include "graphics/sprite_data.h"

void imgui_draw_sprite(ImDrawList *draw_list, const SpriteData *data, ImVec2 position);
void imgui_center_next_item(float item_width);
void imgui_center_next_text(const char *text);