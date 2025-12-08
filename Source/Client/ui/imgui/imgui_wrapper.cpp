/*
 * imgui_wrapper.cpp
 *
 * C++ implementation of the C wrapper for Dear ImGui
 * This file is the ONLY C++ file in the project that interacts with ImGui
 */

#include "imgui_wrapper.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <SDL2/SDL.h>

/* Initialization and lifecycle */

void imgui_init(void* sdl_window, void* gl_context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    /* Enable keyboard and gamepad navigation if desired */
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    /* Setup Dear ImGui style */
    ImGui::StyleColorsDark();

    /* Setup platform/renderer backends */
    ImGui_ImplSDL2_InitForOpenGL(static_cast<SDL_Window*>(sdl_window), gl_context);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void imgui_shutdown(void) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void imgui_new_frame(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void imgui_render(void) {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/* Font management */

void* imgui_add_font_from_file_ttf(const char* filename, float size_pixels) {
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF(filename, size_pixels);
    return static_cast<void*>(font);
}

void* imgui_add_font_from_file_ttf_pixel_perfect(const char* filename, float size_pixels) {
    ImGuiIO& io = ImGui::GetIO();

    /* Configure for pixel-perfect rendering */
    ImFontConfig config;
    config.OversampleH = 1;  /* No horizontal oversampling */
    config.OversampleV = 1;  /* No vertical oversampling */
    config.PixelSnapH = true;  /* Align to pixel grid */

    ImFont* font = io.Fonts->AddFontFromFileTTF(filename, size_pixels, &config);
    return static_cast<void*>(font);
}

void imgui_set_default_font(void* font) {
    ImGuiIO& io = ImGui::GetIO();
    io.FontDefault = static_cast<ImFont*>(font);
}

void imgui_push_font(void* font) {
    ImGui::PushFont(static_cast<ImFont*>(font));
}

void imgui_pop_font(void) {
    ImGui::PopFont();
}

/* Display scaling */

void imgui_set_display_size(float width, float height) {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
}

void imgui_set_display_framebuffer_scale(float scale_x, float scale_y) {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplayFramebufferScale = ImVec2(scale_x, scale_y);
}

/* Process SDL events */

bool imgui_process_event(void* sdl_event) {
    SDL_Event* event = static_cast<SDL_Event*>(sdl_event);
    ImGui_ImplSDL2_ProcessEvent(event);

    /* Return true if ImGui wants to capture this event */
    ImGuiIO& io = ImGui::GetIO();
    if (event->type == SDL_MOUSEMOTION ||
        event->type == SDL_MOUSEBUTTONDOWN ||
        event->type == SDL_MOUSEBUTTONUP ||
        event->type == SDL_MOUSEWHEEL) {
        return io.WantCaptureMouse;
    }
    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP || event->type == SDL_TEXTINPUT) {
        return io.WantCaptureKeyboard;
    }
    return false;
}

/* Window management */

void imgui_set_next_window_pos(float x, float y) {
    ImGui::SetNextWindowPos(ImVec2(x, y));
}

void imgui_set_next_windows_size(float width, float height) {
    ImGui::SetNextWindowSize(ImVec2(width, height));
}

bool imgui_begin(const char* name, bool* p_open, int flags) {
    /* Map our C flags to ImGui flags */
    ImGuiWindowFlags imgui_flags = 0;
    if (flags & IMGUI_WINDOW_FLAG_NO_TITLE_BAR)         imgui_flags |= ImGuiWindowFlags_NoTitleBar;
    if (flags & IMGUI_WINDOW_FLAG_NO_RESIZE)            imgui_flags |= ImGuiWindowFlags_NoResize;
    if (flags & IMGUI_WINDOW_FLAG_NO_MOVE)              imgui_flags |= ImGuiWindowFlags_NoMove;
    if (flags & IMGUI_WINDOW_FLAG_NO_SCROLLBAR)         imgui_flags |= ImGuiWindowFlags_NoScrollbar;
    if (flags & IMGUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE) imgui_flags |= ImGuiWindowFlags_NoScrollWithMouse;
    if (flags & IMGUI_WINDOW_FLAG_NO_COLLAPSE)          imgui_flags |= ImGuiWindowFlags_NoCollapse;
    if (flags & IMGUI_WINDOW_FLAG_ALWAYS_AUTO_RESIZE)   imgui_flags |= ImGuiWindowFlags_AlwaysAutoResize;
    if (flags & IMGUI_WINDOW_FLAG_NO_BACKGROUND)        imgui_flags |= ImGuiWindowFlags_NoBackground;
    if (flags & IMGUI_WINDOW_FLAG_NO_SAVED_SETTINGS)    imgui_flags |= ImGuiWindowFlags_NoSavedSettings;

    return ImGui::Begin(name, p_open, imgui_flags);
}

void imgui_end(void) {
    ImGui::End();
}

/* Child windows */

bool imgui_begin_child(const char* str_id, float width, float height, bool border, int flags) {
    /* Map our C flags to ImGui flags */
    ImGuiWindowFlags imgui_flags = 0;
    if (flags & IMGUI_WINDOW_FLAG_NO_SCROLLBAR)         imgui_flags |= ImGuiWindowFlags_NoScrollbar;
    if (flags & IMGUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE) imgui_flags |= ImGuiWindowFlags_NoScrollWithMouse;
    if (flags & IMGUI_WINDOW_FLAG_HORIZONTAL_SCROLLBAR) imgui_flags |= ImGuiWindowFlags_HorizontalScrollbar;
    if (flags & IMGUI_WINDOW_FLAG_ALWAYS_VERTICAL_SCROLLBAR) imgui_flags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
    if (flags & IMGUI_WINDOW_FLAG_ALWAYS_HORIZONTAL_SCROLLBAR) imgui_flags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;

    return ImGui::BeginChild(str_id, ImVec2(width, height), border, imgui_flags);
}

void imgui_end_child(void) {
    ImGui::EndChild();
}

/* Text and labels */

void imgui_text(const char* text) {
    ImGui::TextUnformatted(text);
}

void imgui_text_colored_rgb(float r, float g, float b, float a, const char* text) {
    ImGui::TextColored(ImVec4(r/255.0f, g/255.0f, b/255.0f, a), "%s", text);
}

void imgui_text_colored(float r, float g, float b, float a, const char* text) {
    ImGui::TextColored(ImVec4(r, g, b, a), "%s", text);
}

void imgui_text_disabled(const char* text) {
    ImGui::TextDisabled("%s", text);
}

void imgui_text_wrapped(const char* text) {
    ImGui::TextWrapped("%s", text);
}

void imgui_label_text(const char* label, const char* text) {
    ImGui::LabelText(label, "%s", text);
}

void imgui_bullet_text(const char* text) {
    ImGui::BulletText("%s", text);
}

/* Text utilities */

void imgui_calc_text_size(float* out_width, float* out_height, const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width) {
    ImVec2 size = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash, wrap_width);
    if (out_width) *out_width = size.x;
    if (out_height) *out_height = size.y;
}

void imgui_calc_text_size_simple(float* out_width, float* out_height, const char* text) {
    ImVec2 size = ImGui::CalcTextSize(text);
    if (out_width) *out_width = size.x;
    if (out_height) *out_height = size.y;
}

void imgui_push_text_wrap_pos(float wrap_local_pos_x) {
    ImGui::PushTextWrapPos(wrap_local_pos_x);
}

void imgui_pop_text_wrap_pos(void) {
    ImGui::PopTextWrapPos();
}

void imgui_set_font_global_scale(float scale) {
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = scale;
}

/* Widgets: Main */

bool imgui_button(const char* label) {
    return ImGui::Button(label);
}

bool imgui_button_sized(const char* label, float width, float height) {
    return ImGui::Button(label, ImVec2(width, height));
}

bool imgui_small_button(const char* label) {
    return ImGui::SmallButton(label);
}

bool imgui_invisible_button(const char* str_id, float width, float height) {
    return ImGui::InvisibleButton(str_id, ImVec2(width, height));
}

bool imgui_checkbox(const char* label, bool* v) {
    return ImGui::Checkbox(label, v);
}

bool imgui_radio_button(const char* label, bool active) {
    return ImGui::RadioButton(label, active);
}

/* Widgets: Input */

bool imgui_input_text(const char* label, char* buf, int buf_size) {
    return ImGui::InputText(label, buf, buf_size);
}

bool imgui_input_int(const char* label, int* v) {
    return ImGui::InputInt(label, v);
}

bool imgui_input_float(const char* label, float* v) {
    return ImGui::InputFloat(label, v);
}

/* Widgets: Sliders */

bool imgui_slider_int(const char* label, int* v, int v_min, int v_max) {
    return ImGui::SliderInt(label, v, v_min, v_max);
}

bool imgui_slider_float(const char* label, float* v, float v_min, float v_max) {
    return ImGui::SliderFloat(label, v, v_min, v_max);
}

/* Widgets: Color */

bool imgui_color_edit3(const char* label, float col[3]) {
    return ImGui::ColorEdit3(label, col);
}

bool imgui_color_edit4(const char* label, float col[4]) {
    return ImGui::ColorEdit4(label, col);
}

/* Layout */

void imgui_separator(void) {
    ImGui::Separator();
}

void imgui_same_line(float offset_from_start_x, float spacing) {
    ImGui::SameLine(offset_from_start_x, spacing);
}

void imgui_same_line_gap() {
    ImGui::SameLine();
}

void imgui_new_line(void) {
    ImGui::NewLine();
}

void imgui_spacing(void) {
    ImGui::Spacing();
}

void imgui_dummy(float width, float height) {
    ImGui::Dummy(ImVec2(width, height));
}

void imgui_indent(float indent_w) {
    ImGui::Indent(indent_w);
}

void imgui_unindent(float indent_w) {
    ImGui::Unindent(indent_w);
}

/* Grouping */

void imgui_begin_group(void) {
    ImGui::BeginGroup();
}

void imgui_end_group(void) {
    ImGui::EndGroup();
}

/* Cursor/Layout positioning */

void imgui_set_cursor_pos(float x, float y) {
    ImGui::SetCursorPos(ImVec2(x, y));
}

void imgui_set_cursor_pos_x(float x) {
    ImGui::SetCursorPosX(x);
}

void imgui_set_cursor_pos_y(float y) {
    ImGui::SetCursorPosY(y);
}

float imgui_get_cursor_pos_x(void) {
    return ImGui::GetCursorPosX();
}

float imgui_get_cursor_pos_y(void) {
    return ImGui::GetCursorPosY();
}

void imgui_align_text_to_frame_padding(void) {
    ImGui::AlignTextToFramePadding();
}

void imgui_center_next_item(float item_width) {
    float avail_width = ImGui::GetContentRegionAvail().x;
    float offset = (avail_width - item_width) * 0.5f;
    if (offset > 0.0f) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    }
}

