# Dear ImGui Integration

This directory contains a **pure C wrapper** for Dear ImGui, keeping all C++ code isolated from the main codebase.

## Architecture

```
ui/imgui/
├── imgui/                 # Dear ImGui library (git submodule)
├── imgui_wrapper.h        # Pure C API (include this from your C code)
├── imgui_wrapper.cpp      # C++ implementation (only C++ file)
└── README.md              # This file
```

## Key Features

- ✅ **Pure C API** - Your C code never touches C++
- ✅ **Complete isolation** - All C++ code is in `imgui_wrapper.cpp`
- ✅ **SDL2 + OpenGL 3.3** - Integrated with existing rendering pipeline
- ✅ **Easy to use** - Simple C functions, no templates or classes to worry about

## How to Use

### 1. Include the header in your C code

```c
#include "ui/imgui/imgui_wrapper.h"
```

### 2. Initialize ImGui (once at startup)

```c
/* In your initialization code, after creating SDL window and GL context */
imgui_init(app.window, app.gl_context);
```

### 3. Frame Loop

In your main render loop:

```c
/* Process SDL events */
SDL_Event event;
while (SDL_PollEvent(&event)) {
    /* Let ImGui process the event first */
    int imgui_captured = imgui_process_event(&event);

    /* If ImGui captured the event, don't pass to your game */
    if (!imgui_captured) {
        /* Your normal event handling */
    }
}

/* Start ImGui frame */
imgui_new_frame();

/* Create your UI here */
if (imgui_begin("My Window", NULL, IMGUI_WINDOW_FLAG_NONE)) {
    imgui_text("Hello from C!");

    if (imgui_button("Click me")) {
        /* Button was clicked */
    }

    imgui_end();
}

/* After all your ImGui calls, render it */
imgui_render();

/* Then do your normal OpenGL rendering */
/* Finally, swap buffers */
SDL_GL_SwapWindow(app.window);
```

### 4. Cleanup (at shutdown)

```c
imgui_shutdown();
```

## Example Usage

See `ui/imgui_demo.c` for a complete example showing various widgets and features.

To add the demo to your build, add it to `CMakeLists.txt`:

```cmake
add_executable(TheLastGate WIN32
    # ... other files ...
    ui/imgui_demo.c
)
```

Then call `imgui_demo_render()` from your main loop.

## Available Functions

The C wrapper provides functions for:

- **Windows**: `imgui_begin()`, `imgui_end()`
- **Text**: `imgui_text()`, `imgui_text_colored()`, `imgui_bullet_text()`
- **Buttons**: `imgui_button()`, `imgui_checkbox()`, `imgui_radio_button()`
- **Input**: `imgui_input_text()`, `imgui_input_int()`, `imgui_input_float()`
- **Sliders**: `imgui_slider_int()`, `imgui_slider_float()`
- **Layout**: `imgui_separator()`, `imgui_same_line()`, `imgui_spacing()`
- **Menus**: `imgui_begin_menu_bar()`, `imgui_menu_item()`
- **Tooltips**: `imgui_set_tooltip()`, `imgui_is_item_hovered()`
- **Colors**: `imgui_color_edit3()`, `imgui_color_edit4()`
- **And many more...**

See `imgui_wrapper.h` for the complete API.

## Input Handling

ImGui can capture mouse and keyboard input. Use these functions to check if ImGui wants input:

```c
if (imgui_want_capture_mouse()) {
    /* Don't pass mouse events to your game */
}

if (imgui_want_capture_keyboard()) {
    /* Don't pass keyboard events to your game */
}
```

Or use `imgui_process_event()` which handles this automatically.

## Adding More Functions

If you need ImGui functions not in the wrapper:

1. Add the C declaration to `imgui_wrapper.h`
2. Add the C++ implementation to `imgui_wrapper.cpp`
3. Follow the existing patterns for type conversion (bool ↔ int, etc.)

## Building

The build system automatically:
- Compiles `imgui_wrapper.cpp` as C++
- Compiles all ImGui source files as C++
- Links everything into a static library
- Keeps the rest of your project as pure C

No special build steps needed - just run CMake as normal.

## Notes

- ImGui state is global - you can call functions from anywhere after initialization
- Window flags are bitfields - combine with `|` operator
- All ImGui functions are thread-safe within a single frame
- The wrapper converts between C types (int) and C++ types (bool) automatically
