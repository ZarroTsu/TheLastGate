/*
 * imgui_wrapper.h
 *
 * Pure C interface for Dear ImGui
 * This wrapper isolates all C++ code and provides a clean C API
 */

#ifndef IMGUI_WRAPPER_H
#define IMGUI_WRAPPER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialization and lifecycle */
void imgui_init(void* sdl_window, void* gl_context);
void imgui_shutdown(void);
void imgui_new_frame(float scale_x, float scale_y);
void imgui_render(void);

/* Font management - call after imgui_init() but before first frame */
void* imgui_add_font_from_file_ttf(const char* filename, float size_pixels);
void* imgui_add_font_from_file_ttf_pixel_perfect(const char* filename, float size_pixels);
void imgui_set_default_font(void* font);
void imgui_push_font(void* font);
void imgui_pop_font(void);
bool imgui_is_freetype_enabled(void);  /* Returns true if FreeType is compiled in */

/* FreeType rasterizer flags (only used if IMGUI_ENABLE_FREETYPE is defined) */
#define IMGUI_FREETYPE_NO_HINTING           (1 << 0)  /* Disable hinting (smoother but less sharp) */
#define IMGUI_FREETYPE_NO_AUTO_HINT         (1 << 1)  /* Disable auto-hinter */
#define IMGUI_FREETYPE_FORCE_AUTO_HINT      (1 << 2)  /* Force auto-hinter over font's native hinter */
#define IMGUI_FREETYPE_LIGHT_HINTING        (1 << 3)  /* Lighter hinting for non-monochrome modes */
#define IMGUI_FREETYPE_MONO_HINTING         (1 << 4)  /* Hinting for monochrome output */
#define IMGUI_FREETYPE_BOLD                 (1 << 5)  /* Make glyphs bold (embolden) */
#define IMGUI_FREETYPE_OBLIQUE              (1 << 6)  /* Make glyphs italic/oblique */
#define IMGUI_FREETYPE_MONOCHROME           (1 << 7)  /* Disable anti-aliasing (1-bit per pixel) */
#define IMGUI_FREETYPE_LOAD_COLOR           (1 << 8)  /* Load color glyphs/emojis */
#define IMGUI_FREETYPE_BITMAP               (1 << 9)  /* Use embedded bitmap strikes if available */

/* Display scaling - call this to set virtual resolution for ImGui */
void imgui_set_display_size(float width, float height);
void imgui_set_display_framebuffer_scale(float scale_x, float scale_y);

/* Process SDL events - call this in your event loop */
/* Returns true if ImGui wants to capture the event, false otherwise */
bool imgui_process_event(void* sdl_event);

/* Window management */
void imgui_set_next_windows_size(float width, float height);
void imgui_set_next_window_pos(float x, float y);
void imgui_set_next_window_pos_ex(float x, float y, int cond);
void imgui_set_next_window_size_ex(float width, float height, int cond);
bool imgui_begin(const char* name, bool* p_open, int flags);
void imgui_end(void);

/* Condition flags for SetNextWindow* functions */
#define IMGUI_COND_NONE             0
#define IMGUI_COND_ALWAYS           (1 << 0)
#define IMGUI_COND_ONCE             (1 << 1)
#define IMGUI_COND_FIRST_USE_EVER   (1 << 2)
#define IMGUI_COND_APPEARING        (1 << 3)

/* Child windows - scrollable regions */
bool imgui_begin_child(const char* str_id, float width, float height, bool border, int flags);
void imgui_end_child(void);