/* Layout metrics */

float imgui_get_frame_height(void) {
    return ImGui::GetFrameHeight();
}

float imgui_get_frame_height_with_spacing(void) {
    return ImGui::GetFrameHeightWithSpacing();
}

void imgui_get_content_region_avail(float* out_width, float* out_height) {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (out_width) *out_width = avail.x;
    if (out_height) *out_height = avail.y;
}

/* Columns */

void imgui_columns(int count, const char* id, int border) {
    ImGui::Columns(count, id, border != 0);
}

void imgui_set_column_width(int index, float width) {
    ImGui::SetColumnWidth(index, width);
}

void imgui_next_column(void) {
    ImGui::NextColumn();
}

int imgui_get_column_index(void) {
    return ImGui::GetColumnIndex();
}

/* Tables (modern, preferred) */

bool imgui_begin_table(const char* str_id, int column_count, int flags) {
    return ImGui::BeginTable(str_id, column_count, flags);
}

void imgui_end_table(void) {
    ImGui::EndTable();
}

void imgui_table_next_row(int row_flags, float min_row_height) {
    ImGui::TableNextRow(row_flags, min_row_height);
}

bool imgui_table_next_column(void) {
    return ImGui::TableNextColumn();
}

bool imgui_table_set_column_index(int column_n) {
    return ImGui::TableSetColumnIndex(column_n);
}

