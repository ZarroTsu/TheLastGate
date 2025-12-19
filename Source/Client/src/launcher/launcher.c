#include "launcher.h"

#include <stddef.h>

#include "graphics/sdl.h"
#include "ui/imgui/imgui_wrapper.h"

static const int window_flags =
        IMGUI_WINDOW_FLAG_NO_COLLAPSE | IMGUI_WINDOW_FLAG_NO_RESIZE | IMGUI_WINDOW_FLAG_NO_MOVE |
        IMGUI_WINDOW_FLAG_NO_TITLE_BAR | IMGUI_WINDOW_FLAG_NO_SCROLLBAR | IMGUI_WINDOW_FLAG_NO_SCROLL_WITH_MOUSE;

void launcher_render(void) {
    imgui_set_next_window_pos(0, 0);
    imgui_set_next_windows_size(SCREEN_WIDTH, SCREEN_HEIGHT);
    sdl_load_sprite(LAUNCHER_BACKGROUND_SPRITE);
    const SpriteData *background_sprite_data = &sprite_data[LAUNCHER_BACKGROUND_SPRITE];


    if (imgui_begin("Launcher", NULL, window_flags)) {
        void *draw_list = imgui_get_window_draw_list();

        imgui_draw_list_add_image(draw_list, background_sprite_data->gl_texture, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                                  background_sprite_data->uv0.u, background_sprite_data->uv0.v,
                                  background_sprite_data->uv1.u, background_sprite_data->uv1.v,
                                  imgui_color_convert_float4_to_u32(1, 1, 1, 1));

        imgui_text("hey");
    }
    imgui_end();
}