/* Window flags (bitfield) - mapped to ImGuiWindowFlags */
#define IMGUI_WINDOW_FLAG_NONE                  0
#define IMGUI_WINDOW_FLAG_NO_TITLE_BAR          (1 << 0)
#define IMGUI_WINDOW_FLAG_NO_RESIZE             (1 << 1)
#define IMGUI_WINDOW_FLAG_NO_MOVE               (1 << 2)
#define IMGUI_WINDOW_FLAG_NO_SCROLLBAR          (1 << 3)
#define IMGUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE  (1 << 4)
#define IMGUI_WINDOW_FLAG_NO_COLLAPSE           (1 << 5)
#define IMGUI_WINDOW_FLAG_ALWAYS_AUTO_RESIZE    (1 << 6)
#define IMGUI_WINDOW_FLAG_NO_BACKGROUND         (1 << 7)
#define IMGUI_WINDOW_FLAG_NO_SAVED_SETTINGS     (1 << 8)
#define IMGUI_WINDOW_FLAG_NO_MOUSE_INPUTS       (1 << 9)
#define IMGUI_WINDOW_FLAG_MENU_BAR              (1 << 10)
#define IMGUI_WINDOW_FLAG_HORIZONTAL_SCROLLBAR  (1 << 11)
#define IMGUI_WINDOW_FLAG_NO_FOCUS_ON_APPEARING (1 << 12)
#define IMGUI_WINDOW_FLAG_NO_BRING_TO_FRONT_ON_FOCUS (1 << 13)
#define IMGUI_WINDOW_FLAG_ALWAYS_VERTICAL_SCROLLBAR (1 << 14)
#define IMGUI_WINDOW_FLAG_ALWAYS_HORIZONTAL_SCROLLBAR (1 << 15)
#define IMGUI_WINDOW_FLAG_NO_NAV_INPUTS         (1 << 18)
#define IMGUI_WINDOW_FLAG_NO_NAV_FOCUS          (1 << 19)
#define IMGUI_WINDOW_FLAG_UNSAVED_DOCUMENT      (1 << 20)
#define IMGUI_WINDOW_FLAG_NO_DOCKING            (1 << 21)
/* Combination flags */
#define IMGUI_WINDOW_FLAG_NO_NAV                (IMGUI_WINDOW_FLAG_NO_NAV_INPUTS | IMGUI_WINDOW_FLAG_NO_NAV_FOCUS)
#define IMGUI_WINDOW_FLAG_NO_DECORATION         (IMGUI_WINDOW_FLAG_NO_TITLE_BAR | IMGUI_WINDOW_FLAG_NO_RESIZE | IMGUI_WINDOW_FLAG_NO_SCROLLBAR | IMGUI_WINDOW_FLAG_NO_COLLAPSE)
#define IMGUI_WINDOW_FLAG_NO_INPUTS             (IMGUI_WINDOW_FLAG_NO_MOUSE_INPUTS | IMGUI_WINDOW_FLAG_NO_NAV_INPUTS | IMGUI_WINDOW_FLAG_NO_NAV_FOCUS)


/* Text and labels */
void imgui_text(const char* text);
void imgui_text_formatted(const char* format, ...);
void imgui_text_colored(float r, float g, float b, float a, const char* text);
void imgui_text_colored_rgb(float r, float g, float b, float a, const char* text);
void imgui_text_colored_32(unsigned int col, const char* text);
void imgui_text_disabled(const char* text);
void imgui_text_wrapped(const char* text);
void imgui_text_wrapped_colored_32(unsigned int col, const char* text);
void imgui_label_text(const char* label, const char* text);
void imgui_bullet_text(const char* text);

/* Text utilities */
void imgui_calc_text_size(float* out_width, float* out_height, const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width);
void imgui_calc_text_size_simple(float* out_width, float* out_height, const char* text);
void imgui_push_text_wrap_pos(float wrap_local_pos_x);
void imgui_pop_text_wrap_pos(void);
void imgui_set_font_global_scale(float scale);

/* Widgets: Main */
bool imgui_button(const char* label);
bool imgui_button_sized(const char* label, float width, float height);
bool imgui_small_button(const char* label);
bool imgui_invisible_button(const char* str_id, float width, float height);
bool imgui_checkbox(const char* label, bool* v);
bool imgui_radio_button(const char* label, bool active);

/* Widgets: Input */
bool imgui_input_text(const char* label, char* buf, int buf_size);
bool imgui_input_text_area(const char* label, char* buf, int buf_size, float width, float height);
bool imgui_input_int(const char* label, int* v);
bool imgui_input_float(const char* label, float* v);
bool imgui_input_password(const char* label, char* buf, int buf_size);

/* Widgets: Sliders */
bool imgui_slider_int(const char* label, int* v, int v_min, int v_max);
bool imgui_slider_float(const char* label, float* v, float v_min, float v_max);
bool imgui_vslider_int(const char* label, float width, float height, int* v, int v_min, int v_max);
bool imgui_vslider_float(const char* label, float width, float height, float* v, float v_min, float v_max);

/* Widgets: Color */
bool imgui_color_edit3(const char* label, float col[3]);
bool imgui_color_edit4(const char* label, float col[4]);