void imgui_table_setup_column(const char* label, int flags, float init_width_or_weight, unsigned int user_id) {
    ImGui::TableSetupColumn(label, flags, init_width_or_weight, user_id);
}

void imgui_table_headers_row(void) {
    ImGui::TableHeadersRow();
}

void imgui_table_header(const char* label) {
    ImGui::TableHeader(label);
}

/* Tree/Collapsing */

bool imgui_tree_node(const char* label) {
    return ImGui::TreeNode(label);
}

void imgui_tree_pop(void) {
    ImGui::TreePop();
}

bool imgui_collapsing_header(const char* label, int flags) {
    return ImGui::CollapsingHeader(label, flags);
}

/* Tab bars */

bool imgui_begin_tab_bar(const char* str_id, int flags) {
    return ImGui::BeginTabBar(str_id, flags);
}

void imgui_end_tab_bar(void) {
    ImGui::EndTabBar();
}

bool imgui_begin_tab_item(const char* label, bool* p_open, int flags) {
    return ImGui::BeginTabItem(label, p_open, flags);
}

void imgui_end_tab_item(void) {
    ImGui::EndTabItem();
}

bool imgui_tab_item_button(const char* label, int flags) {
    return ImGui::TabItemButton(label, flags);
}

void imgui_set_tab_item_closed(const char* tab_or_docked_window_label) {
    ImGui::SetTabItemClosed(tab_or_docked_window_label);
}

