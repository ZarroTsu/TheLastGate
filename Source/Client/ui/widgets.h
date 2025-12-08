#pragma once
#include <stdbool.h>

#include "imgui_wrapper.h"

#define BUTTON_TEXT_COLOR imgui_color_convert_float4_to_u32(0.95f, 0.85f, 0.41f, 1.0f)
#define BUTTON_TEXT_HOVER_COLOR imgui_color_convert_float4_to_u32(0.85f, 0.75f, 0.31f, 1.0f)
#define BUTTON_TEXT_ACTIVE_COLOR imgui_color_convert_float4_to_u32(0.75f, 0.65f, 0.21f, 1.0f)

#define BUTTON_TINT imgui_color_convert_float4_to_u32(1.0f, 1.0f, 1.0f, 1.0f)
#define BUTTON_HOVER_TINT imgui_color_convert_float4_to_u32(0.7f, 0.7f, 0.7f, 1.0f)
#define BUTTON_ACTIVE_TINT imgui_color_convert_float4_to_u32(0.85f, 0.85f, 0.85f, 1.0f);

bool ui_button(const char *label, float width, float height);
bool tab_button(const char* label, bool is_active, float width);
bool keybind(const char *keybind_label);