/* Layout */
void imgui_separator(void);
void imgui_same_line(float offset_from_start_x, float spacing);
void imgui_same_line_gap(void);
void imgui_new_line(void);
void imgui_spacing(void);
void imgui_dummy(float width, float height);
void imgui_indent(float indent_w);
void imgui_unindent(float indent_w);

/* Grouping */
void imgui_begin_group(void);
void imgui_end_group(void);

/* Cursor/Layout positioning */
void imgui_set_cursor_pos(float x, float y);
void imgui_set_cursor_pos_x(float x);
void imgui_set_cursor_pos_y(float y);
float imgui_get_cursor_pos_x(void);
float imgui_get_cursor_pos_y(void);
float imgui_get_window_pos_x(void);
float imgui_get_window_pos_y(void);
void imgui_align_text_to_frame_padding(void);

/* Centering helper - call before the item you want to center */
void imgui_center_next_item(float item_width);
void imgui_center_next_text(const char* text);

/* Layout metrics */
float imgui_get_frame_height(void);
float imgui_get_frame_height_with_spacing(void);
void imgui_get_content_region_avail(float* out_width, float* out_height);

/* Scrolling */
float imgui_get_scroll_x(void);
float imgui_get_scroll_y(void);
float imgui_get_scroll_max_x(void);
float imgui_get_scroll_max_y(void);
void imgui_set_scroll_x(float scroll_x);
void imgui_set_scroll_y(float scroll_y);
void imgui_set_scroll_here_x(float center_x_ratio);
void imgui_set_scroll_here_y(float center_y_ratio);

/* Columns (legacy) */
void imgui_columns(int count, const char* id, int border);
void imgui_set_column_width(int index, float width);
void imgui_next_column(void);
int imgui_get_column_index(void);

/* Tables (modern, preferred over Columns) */
bool imgui_begin_table(const char* str_id, int column_count, int flags);
void imgui_end_table(void);
void imgui_table_next_row(int row_flags, float min_row_height);
bool imgui_table_next_column(void);
bool imgui_table_set_column_index(int column_n);
void imgui_table_setup_column(const char* label, int flags, float init_width_or_weight, unsigned int user_id);
void imgui_table_headers_row(void);
void imgui_table_header(const char* label);

/* Table flags */
#define IMGUI_TABLE_FLAG_NONE                       0
#define IMGUI_TABLE_FLAG_RESIZABLE                  (1 << 0)
#define IMGUI_TABLE_FLAG_REORDERABLE                (1 << 1)
#define IMGUI_TABLE_FLAG_HIDEABLE                   (1 << 2)
#define IMGUI_TABLE_FLAG_SORTABLE                   (1 << 3)
#define IMGUI_TABLE_FLAG_NO_SAVED_SETTINGS          (1 << 4)
#define IMGUI_TABLE_FLAG_CONTEXT_MENU_IN_BODY       (1 << 5)
#define IMGUI_TABLE_FLAG_ROW_BG                     (1 << 6)
#define IMGUI_TABLE_FLAG_BORDERS_INNER_H            (1 << 7)
#define IMGUI_TABLE_FLAG_BORDERS_OUTER_H            (1 << 8)
#define IMGUI_TABLE_FLAG_BORDERS_INNER_V            (1 << 9)
#define IMGUI_TABLE_FLAG_BORDERS_OUTER_V            (1 << 10)
#define IMGUI_TABLE_FLAG_BORDERS_H                  (IMGUI_TABLE_FLAG_BORDERS_INNER_H | IMGUI_TABLE_FLAG_BORDERS_OUTER_H)
#define IMGUI_TABLE_FLAG_BORDERS_V                  (IMGUI_TABLE_FLAG_BORDERS_INNER_V | IMGUI_TABLE_FLAG_BORDERS_OUTER_V)
#define IMGUI_TABLE_FLAG_BORDERS_INNER              (IMGUI_TABLE_FLAG_BORDERS_INNER_V | IMGUI_TABLE_FLAG_BORDERS_INNER_H)
#define IMGUI_TABLE_FLAG_BORDERS_OUTER              (IMGUI_TABLE_FLAG_BORDERS_OUTER_V | IMGUI_TABLE_FLAG_BORDERS_OUTER_H)
#define IMGUI_TABLE_FLAG_BORDERS                    (IMGUI_TABLE_FLAG_BORDERS_INNER | IMGUI_TABLE_FLAG_BORDERS_OUTER)
#define IMGUI_TABLE_FLAG_NO_BORDERS_IN_BODY         (1 << 11)
#define IMGUI_TABLE_FLAG_NO_BORDERS_IN_BODY_UNTIL_RESIZE (1 << 12)
#define IMGUI_TABLE_FLAG_SIZING_FIXED_FIT           (1 << 13)
#define IMGUI_TABLE_FLAG_SIZING_FIXED_SAME          (2 << 13)
#define IMGUI_TABLE_FLAG_SIZING_STRETCH_PROP        (3 << 13)
#define IMGUI_TABLE_FLAG_SIZING_STRETCH_SAME        (4 << 13)
#define IMGUI_TABLE_FLAG_NO_HOST_EXTEND_X           (1 << 16)
#define IMGUI_TABLE_FLAG_NO_HOST_EXTEND_Y           (1 << 17)
#define IMGUI_TABLE_FLAG_NO_KEEP_COLUMNS_VISIBLE    (1 << 18)
#define IMGUI_TABLE_FLAG_PRECISE_WIDTHS             (1 << 19)
#define IMGUI_TABLE_FLAG_NO_CLIP                    (1 << 20)
#define IMGUI_TABLE_FLAG_PAD_OUTER_X                (1 << 21)
#define IMGUI_TABLE_FLAG_NO_PAD_OUTER_X             (1 << 22)
#define IMGUI_TABLE_FLAG_NO_PAD_INNER_X             (1 << 23)
#define IMGUI_TABLE_FLAG_SCROLL_X                   (1 << 24)
#define IMGUI_TABLE_FLAG_SCROLL_Y                   (1 << 25)
#define IMGUI_TABLE_FLAG_SORT_MULTI                 (1 << 26)
#define IMGUI_TABLE_FLAG_SORT_TRISTATE              (1 << 27)
#define IMGUI_TABLE_FLAG_SIZING_MASK                (IMGUI_TABLE_FLAG_SIZING_FIXED_FIT | IMGUI_TABLE_FLAG_SIZING_FIXED_SAME | IMGUI_TABLE_FLAG_SIZING_STRETCH_PROP | IMGUI_TABLE_FLAG_SIZING_STRETCH_SAME)