/* Selectables */

bool imgui_selectable(const char* label, bool selected) {
    return ImGui::Selectable(label, selected);
}

/* Combo box */

bool imgui_begin_combo(const char* label, const char* preview_value) {
    return ImGui::BeginCombo(label, preview_value);
}

void imgui_end_combo(void) {
    ImGui::EndCombo();
}

/* List box */

bool imgui_begin_list_box(const char* label) {
    return ImGui::BeginListBox(label);
}

void imgui_end_list_box(void) {
    ImGui::EndListBox();
}

/* Menus */

bool imgui_begin_main_menu_bar(void) {
    return ImGui::BeginMainMenuBar();
}

void imgui_end_main_menu_bar(void) {
    ImGui::EndMainMenuBar();
}

bool imgui_begin_menu_bar(void) {
    return ImGui::BeginMenuBar();
}

void imgui_end_menu_bar(void) {
    ImGui::EndMenuBar();
}

bool imgui_begin_menu(const char* label, bool enabled) {
    return ImGui::BeginMenu(label, enabled);
}

void imgui_end_menu(void) {
    ImGui::EndMenu();
}

bool imgui_menu_item(const char* label, const char* shortcut, bool selected, bool enabled) {
    return ImGui::MenuItem(label, shortcut, selected, enabled);
}

/* Tooltips */

void imgui_set_tooltip(const char* text) {
    ImGui::SetTooltip("%s", text);
}

bool imgui_begin_tooltip(void) {
    ImGui::BeginTooltip();
    return true;
}

void imgui_end_tooltip(void) {
    ImGui::EndTooltip();
}

/* Popups */

void imgui_open_popup(const char* str_id) {
    ImGui::OpenPopup(str_id);
}

bool imgui_begin_popup(const char* str_id) {
    return ImGui::BeginPopup(str_id);
}

bool imgui_begin_popup_modal(const char* name, bool* p_open, int flags) {
    ImGuiWindowFlags imgui_flags = 0;
    if (flags & IMGUI_WINDOW_FLAG_NO_TITLE_BAR)         imgui_flags |= ImGuiWindowFlags_NoTitleBar;
    if (flags & IMGUI_WINDOW_FLAG_NO_RESIZE)            imgui_flags |= ImGuiWindowFlags_NoResize;
    if (flags & IMGUI_WINDOW_FLAG_NO_MOVE)              imgui_flags |= ImGuiWindowFlags_NoMove;

    return ImGui::BeginPopupModal(name, p_open, imgui_flags);
}

void imgui_end_popup(void) {
    ImGui::EndPopup();
}

void imgui_close_current_popup(void) {
    ImGui::CloseCurrentPopup();
}

/* Utilities */

bool imgui_is_item_hovered(void) {
    return ImGui::IsItemHovered();
}

bool imgui_is_item_active(void) {
    return ImGui::IsItemActive();
}

bool imgui_is_item_clicked(int mouse_button) {
    return ImGui::IsItemClicked(mouse_button);
}

void imgui_push_item_width(float item_width) {
    ImGui::PushItemWidth(item_width);
}

void imgui_pop_item_width(void) {
    ImGui::PopItemWidth();
}

/* Item rectangle queries */

void imgui_get_item_rect_min(float* out_x, float* out_y) {
    ImVec2 min = ImGui::GetItemRectMin();
    if (out_x) *out_x = min.x;
    if (out_y) *out_y = min.y;
}

void imgui_get_item_rect_max(float* out_x, float* out_y) {
    ImVec2 max = ImGui::GetItemRectMax();
    if (out_x) *out_x = max.x;
    if (out_y) *out_y = max.y;
}

