# Migrating the Options Dialog from Windows to SDL

## Current Windows Dialog Overview

The options dialog (`options.c`) is a **Windows native dialog** that handles:

1. **Character Creation/Login:**
   - Character name input
   - Character description
   - Password input
   - Race selection (Templar, Mercenary, Harakim)
   - Gender selection (Male, Female)
   - Save/Load character functionality

2. **Game Settings:**
   - Sound enable/disable
   - Shadow rendering enable/disable
   - Dark mode toggle

3. **Network Connection:**
   - Connects to game server via threading (`_beginthread`)
   - Shows connection status

## SDL Replacement Options

You have **three main approaches** to replace the Windows dialog with SDL:

---

## Option 1: Use SDL + ImGui (RECOMMENDED - Easiest)

**ImGui** (Immediate Mode GUI) is a popular, lightweight GUI library that works perfectly with SDL2.

### Pros:
- Very easy to use - create UI with simple function calls
- No XML/resource files needed
- Built-in widgets: buttons, text inputs, checkboxes, etc.
- Looks modern and customizable
- Widely used in game development
- Cross-platform

### Cons:
- Adds a dependency (but it's header-only, easy to integrate)
- Different look than native OS dialogs (but more consistent across platforms)

### Installation:

1. Download ImGui from: https://github.com/ocornut/imgui
2. Copy files to your project:
   - `imgui.cpp`, `imgui.h`
   - `imgui_draw.cpp`
   - `imgui_widgets.cpp`
   - `imgui_tables.cpp`
   - `imgui_impl_sdl2.cpp`, `imgui_impl_sdl2.h` (SDL2 backend)
   - `imgui_impl_sdlrenderer2.cpp`, `imgui_impl_sdlrenderer2.h` (SDL2 Renderer backend)

3. Add to your Makefile:
```makefile
IMGUI_SOURCES = imgui.cpp imgui_draw.cpp imgui_widgets.cpp imgui_tables.cpp \
                imgui_impl_sdl2.cpp imgui_impl_sdlrenderer2.cpp
LIBS += -lstdc++  # ImGui is C++, needs C++ linker
```

### Example Code:

```cpp
// In your main loop initialization (once):
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

// Setup ImGui context
IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGuiIO& io = ImGui::GetIO();

// Setup ImGui SDL2 binding
ImGui_ImplSDL2_InitForSDLRenderer(app.window, app.renderer);
ImGui_ImplSDLRenderer2_Init(app.renderer);

// In your options dialog function:
void show_options_dialog() {
    static char char_name[80] = "";
    static char char_desc[150] = "";
    static char password[15] = "";
    static bool enable_sound = true;
    static bool enable_shadows = true;
    static bool enable_darkmode = false;
    static int race = 0; // 0=Templar, 1=Mercenary, 2=Harakim
    static int gender = 0; // 0=Male, 1=Female

    // Start ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Create options window
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("The Last Gate - Options", NULL, ImGuiWindowFlags_NoCollapse);

    // Character Info Section
    ImGui::SeparatorText("Character Information");
    ImGui::InputText("Name", char_name, sizeof(char_name));
    ImGui::InputText("Description", char_desc, sizeof(char_desc));
    ImGui::InputText("Password", password, sizeof(password), ImGuiInputTextFlags_Password);

    // Race Selection
    ImGui::SeparatorText("Race");
    ImGui::RadioButton("Templar", &race, 0); ImGui::SameLine();
    ImGui::RadioButton("Mercenary", &race, 1); ImGui::SameLine();
    ImGui::RadioButton("Harakim", &race, 2);

    // Gender Selection
    ImGui::SeparatorText("Gender");
    ImGui::RadioButton("Male", &gender, 0); ImGui::SameLine();
    ImGui::RadioButton("Female", &gender, 1);

    // Game Settings
    ImGui::SeparatorText("Game Settings");
    ImGui::Checkbox("Enable Sound", &enable_sound);
    ImGui::Checkbox("Enable Shadows", &enable_shadows);
    ImGui::Checkbox("Dark Mode", &enable_darkmode);

    // Buttons
    ImGui::Spacing();
    if (ImGui::Button("New Character", ImVec2(120, 0))) {
        // Reset form
        strcpy(char_name, "");
        strcpy(char_desc, "");
        strcpy(password, "");
    }
    ImGui::SameLine();
    if (ImGui::Button("Load", ImVec2(80, 0))) {
        // Open file dialog (use nativefiledialog or tinyfiledialogs)
    }
    ImGui::SameLine();
    if (ImGui::Button("Save", ImVec2(80, 0))) {
        // Save character
    }

    ImGui::Spacing();
    if (ImGui::Button("Connect", ImVec2(120, 0))) {
        // Copy values to global variables
        strcpy(pdata.cname, char_name);
        strcpy(pdata.desc, char_desc);
        dosound = enable_sound;
        do_shadow = enable_shadows;
        do_darkmode = enable_darkmode;
        // Launch connection thread
    }
    ImGui::SameLine();
    if (ImGui::Button("Quit", ImVec2(80, 0))) {
        quit = 1;
    }

    ImGui::End();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}
```

---

## Option 2: Use Native File Dialogs Library + Custom SDL UI

For a **hybrid approach**, use native OS file dialogs for Load/Save, and build a custom SDL UI for the rest.

### Libraries for Native File Dialogs:

**tinyfiledialogs** (Recommended - Single C file):
```c
#include "tinyfiledialogs.h"

// Load dialog example:
char const* filters[2] = {"*.chr", "*.txt"};
char* filename = tinyfiledialogs_openFileDialog(
    "Load Character",      // Title
    "",                    // Default path
    2,                     // Number of filters
    filters,               // Filter patterns
    "Character Files",     // Filter description
    0                      // Allow multiple select (0=no)
);

if (filename) {
    load_char(hwnd, filename);
}
```

Download: https://sourceforge.net/projects/tinyfiledialogs/

**nativefiledialog** (More features):
- GitHub: https://github.com/mlabbe/nativefiledialog
- Supports file/folder browsing, multiple selection
- Cross-platform (Windows, Mac, Linux)

### Custom SDL UI:

For the character creation form, you'd need to implement:

1. **Text Input Fields:**
   - Capture SDL_TEXTINPUT events
   - Render text with your existing font system
   - Handle cursor position, backspace, etc.

2. **Checkboxes/Radio Buttons:**
   - Draw using `sdl_showbox()` and sprites
   - Handle mouse clicks to toggle state

3. **Buttons:**
   - Draw using sprites or rectangles
   - Handle hover and click states

**Example Custom Button:**
```c
typedef struct {
    int x, y, w, h;
    char label[32];
    bool hovered;
    bool clicked;
} Button;

bool draw_button(Button* btn, SDL_Renderer* renderer, int mouse_x, int mouse_y, bool mouse_down) {
    // Check hover
    btn->hovered = (mouse_x >= btn->x && mouse_x < btn->x + btn->w &&
                   mouse_y >= btn->y && mouse_y < btn->y + btn->h);

    // Draw button
    SDL_Color color = btn->hovered ? (SDL_Color){100, 100, 255, 255}
                                    : (SDL_Color){50, 50, 200, 255};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {btn->x, btn->y, btn->w, btn->h};
    SDL_RenderFillRect(renderer, &rect);

    // Draw label (using your existing text rendering)
    sdl_puttext(btn->x + 10, btn->y + 10, 1, btn->label);

    // Check click
    if (btn->hovered && mouse_down && !btn->clicked) {
        btn->clicked = true;
        return true; // Button was clicked
    }
    btn->clicked = mouse_down;
    return false;
}
```

---

## Option 3: Full Custom SDL UI (HARD - Not Recommended)

Build everything from scratch using SDL2 primitives. This is a **lot of work**:

- Text input with cursor, selection, copy/paste
- Scroll bars
- Tab navigation
- Keyboard shortcuts
- Input validation
- Layout management

**Only do this if you want to learn UI programming or need pixel-perfect control.**

---

## Recommended Migration Path

### Phase 1: Keep Windows Dialog (Temporary)

While porting to SDL, you can **keep the Windows dialog** working:

```c
void options(void) {
    #if DD_ENABLED
        // Use Windows dialog (old code)
        if (DialogBox(hinst, MAKEINTRESOURCE(OPTIONS), desk_hwnd, OptionsProc) == -1) {
            MessageBox(0, "Dialog failed", "Error", MB_OK);
            exit(1);
        }
    #else
        // Use SDL options UI (new code)
        show_sdl_options_dialog();
    #endif
}
```

### Phase 2: Implement SDL Options with ImGui

1. Add ImGui to your project
2. Create `sdl_options.cpp` with ImGui UI
3. Port all the settings and inputs
4. Test thoroughly

### Phase 3: Remove Windows Code

Once SDL version works, remove the `#if DD_ENABLED` block and Windows dialog entirely.

---

## Handling File Dialogs

### Option A: tinyfiledialogs (Native OS Dialogs)
```c
#include "tinyfiledialogs.h"

int save_dialog_sdl(char* outpath) {
    char const* filters[1] = {"*.chr"};
    char* filename = tinyfiledialogs_saveFileDialog(
        "Save Character",
        "character.chr",
        1,
        filters,
        "Character Files"
    );

    if (filename) {
        strcpy(outpath, filename);
        return 1;
    }
    return 0;
}
```

### Option B: ImGui File Browser Extension
- ImGui has community extensions for file browsing
- GitHub: https://github.com/AirGuanZ/imgui-filebrowser
- Fully integrated with ImGui, no native dialogs

---

## Threading (Connection Dialog)

The Windows version uses `_beginthread()` for async server connection. In SDL:

### Option 1: SDL_CreateThread (Recommended)
```c
#include <SDL2/SDL_thread.h>

int connection_thread_func(void* data) {
    HWND hwnd = (HWND)data;  // Or pass a struct with all needed data

    // Connect to server
    so_connect_impl();

    return 0;
}

// Launch thread:
SDL_Thread* thread = SDL_CreateThread(connection_thread_func, "Connection", hwnd);
SDL_DetachThread(thread);  // Let it run independently
```

### Option 2: Keep Using Windows Threads
If you're on Windows only, `_beginthread()` still works fine with SDL. Just guard it:

```c
#ifdef _WIN32
    _beginthread(so_connect, 16384, (void*)hwnd);
#else
    SDL_CreateThread(so_connect_wrapper, "Connection", hwnd);
#endif
```

---

## Complete Migration Checklist

- [ ] Choose UI approach (ImGui recommended)
- [ ] Install ImGui and integrate with SDL
- [ ] Create new `sdl_options.cpp` file
- [ ] Port all form inputs (name, desc, password)
- [ ] Port race/gender radio buttons
- [ ] Port checkboxes (sound, shadows, darkmode)
- [ ] Implement buttons (Connect, Quit, New, Load, Save)
- [ ] Add file dialog library (tinyfiledialogs)
- [ ] Replace `_beginthread` with `SDL_CreateThread`
- [ ] Test character creation flow
- [ ] Test save/load functionality
- [ ] Test server connection
- [ ] Remove Windows dialog code
- [ ] Update build system to compile C++ (for ImGui)

---

## Build System Changes

### Makefile:
```makefile
# Add C++ compiler for ImGui
CXX = g++
CXXFLAGS = $(CFLAGS) -std=c++11

# ImGui sources
IMGUI_DIR = imgui
IMGUI_SOURCES = $(IMGUI_DIR)/imgui.cpp \
                $(IMGUI_DIR)/imgui_draw.cpp \
                $(IMGUI_DIR)/imgui_widgets.cpp \
                $(IMGUI_DIR)/imgui_tables.cpp \
                $(IMGUI_DIR)/imgui_impl_sdl2.cpp \
                $(IMGUI_DIR)/imgui_impl_sdlrenderer2.cpp

# Your SDL options file
SDL_OPTIONS_SOURCES = sdl_options.cpp

# Combine all sources
CXX_SOURCES = $(IMGUI_SOURCES) $(SDL_OPTIONS_SOURCES)
CXX_OBJECTS = $(CXX_SOURCES:.cpp=.o)

# Link with C++
TheLastGate.exe: $(C_OBJECTS) $(CXX_OBJECTS)
	$(CXX) -o $@ $^ $(LIBS)

# C++ compilation rule
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(IMGUI_DIR) -c $< -o $@
```

---

## Summary

**Best Approach for Your Project:**

1. **Use ImGui** for the options dialog - it's battle-tested, easy to use, and looks professional
2. **Use tinyfiledialogs** for native Load/Save dialogs - gives familiar OS file pickers
3. **Use SDL_CreateThread** for async server connection - cross-platform threading
4. **Keep the Windows dialog working** during migration with `#if DD_ENABLED` guards

This gives you a modern, cross-platform UI while maintaining the same functionality as the Windows dialog.