/* Table row flags */
#define IMGUI_TABLE_ROW_FLAG_NONE                   0
#define IMGUI_TABLE_ROW_FLAG_HEADERS                (1 << 0)

/* Table column flags */
#define IMGUI_TABLE_COLUMN_FLAG_NONE                0
#define IMGUI_TABLE_COLUMN_FLAG_DISABLED            (1 << 0)
#define IMGUI_TABLE_COLUMN_FLAG_DEFAULT_HIDE        (1 << 1)
#define IMGUI_TABLE_COLUMN_FLAG_DEFAULT_SORT        (1 << 2)
#define IMGUI_TABLE_COLUMN_FLAG_WIDTH_STRETCH       (1 << 3)
#define IMGUI_TABLE_COLUMN_FLAG_WIDTH_FIXED         (1 << 4)
#define IMGUI_TABLE_COLUMN_FLAG_NO_RESIZE           (1 << 5)
#define IMGUI_TABLE_COLUMN_FLAG_NO_REORDER          (1 << 6)
#define IMGUI_TABLE_COLUMN_FLAG_NO_HIDE             (1 << 7)
#define IMGUI_TABLE_COLUMN_FLAG_NO_CLIP             (1 << 8)
#define IMGUI_TABLE_COLUMN_FLAG_NO_SORT             (1 << 9)
#define IMGUI_TABLE_COLUMN_FLAG_NO_SORT_ASCENDING   (1 << 10)
#define IMGUI_TABLE_COLUMN_FLAG_NO_SORT_DESCENDING  (1 << 11)
#define IMGUI_TABLE_COLUMN_FLAG_NO_HEADER_LABEL     (1 << 12)
#define IMGUI_TABLE_COLUMN_FLAG_NO_HEADER_WIDTH     (1 << 13)
#define IMGUI_TABLE_COLUMN_FLAG_PREFER_SORT_ASCENDING   (1 << 14)
#define IMGUI_TABLE_COLUMN_FLAG_PREFER_SORT_DESCENDING  (1 << 15)
#define IMGUI_TABLE_COLUMN_FLAG_INDENT_ENABLE       (1 << 16)
#define IMGUI_TABLE_COLUMN_FLAG_INDENT_DISABLE      (1 << 17)
#define IMGUI_TABLE_COLUMN_FLAG_IS_ENABLED          (1 << 24)
#define IMGUI_TABLE_COLUMN_FLAG_IS_VISIBLE          (1 << 25)
#define IMGUI_TABLE_COLUMN_FLAG_IS_SORTED           (1 << 26)
#define IMGUI_TABLE_COLUMN_FLAG_IS_HOVERED          (1 << 27)