void imgui_get_item_rect_size(float* out_x, float* out_y) {
    ImVec2 size = ImGui::GetItemRectSize();
    if (out_x) *out_x = size.x;
    if (out_y) *out_y = size.y;
}

/* Style */

void imgui_push_style_color(int idx, float r, float g, float b, float a) {
    ImGui::PushStyleColor(idx, ImVec4(r, g, b, a));
}

void imgui_pop_style_color(int count) {
    ImGui::PopStyleColor(count);
}

void imgui_push_style_var_float(int idx, float val) {
    ImGui::PushStyleVar(idx, val);
}

void imgui_push_style_var_vec2(int idx, float x, float y) {
    ImGui::PushStyleVar(idx, ImVec2(x, y));
}

void imgui_pop_style_var(int count) {
    ImGui::PopStyleVar(count);
}

/* Style getters */

void imgui_get_style_item_spacing(float* out_x, float* out_y) {
    ImGuiStyle& style = ImGui::GetStyle();
    if (out_x) *out_x = style.ItemSpacing.x;
    if (out_y) *out_y = style.ItemSpacing.y;
}

void imgui_get_style_item_inner_spacing(float* out_x, float* out_y) {
    ImGuiStyle& style = ImGui::GetStyle();
    if (out_x) *out_x = style.ItemInnerSpacing.x;
    if (out_y) *out_y = style.ItemInnerSpacing.y;
}

void imgui_get_style_frame_padding(float* out_x, float* out_y) {
    ImGuiStyle& style = ImGui::GetStyle();
    if (out_x) *out_x = style.FramePadding.x;
    if (out_y) *out_y = style.FramePadding.y;
}

void imgui_get_style_window_padding(float* out_x, float* out_y) {
    ImGuiStyle& style = ImGui::GetStyle();
    if (out_x) *out_x = style.WindowPadding.x;
    if (out_y) *out_y = style.WindowPadding.y;
}

float imgui_get_style_indent_spacing(void) {
    return ImGui::GetStyle().IndentSpacing;
}

float imgui_get_style_scrollbar_size(void) {
    return ImGui::GetStyle().ScrollbarSize;
}

/* Demo/Debug */

void imgui_show_demo_window(bool* p_open) {
    ImGui::ShowDemoWindow(p_open);
}

/* Input state queries */

bool imgui_want_capture_mouse(void) {
    return ImGui::GetIO().WantCaptureMouse;
}

bool imgui_want_capture_keyboard(void) {
    return ImGui::GetIO().WantCaptureKeyboard;
}

/* Draw list API */

void* imgui_get_window_draw_list(void) {
    return static_cast<void*>(ImGui::GetWindowDrawList());
}

void* imgui_get_background_draw_list(void) {
    return static_cast<void*>(ImGui::GetBackgroundDrawList());
}

void* imgui_get_foreground_draw_list(void) {
    return static_cast<void*>(ImGui::GetForegroundDrawList());
}

/* Draw list commands */

void imgui_draw_list_add_line(void* draw_list, float x1, float y1, float x2, float y2, unsigned int col, float thickness) {
    ImDrawList* dl = static_cast<ImDrawList*>(draw_list);
    dl->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), col, thickness);
}

void imgui_draw_list_add_rect(void* draw_list, float min_x, float min_y, float max_x, float max_y, unsigned int col, float rounding, int flags, float thickness) {
    ImDrawList* dl = static_cast<ImDrawList*>(draw_list);
    dl->AddRect(ImVec2(min_x, min_y), ImVec2(max_x, max_y), col, rounding, flags, thickness);
}

void imgui_draw_list_add_rect_filled(void* draw_list, float min_x, float min_y, float max_x, float max_y, unsigned int col, float rounding, int flags) {
    ImDrawList* dl = static_cast<ImDrawList*>(draw_list);
    dl->AddRectFilled(ImVec2(min_x, min_y), ImVec2(max_x, max_y), col, rounding, flags);
}

void imgui_draw_list_add_image(void* draw_list, void* texture_id, float min_x, float min_y, float max_x, float max_y, float uv0_x, float uv0_y, float uv1_x, float uv1_y, unsigned int tint_col) {
    ImDrawList* dl = static_cast<ImDrawList*>(draw_list);
    ImTextureID tex = reinterpret_cast<ImTextureID>(texture_id);
    dl->AddImage(tex, ImVec2(min_x, min_y), ImVec2(max_x, max_y), ImVec2(uv0_x, uv0_y), ImVec2(uv1_x, uv1_y), tint_col);
}

