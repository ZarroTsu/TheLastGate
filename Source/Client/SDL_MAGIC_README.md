# SDL2 Magic Glow Effect Implementation

This implements the `dd_alphaeffect_magic` effect from DirectDraw using modern SDL2 rendering with pre-rendered gradient textures.

## Files

- **sdl_magic.c** - Core implementation of magic glow effects
- **sdl_magic_demo.c** - Interactive demo showcasing all effect colors
- **Makefile.magic** - Build configuration for the demo
- **sdl.h** - Updated with magic effect function declarations

## Features

### Pre-rendered Gradient Textures
Creates 64x64 pixel gradient textures with the exact elliptical falloff pattern from the original DirectDraw implementation:
- Horizontal fade: Linear falloff from center (x=32)
- Vertical fade: Asymmetric (gentle top, long middle, fast bottom)
- Optimized for isometric view

### Additive Blending
Uses SDL_BLENDMODE_ADD for glow effects that brighten the background, creating realistic magical lighting.

### Color Combinations (nr parameter)
- `nr=1` (0b001): Red glow - fire, damage effects
- `nr=2` (0b010): Green glow - poison, nature magic
- `nr=3` (0b011): Yellow glow - holy magic (red+green)
- `nr=4` (0b100): Blue glow - ice, mana effects
- `nr=5` (0b101): Magenta glow - arcane magic (red+blue)
- `nr=6` (0b110): Cyan glow - water magic (green+blue)
- `nr=7` (0b111): White glow - pure light (all colors)

### Strength Control (str parameter)
- `str=1`: Full intensity
- `str=2`: Half intensity
- `str=4`: Quarter intensity
- Higher values = weaker effect

## API

### Initialization
```c
void sdl_init_magic_effects(void);
```
Call once during game initialization to pre-render all gradient textures.

### Rendering
```c
void sdl_alphaeffect_magic(int nr, int str, int xpos, int ypos, int xoff, int yoff);
```
Render a magic glow effect:
- `nr`: Color flags (1-7)
- `str`: Strength divisor (1+)
- `xpos`, `ypos`: World coordinates (center of effect)
- `xoff`, `yoff`: Screen offsets

### Scaled Rendering
```c
void sdl_alphaeffect_magic_scaled(int nr, int str, int xpos, int ypos, int xoff, int yoff, float scale);
```
Same as above but supports custom scaling:
- `scale`: Size multiplier (1.0 = 64x64, 2.0 = 128x128)

### Cleanup
```c
void sdl_deinit_magic_effects(void);
```
Free all gradient textures during shutdown.

## Building the Demo

### Windows (MinGW)
```bash
mingw32-make -f Makefile.magic
magic_demo.exe
```

### Linux/macOS
```bash
make -f Makefile.magic
./magic_demo
```

## Demo Controls

- **ESC** - Exit
- **1** - Red glow (fire/damage)
- **2** - Green glow (poison/nature)
- **3** - Yellow glow (holy magic)
- **4** - Blue glow (ice/mana)
- **5** - Magenta glow (arcane)
- **6** - Cyan glow (water)
- **7** - White glow (pure light)
- **+/-** - Adjust strength (1-8)
- **Mouse** - Move main effect position

The demo shows:
1. A grid of all 7 colors at the top (for comparison)
2. A large effect that follows your mouse
3. Dark background with grid lines to show blending

## Usage Example

```c
// Initialize once at startup
sdl_init_magic_effects();

// During game loop - render fire effect at player position
sdl_alphaeffect_magic(1, 1, player.x, player.y, 0, 0);

// Render weak blue mana effect
sdl_alphaeffect_magic(4, 4, spell.x, spell.y, 0, 0);

// Render large healing aura (scaled 2x)
sdl_alphaeffect_magic_scaled(7, 1, target.x, target.y, 0, 0, 2.0);

// Cleanup at shutdown
sdl_deinit_magic_effects();
```

## Integration with Existing Code

To replace `dd_alphaeffect_magic` calls:

1. Add `#include "sdl_magic.c"` to your build (or compile separately)
2. Call `sdl_init_magic_effects()` during initialization
3. Replace `dd_alphaeffect_magic()` calls with `sdl_alphaeffect_magic()`
4. Call `sdl_deinit_magic_effects()` during cleanup

The function signature is identical to the DirectDraw version, so it's a drop-in replacement.

## Technical Details

### Gradient Algorithm
The elliptical gradient exactly matches the original DirectDraw implementation:

```c
int e = 32;  // Start with full intensity

// Horizontal fade (symmetric)
if (x < 32) e -= (32 - x);
if (x > 31) e -= (x - 31);

// Vertical fade (asymmetric for isometric)
if (y < 16) e -= (16 - y);      // Gentle top fade
if (y > 55) e -= (y - 55) * 2;  // Fast bottom fade

e /= strength;  // Apply strength divisor
```

This creates a vertically-compressed ellipse suitable for isometric projection.

### Memory Usage
- 8 gradient textures (one per color combination)
- Each texture: 64×64 pixels × 4 bytes (RGBA8888) = 16 KB
- Total: ~128 KB of video memory

### Performance
- Pre-rendered gradients = very fast (~1 texture blit per effect)
- No per-pixel calculations during rendering
- Suitable for rendering many simultaneous effects

## Comparison with DirectDraw Version

### DirectDraw (Original)
- Reads destination pixels
- Darkens background (subtracts from RGB)
- Adds colored glow
- Writes back to framebuffer
- CPU-based pixel manipulation

### SDL2 (This Implementation)
- Pre-rendered gradient textures
- Additive blending (brightens background)
- GPU-accelerated rendering
- No destination pixel reads

### Visual Differences
- **Background darkening**: The SDL2 version uses additive blending instead of subtractive darkening. This means:
  - Original: Darkens background, then adds glow
  - SDL2: Only brightens with glow
- The glow appearance is very similar, but the SDL2 version is slightly brighter
- For pixel-perfect recreation, would need render-to-texture with custom blend mode

## Future Enhancements

### True Shader Implementation
For exact pixel-perfect recreation including background darkening:
1. Render scene to texture
2. Apply custom fragment shader with subtractive darkening
3. Render final result to screen

See `dd.c:1715-1757` for GLSL shader example.

### Multiple Simultaneous Effects
The current implementation handles overlapping effects naturally through additive blending. Colors will mix realistically (red + blue = magenta).

### Animated Effects
Add pulsing or ripple animations by modifying alpha/scale over time:
```c
float pulse = sin(SDL_GetTicks() / 200.0f) * 0.3f + 0.7f;  // 0.4-1.0 range
sdl_alphaeffect_magic_scaled(7, 1, x, y, 0, 0, pulse);
```

## Troubleshooting

### Black screen
- Ensure `sdl_init_magic_effects()` is called after SDL initialization
- Check SDL error with `SDL_GetError()`

### No visible effect
- Verify coordinates are on-screen
- Check `str` parameter isn't too high (try `str=1`)
- Ensure renderer clear/present cycle is correct

### Performance issues
- Reduce number of simultaneous effects
- Consider using lower resolution gradients (32×32 instead of 64×64)

## License

Part of The Last Gate client codebase.