/* Tree/Collapsing */
bool imgui_tree_node(const char* label);
void imgui_tree_pop(void);
bool imgui_collapsing_header(const char* label, int flags);

/* Tab bars */
bool imgui_begin_tab_bar(const char* str_id, int flags);
void imgui_end_tab_bar(void);
bool imgui_begin_tab_item(const char* label, bool* p_open, int flags);
void imgui_end_tab_item(void);
bool imgui_tab_item_button(const char* label, int flags);
void imgui_set_tab_item_closed(const char* tab_or_docked_window_label);

/* Tab bar flags */
#define IMGUI_TAB_BAR_FLAG_NONE                           0
#define IMGUI_TAB_BAR_FLAG_REORDERABLE                    (1 << 0)
#define IMGUI_TAB_BAR_FLAG_AUTO_SELECT_NEW_TABS           (1 << 1)
#define IMGUI_TAB_BAR_FLAG_TAB_LIST_POPUP_BUTTON          (1 << 2)
#define IMGUI_TAB_BAR_FLAG_NO_CLOSE_WITH_MIDDLE_MOUSE_BUTTON (1 << 3)
#define IMGUI_TAB_BAR_FLAG_NO_TAB_LIST_SCROLLING_BUTTONS  (1 << 4)
#define IMGUI_TAB_BAR_FLAG_NO_TOOLTIP                     (1 << 5)
#define IMGUI_TAB_BAR_FLAG_FITTING_POLICY_RESIZE_DOWN     (1 << 6)
#define IMGUI_TAB_BAR_FLAG_FITTING_POLICY_SCROLL          (1 << 7)
#define IMGUI_TAB_BAR_FLAG_FITTING_POLICY_MASK            (IMGUI_TAB_BAR_FLAG_FITTING_POLICY_RESIZE_DOWN | IMGUI_TAB_BAR_FLAG_FITTING_POLICY_SCROLL)
#define IMGUI_TAB_BAR_FLAG_FITTING_POLICY_DEFAULT         IMGUI_TAB_BAR_FLAG_FITTING_POLICY_RESIZE_DOWN

/* Tab item flags */
#define IMGUI_TAB_ITEM_FLAG_NONE                          0
#define IMGUI_TAB_ITEM_FLAG_UNSAVED_DOCUMENT              (1 << 0)
#define IMGUI_TAB_ITEM_FLAG_SET_SELECTED                  (1 << 1)
#define IMGUI_TAB_ITEM_FLAG_NO_CLOSE_WITH_MIDDLE_MOUSE_BUTTON (1 << 2)
#define IMGUI_TAB_ITEM_FLAG_NO_PUSH_ID                    (1 << 3)
#define IMGUI_TAB_ITEM_FLAG_NO_TOOLTIP                    (1 << 4)
#define IMGUI_TAB_ITEM_FLAG_NO_REORDER                    (1 << 5)
#define IMGUI_TAB_ITEM_FLAG_LEADING                       (1 << 6)
#define IMGUI_TAB_ITEM_FLAG_TRAILING                      (1 << 7)

/* Selectables */
bool imgui_selectable(const char* label, bool selected);

/* Combo box */
bool imgui_begin_combo(const char* label, const char* preview_value, int flags);
void imgui_end_combo(void);

/* Combo flags */
#define IMGUI_COMBO_FLAG_NONE                   0
#define IMGUI_COMBO_FLAG_POPUP_ALIGN_LEFT       (1 << 0)
#define IMGUI_COMBO_FLAG_HEIGHT_SMALL           (1 << 1)
#define IMGUI_COMBO_FLAG_HEIGHT_REGULAR         (1 << 2)
#define IMGUI_COMBO_FLAG_HEIGHT_LARGE           (1 << 3)
#define IMGUI_COMBO_FLAG_HEIGHT_LARGEST         (1 << 4)
#define IMGUI_COMBO_FLAG_NO_ARROW_BUTTON        (1 << 5)
#define IMGUI_COMBO_FLAG_NO_PREVIEW             (1 << 6)
#define IMGUI_COMBO_FLAG_WIDTH_FIT_PREVIEW      (1 << 7)
#define IMGUI_COMBO_FLAG_HEIGHT_MASK            (IMGUI_COMBO_FLAG_HEIGHT_SMALL | IMGUI_COMBO_FLAG_HEIGHT_REGULAR | IMGUI_COMBO_FLAG_HEIGHT_LARGE | IMGUI_COMBO_FLAG_HEIGHT_LARGEST)

