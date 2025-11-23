# SDL Migration TODO - Remaining Windows/DirectDraw Dependencies

This document lists all remaining Windows-specific and DirectDraw code that needs SDL replacements when running with `DD_ENABLED=0`.

## Summary Status

✅ **COMPLETED:**
- SDL window creation and rendering
- SDL input handling (keyboard, mouse, text input)
- SDL sprite rendering (copysprite, copyspritex)
- SDL text rendering (puttext, xputtext, gputtext)
- SDL boxes and bars (showbox, showbar)
- SDL shadows and alpha effects
- SDL minimap rendering
- SDL cursors
- SDL audio (SDL2_mixer integration)
- Windows message pump (do_msg, eng_flip)

❌ **STILL NEEDS SDL REPLACEMENT:**
- Windows timing functions (GetTickCount, Sleep)
- Windows window management (WinMain, HWND, InitWindow)
- DirectDraw frame flipping when DD_ENABLED=1

---

## 2. MEDIUM: Windows Timing Functions

**Status:** Used throughout codebase, needs SDL replacement when DD_ENABLED=0

### GetTickCount() Usage:

**engine.c:**
- Line 2257, 2259: Animation timing for spell effects
- Line 3535: Frame timing in eng_flip
- Line 3548: Frame timing loop
- Line 4827: Main game loop initialization

**dd.c:**
- Line 2330: Cache timing

### SDL Replacement:

```c
// Simple find/replace when SDL_ENABLED:
GetTickCount() -> SDL_GetTicks()
Sleep(ms)      -> SDL_Delay(ms)
```

**TODO:**
- [ ] Create timing abstraction macro in common header:
```c
#if SDL_ENABLED
    #define GAME_GetTicks() SDL_GetTicks()
    #define GAME_Delay(ms) SDL_Delay(ms)
#else
    #define GAME_GetTicks() GetTickCount()
    #define GAME_Delay(ms) Sleep(ms)
#endif
```
- [ ] Replace all `GetTickCount()` with `GAME_GetTicks()`
- [ ] Replace all `Sleep()` with `GAME_Delay()`

---

## 3. LOW PRIORITY: Windows Window Management

**Status:** Only needed when DD_ENABLED=1, SDL has its own window

### WinMain and InitWindow - `main.c`

**WinMain() - Line 928:**
- Windows-specific entry point
- Works fine with SDL (SDL wraps main() as WinMain on Windows)

**InitWindow() - Line 758:**
```c
HWND InitWindow(HINSTANCE hInstance,int nCmdShow)
```
- Only called when `DD_ENABLED=1` (line 1012)
- Creates Windows window for DirectDraw
- SDL creates its own window in `sdl_init()`

**No action needed** - SDL and DirectDraw windows are separate, only one is created depending on DD_ENABLED flag.

---

## 4. LOW PRIORITY: Windows Cursor Management

**Status:** Already has SDL implementation, just needs refactoring

### SetCursor() - `engine.c:3082-3083`

```c
if (DD_ENABLED) SetCursor(cursor[cursor_type]);
if (SDL_ENABLED) SDL_SetCursor(cursors[cursor_type]);
```

**Already working correctly!** Both implementations exist side-by-side.

**Optional cleanup:**
- [ ] Remove Windows cursor loading from `main.c` when `SDL_ENABLED=1`
- [ ] Consolidate cursor loading into abstraction layer

---

## 5. BONUS: Remove debug printf from eng_display_win

### Location: `engine.c:2093`

```c
printf("eng_display_win called: init=%d, DD_ENABLED=%d, SDL_ENABLED=%d\n", init, DD_ENABLED, SDL_ENABLED);
```

**TODO:**
- [ ] Remove debug printf once rendering issues are fully resolved

---

## Priority Order

### HIGH PRIORITY (Blocks pure SDL mode):
1. **Add SDL timing macros** - Create `GAME_GetTicks()` and `GAME_Delay()` abstractions
2. **Guard Windows message pump** - Add `#if DD_ENABLED` around PeekMessage/TranslateMessage/DispatchMessage in `do_msg()` and `eng_flip()`
3. **Replace timing calls** - Change all `GetTickCount()` to use new macros

### MEDIUM PRIORITY (Quality of life):
4. **Test frame timing** - Ensure SDL_Delay properly limits frame rate
5. **Remove debug output** - Clean up printf from eng_display_win

### LOW PRIORITY (Optional cleanup):
6. **Cursor abstraction** - Consolidate cursor management (already works)
7. **Documentation** - Update build instructions

---

## Testing Checklist

When implementing these changes, test with:

- [ ] `DD_ENABLED=0 SDL_ENABLED=1` - Pure SDL mode (target)
- [ ] `DD_ENABLED=1 SDL_ENABLED=0` - Pure DirectDraw mode (legacy)
- [ ] Verify frame rate is consistent (~60 FPS or your target)
- [ ] Verify no Windows message errors in SDL mode
- [ ] Check CPU usage is reasonable in SDL mode
- [ ] Test all input (keyboard, mouse, text) still works
- [ ] Verify rendering is smooth without tearing

---

## Notes

- The codebase currently has a hybrid approach where DirectDraw code still exists but is gated behind `DD_ENABLED`
- SDL code runs alongside but doesn't depend on DirectDraw
- The main blocker for pure SDL mode is the Windows message pump in `do_msg()` and `eng_flip()`
- Once timing is abstracted, the game should run fully on SDL without any Windows-specific code paths (when DD_ENABLED=0)
