//
// Created by james on 12/15/2025.
//

#include "ui.h"

#include "hotbar.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include "inventory.hpp"
#include "option_window.hpp"
#include "game/game_ui.h"

static void start_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1280, 720);
    io.DisplayFramebufferScale = ImVec2(1, 1);
    ImGui::NewFrame();
}

static void render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


EXTERN_C_BEGIN
#include <stddef.h>

FontSizes font_sizes = {0};
FontSizes font_sizes_bold = {0};

void ui_init(void *sdl_window, void *gl_context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
    io.FontGlobalScale = 1.0F;

    /* Setup Dear ImGui style */
    ImGui::StyleColorsDark();

    /* Setup platform/renderer backends */
    ImGui_ImplSDL2_InitForOpenGL(static_cast<SDL_Window *>(sdl_window), gl_context);
    ImGui_ImplOpenGL3_Init("#version 410 core");
}

void ui_shutdown(void) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void ui_render(void) {
    start_frame();

    spell_hud();

    if (game_ui_state.show_options) {
    	options_window_render();
    }

    inventory_render();

    render();
}

EXTERN_C_END