/* List box */
bool imgui_begin_list_box(const char* label);
void imgui_end_list_box(void);

/* Menus */
bool imgui_begin_main_menu_bar(void);
void imgui_end_main_menu_bar(void);
bool imgui_begin_menu_bar(void);
void imgui_end_menu_bar(void);
bool imgui_begin_menu(const char* label, bool enabled);
void imgui_end_menu(void);
bool imgui_menu_item(const char* label, const char* shortcut, bool selected, bool enabled);

/* Tooltips */
void imgui_set_tooltip(const char* text);
bool imgui_begin_tooltip(void);
void imgui_end_tooltip(void);

/* Popups */
void imgui_open_popup(const char* str_id);
bool imgui_begin_popup(const char* str_id);
bool imgui_begin_popup_modal(const char* name, bool* p_open, int flags);
void imgui_end_popup(void);
void imgui_close_current_popup(void);

/* ID stack - for distinguishing widgets with same label */
void imgui_push_id_str(const char* str_id);
void imgui_push_id_int(int int_id);
void imgui_push_id_ptr(const void* ptr_id);
void imgui_pop_id(void);

/* Disabling widgets */
void imgui_begin_disabled(bool disabled);
void imgui_end_disabled(void);

/* Utilities */
bool imgui_is_item_hovered(void);
bool imgui_is_item_active(void);
bool imgui_is_item_clicked(int mouse_button);
bool imgui_is_mouse_clicked(int mouse_button);
bool imgui_is_window_hovered(void);
void imgui_push_item_width(float item_width);
void imgui_pop_item_width(void);
void imgui_set_item_default_focus(void);

/* Item rectangle queries - get bounding box of last item */
void imgui_get_item_rect_min(float* out_x, float* out_y);
void imgui_get_item_rect_max(float* out_x, float* out_y);
void imgui_get_item_rect_size(float* out_x, float* out_y);

/* Style */
void imgui_push_style_color_32(int idx, unsigned int col);
void imgui_push_style_color(int idx, float r, float g, float b, float a);
void imgui_pop_style_color(int count);
void imgui_push_style_var_float(int idx, float val);
void imgui_push_style_var_vec2(int idx, float x, float y);
void imgui_pop_style_var(int count);

/* Style variable indices (ImGuiStyleVar_) */
#define IMGUI_STYLE_VAR_ALPHA                   0   /* float */
#define IMGUI_STYLE_VAR_DISABLED_ALPHA          1   /* float */
#define IMGUI_STYLE_VAR_WINDOW_PADDING          2   /* ImVec2 */
#define IMGUI_STYLE_VAR_WINDOW_ROUNDING         3   /* float */
#define IMGUI_STYLE_VAR_WINDOW_BORDER_SIZE      4   /* float */
#define IMGUI_STYLE_VAR_WINDOW_MIN_SIZE         5   /* ImVec2 */
#define IMGUI_STYLE_VAR_WINDOW_TITLE_ALIGN      6   /* ImVec2 */
#define IMGUI_STYLE_VAR_CHILD_ROUNDING          7   /* float */
#define IMGUI_STYLE_VAR_CHILD_BORDER_SIZE       8   /* float */
#define IMGUI_STYLE_VAR_POPUP_ROUNDING          9   /* float */
#define IMGUI_STYLE_VAR_POPUP_BORDER_SIZE       10  /* float */
#define IMGUI_STYLE_VAR_FRAME_PADDING           11  /* ImVec2 */
#define IMGUI_STYLE_VAR_FRAME_ROUNDING          12  /* float */
#define IMGUI_STYLE_VAR_FRAME_BORDER_SIZE       13  /* float */
#define IMGUI_STYLE_VAR_ITEM_SPACING            14  /* ImVec2 */
#define IMGUI_STYLE_VAR_ITEM_INNER_SPACING      15  /* ImVec2 */
#define IMGUI_STYLE_VAR_INDENT_SPACING          16  /* float */
#define IMGUI_STYLE_VAR_CELL_PADDING            17  /* ImVec2 */
#define IMGUI_STYLE_VAR_SCROLLBAR_SIZE          18  /* float */
#define IMGUI_STYLE_VAR_SCROLLBAR_ROUNDING      19  /* float */
#define IMGUI_STYLE_VAR_SCROLLBAR_PADDING       20  /* ImVec2 */
#define IMGUI_STYLE_VAR_GRAB_MIN_SIZE           21  /* float */
#define IMGUI_STYLE_VAR_GRAB_ROUNDING           22  /* float */
#define IMGUI_STYLE_VAR_TAB_ROUNDING            23  /* float */
#define IMGUI_STYLE_VAR_BUTTON_TEXT_ALIGN       24  /* ImVec2 */
#define IMGUI_STYLE_VAR_SELECTABLE_TEXT_ALIGN   25  /* ImVec2 */

