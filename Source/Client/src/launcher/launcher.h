#pragma once

#define LAUNCHER_BACKGROUND_SPRITE 18130
#include <stdbool.h>

static const unsigned int BACKGROUND_COLOR_32 = 0xFF050017;
static const unsigned int GOLD_COLOR_32 = 0xFF4AABCE;
static const unsigned int BEIGE_COLOR_32 = 0xFFAFD8E9;
static const unsigned int BEIGE_COLOR_HOVERED_32 = 0xFF71B6D8;
static const unsigned int BEIGE_COLOR_ACTIVE_32 = 0xFF509FCF;
static const unsigned int BUTTON_COLOR_32 = 0xFF4BABCE;
static const unsigned int BUTTON_COLOR_HOVERED_32 = 0xFF3693BC;
static const unsigned int BUTTON_COLOR_ACTIVE_32 = 0xFF2C74A2;
static const unsigned int BUTTON_COLOR_DISABLED_32 = 0xFF0D353E;
static const unsigned int BORDERS_32 = 0xFF446A75;
static const unsigned int HEADER_TEXT_COLOR_32 = 0xFFAB811E;
static const unsigned int DROPDOWN_SELECTED_32 = 0xFF7F434C;
static const unsigned int DROPDOWN_HIGHLIGHT_32 = 0xFF6F4046;
static const unsigned int DROPDOWN_ACTIVE_32 = 0x7F412529;
static const unsigned int INNER_WINDOW_32 = 0xFF210004;
static const unsigned int POPOVER_BACKGROUND_32 = 0XE1210003;
static const unsigned int CURRENT_CHARACTER_COLOR_32 = 0xFF6342D6;

typedef struct {
    bool show_first_confirmation;
    bool show_second_confirmation;
    char file_path[512];
    bool new_character;
} SwitchCharacterConfirmState;

void launcher_init(void);
void launcher_shutdown(void);
void launcher_render(void);

void launcher_set_fyi(const char* text);