void imgui_draw_list_add_text(void* draw_list, float x, float y, unsigned int col, const char* text) {
    ImDrawList* dl = static_cast<ImDrawList*>(draw_list);
    dl->AddText(ImVec2(x, y), col, text);
}

void imgui_draw_list_add_image_9_slice(void* draw_list, void* texture_id,
                                       float min_x, float min_y, float max_x, float max_y,
                                       float border_left, float border_right,
                                       float border_top, float border_bottom,
                                       float texture_width, float texture_height,
                                       float uv0_x, float uv0_y, float uv1_x, float uv1_y,
                                       unsigned int tint_col) {
    ImDrawList* dl = static_cast<ImDrawList*>(draw_list);
    ImTextureID tex = reinterpret_cast<ImTextureID>(texture_id);

    /* Calculate the UV range for this sprite in the atlas */
    float uv_width = uv1_x - uv0_x;
    float uv_height = uv1_y - uv0_y;

    /* Calculate UV coordinates for borders (relative to sprite's UV range in atlas) */
    float uv_left = uv0_x + (border_left / texture_width) * uv_width;
    float uv_right = uv1_x - (border_right / texture_width) * uv_width;
    float uv_top = uv0_y + (border_top / texture_height) * uv_height;
    float uv_bottom = uv1_y - (border_bottom / texture_height) * uv_height;

    /* Calculate screen positions for the 9 slices */
    float x0 = min_x;
    float x1 = min_x + border_left;
    float x2 = max_x - border_right;
    float x3 = max_x;

    float y0 = min_y;
    float y1 = min_y + border_top;
    float y2 = max_y - border_bottom;
    float y3 = max_y;

    /* Draw the 9 slices using atlas UV coordinates:
     * +---+-------+---+
     * | 1 |   2   | 3 |
     * +---+-------+---+
     * | 4 |   5   | 6 |
     * +---+-------+---+
     * | 7 |   8   | 9 |
     * +---+-------+---+
     */

    /* Top-left corner (1) */
    dl->AddImage(tex, ImVec2(x0, y0), ImVec2(x1, y1),
                 ImVec2(uv0_x, uv0_y), ImVec2(uv_left, uv_top), tint_col);

    /* Top edge (2) */
    dl->AddImage(tex, ImVec2(x1, y0), ImVec2(x2, y1),
                 ImVec2(uv_left, uv0_y), ImVec2(uv_right, uv_top), tint_col);

    /* Top-right corner (3) */
    dl->AddImage(tex, ImVec2(x2, y0), ImVec2(x3, y1),
                 ImVec2(uv_right, uv0_y), ImVec2(uv1_x, uv_top), tint_col);

    /* Left edge (4) */
    dl->AddImage(tex, ImVec2(x0, y1), ImVec2(x1, y2),
                 ImVec2(uv0_x, uv_top), ImVec2(uv_left, uv_bottom), tint_col);

    /* Center (5) */
    dl->AddImage(tex, ImVec2(x1, y1), ImVec2(x2, y2),
                 ImVec2(uv_left, uv_top), ImVec2(uv_right, uv_bottom), tint_col);

    /* Right edge (6) */
    dl->AddImage(tex, ImVec2(x2, y1), ImVec2(x3, y2),
                 ImVec2(uv_right, uv_top), ImVec2(uv1_x, uv_bottom), tint_col);

    /* Bottom-left corner (7) */
    dl->AddImage(tex, ImVec2(x0, y2), ImVec2(x1, y3),
                 ImVec2(uv0_x, uv_bottom), ImVec2(uv_left, uv1_y), tint_col);

    /* Bottom edge (8) */
    dl->AddImage(tex, ImVec2(x1, y2), ImVec2(x2, y3),
                 ImVec2(uv_left, uv_bottom), ImVec2(uv_right, uv1_y), tint_col);

    /* Bottom-right corner (9) */
    dl->AddImage(tex, ImVec2(x2, y2), ImVec2(x3, y3),
                 ImVec2(uv_right, uv_bottom), ImVec2(uv1_x, uv1_y), tint_col);
}

/* Color helper */

unsigned int imgui_color_convert_float4_to_u32(float r, float g, float b, float a) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, a));
}

void imgui_text_formatted(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    ImGui::Text(buffer);
}