/* Style getters - get current style values */
void imgui_get_style_item_spacing(float* out_x, float* out_y);
void imgui_get_style_item_inner_spacing(float* out_x, float* out_y);
void imgui_get_style_frame_padding(float* out_x, float* out_y);
void imgui_get_style_window_padding(float* out_x, float* out_y);
float imgui_get_style_indent_spacing(void);
float imgui_get_style_scrollbar_size(void);

/* Style color indices (ImGuiCol_) */
#define IMGUI_COL_TEXT                      0
#define IMGUI_COL_TEXT_DISABLED             1
#define IMGUI_COL_WINDOW_BG                 2
#define IMGUI_COL_CHILD_BG                  3
#define IMGUI_COL_POPUP_BG                  4
#define IMGUI_COL_BORDER                    5
#define IMGUI_COL_BORDER_SHADOW             6
#define IMGUI_COL_FRAME_BG                  7
#define IMGUI_COL_FRAME_BG_HOVERED          8
#define IMGUI_COL_FRAME_BG_ACTIVE           9
#define IMGUI_COL_TITLE_BG                  10
#define IMGUI_COL_TITLE_BG_ACTIVE           11
#define IMGUI_COL_TITLE_BG_COLLAPSED        12
#define IMGUI_COL_MENU_BAR_BG               13
#define IMGUI_COL_SCROLLBAR_BG              14
#define IMGUI_COL_SCROLLBAR_GRAB            15
#define IMGUI_COL_SCROLLBAR_GRAB_HOVERED    16
#define IMGUI_COL_SCROLLBAR_GRAB_ACTIVE     17
#define IMGUI_COL_CHECK_MARK                18
#define IMGUI_COL_SLIDER_GRAB               19
#define IMGUI_COL_SLIDER_GRAB_ACTIVE        20
#define IMGUI_COL_BUTTON                    21
#define IMGUI_COL_BUTTON_HOVERED            22
#define IMGUI_COL_BUTTON_ACTIVE             23
#define IMGUI_COL_HEADER                    24
#define IMGUI_COL_HEADER_HOVERED            25
#define IMGUI_COL_HEADER_ACTIVE             26
#define IMGUI_COL_SEPARATOR                 27
#define IMGUI_COL_SEPARATOR_HOVERED         28
#define IMGUI_COL_SEPARATOR_ACTIVE          29
#define IMGUI_COL_RESIZE_GRIP               30
#define IMGUI_COL_RESIZE_GRIP_HOVERED       31
#define IMGUI_COL_RESIZE_GRIP_ACTIVE        32
#define IMGUI_COL_INPUT_TEXT_CURSOR         33
#define IMGUI_COL_TAB_HOVERED               34
#define IMGUI_COL_TAB                       35
#define IMGUI_COL_TAB_SELECTED              36
#define IMGUI_COL_TAB_SELECTED_OVERLINE     37
#define IMGUI_COL_TAB_DIMMED                38
#define IMGUI_COL_TAB_DIMMED_SELECTED       39
#define IMGUI_COL_TAB_DIMMED_SELECTED_OVERLINE 40
#define IMGUI_COL_PLOT_LINES                41
#define IMGUI_COL_PLOT_LINES_HOVERED        42
#define IMGUI_COL_PLOT_HISTOGRAM            43
#define IMGUI_COL_PLOT_HISTOGRAM_HOVERED    44
#define IMGUI_COL_TABLE_HEADER_BG           45
#define IMGUI_COL_TABLE_BORDER_STRONG       46
#define IMGUI_COL_TABLE_BORDER_LIGHT        47
#define IMGUI_COL_TABLE_ROW_BG              48
#define IMGUI_COL_TABLE_ROW_BG_ALT          49
#define IMGUI_COL_TEXT_LINK                 50
#define IMGUI_COL_TEXT_SELECTED_BG          51
#define IMGUI_COL_TREE_LINES                52
#define IMGUI_COL_DRAG_DROP_TARGET          53
#define IMGUI_COL_DRAG_DROP_TARGET_BG       54
#define IMGUI_COL_UNSAVED_MARKER            55
#define IMGUI_COL_NAV_CURSOR                56
#define IMGUI_COL_NAV_WINDOWING_HIGHLIGHT   57
#define IMGUI_COL_NAV_WINDOWING_DIM_BG      58
#define IMGUI_COL_MODAL_WINDOW_DIM_BG       59
/* Backwards compatibility aliases (renamed in newer ImGui versions) */
#define IMGUI_COL_TAB_ACTIVE                IMGUI_COL_TAB_SELECTED
#define IMGUI_COL_TAB_UNFOCUSED             IMGUI_COL_TAB_DIMMED
#define IMGUI_COL_TAB_UNFOCUSED_ACTIVE      IMGUI_COL_TAB_DIMMED_SELECTED
#define IMGUI_COL_NAV_HIGHLIGHT             IMGUI_COL_NAV_CURSOR

/* Demo/Debug */
void imgui_show_demo_window(bool* p_open);

/* Input state queries */
bool imgui_want_capture_mouse(void);
bool imgui_want_capture_keyboard(void);

/* Mouse input */
float imgui_get_mouse_wheel(void);
float imgui_get_mouse_wheel_h(void);

/* Keyboard input - for keybinding capture */
bool imgui_is_key_pressed(int sdl_keycode);
int imgui_get_key_mods(void);

/* Draw list API - for custom drawing */
void* imgui_get_window_draw_list(void);
void* imgui_get_background_draw_list(void);
void* imgui_get_foreground_draw_list(void);

/* Draw list commands - use the draw list returned from above functions */
void imgui_draw_list_reset_render_state(void *draw_list);
void imgui_draw_list_add_line(void* draw_list, float x1, float y1, float x2, float y2, unsigned int col, float thickness);
void imgui_draw_list_add_rect(void* draw_list, float min_x, float min_y, float max_x, float max_y, unsigned int col, float rounding, int flags, float thickness);
void imgui_draw_list_add_rect_filled(void* draw_list, float min_x, float min_y, float max_x, float max_y, unsigned int col, float rounding, int flags);
void imgui_draw_list_add_image(void* draw_list, void* texture_id, float min_x, float min_y, float max_x, float max_y, float uv0_x, float uv0_y, float uv1_x, float uv1_y, unsigned int tint_col);
void imgui_draw_list_add_text(void* draw_list, float x, float y, unsigned int col, const char* text);
void imgui_draw_list_add_text_outline(void* draw_list, float x, float y, unsigned int col, unsigned int outline_col, const char* text);

/* Draw list callback - for custom rendering within ImGui */
typedef void (*imgui_draw_callback_fn)(void* user_data);
void imgui_draw_list_add_callback(void* draw_list, imgui_draw_callback_fn callback, void* user_data);

/* 9-slice/9-patch image drawing - for scalable UI elements like buttons */
/* border_* are the pixel sizes of the borders in the source texture */
/* texture_width/height are the sprite dimensions in pixels */
/* uv0/uv1 are the atlas coordinates for the sprite */
void imgui_draw_list_add_image_9_slice(void* draw_list, void* texture_id,
                                       float min_x, float min_y, float max_x, float max_y,
                                       float border_left, float border_right,
                                       float border_top, float border_bottom,
                                       float texture_width, float texture_height,
                                       float uv0_x, float uv0_y, float uv1_x, float uv1_y,
                                       unsigned int tint_col);

/* Color helper - convert RGBA (0-255) to ImGui color (packed u32) */
unsigned int imgui_color_convert_float4_to_u32(float r, float g, float b, float a);

#ifdef __cplusplus
}
#endif

#endif /* IMGUI_WRAPPER_H */
