/*
 * DD.H - DirectDraw Interface Header
 *
 * This header provides the main DirectDraw rendering interface for the client.
 * It handles display initialization, sprite rendering, and screen updates.
 *
 * TODO: MODERN GCC/MINGW COMPATIBILITY
 * ====================================
 * This entire interface is built on deprecated DirectDraw API.
 * For modern compiler compatibility, the underlying implementation
 * must be replaced while keeping the API signatures intact.
 *
 * Recommended approach:
 * 1. Keep this header's function signatures unchanged
 * 2. Re-implement dd.c using SDL2 or another modern graphics library
 * 3. Map DirectDraw concepts to modern equivalents:
 *    - LPDIRECTDRAWSURFACE -> SDL_Texture* or SDL_Surface*
 *    - Flip/BltFast -> SDL_RenderPresent/SDL_RenderCopy
 *    - Color keys -> SDL_SetColorKey or alpha blending
 *    - Surface locking -> SDL_LockTexture/SDL_LockSurface
 *
 * See dd.c for detailed TODO comments on each function.
 */

// Display dimensions (width and height in pixels)
extern int MAXX,MAXY;

extern int tricky_flag;
extern int maxmem,usedmem,maxvid,usedvid;
extern char *DDERR;
extern int blockcnt,blocktot,blockgc;
extern int alphapix,fullpix;
extern int dd_cache_hit,dd_cache_miss,MAXCACHE,invisible,cachex,cachey,MAXXOVER;
extern int RED,GREEN,BLUE,RGBM,MAXXOVER;
extern int gamma;
extern int usedvidmem;

void copysprite(int nr,int effect,int x,int y,int xoff,int yoff);
void dd_flip(void);
void dd_flip_windowed(void);
void dd_showbar(int xf,int yf,int xs,int ys,unsigned short col);
void copyspritex(int nr,int xpos,int ypos,int effect);
void dd_showbox(int xf,int yf,int xs,int ys,unsigned short col);
void dd_alphaeffect_magic(int nr,int str,int xpos,int ypos,int xoff,int yoff);
int get_avgcol(int nr);

void dd_puttext(int x,int y,int font,char *text);
void dd_gputc(int xpos,int ypos,int font,int c);
void dd_gputtext(int xpos,int ypos,int font,char *text,int xoff,int yoff);
void dd_putc(int xpos,int ypos,char font,int c);
void dd_xputtext(int x,int y,int font,char *format,...);

void dd_show_map(unsigned short *src,int xo,int yo,int magnify);
void dd_invalidate_alpha(void);

// Global tick counter for animation timing
extern unsigned int volatile dd_tick;

/**
 * init_xalloc - Initialize custom memory allocator heap
 *
 * Creates a Windows heap for managing memory allocations. This custom allocator
 * tracks memory usage and provides optimized allocation for the game engine.
 * The heap is created with HEAP_GENERATE_EXCEPTIONS to ensure allocation failures
 * raise exceptions rather than returning NULL.
 */
void init_xalloc(void);

/**
 * xmalloc - Allocate memory from the custom heap
 * @size: Number of bytes to allocate
 *
 * Allocates memory from the custom heap and tracks allocation statistics.
 * Updates global memory usage counters (usedmem) and block count (blockcnt).
 *
 * Returns: Pointer to allocated memory
 */
void *xmalloc(int size);

/**
 * dd_init - Initialize DirectDraw in fullscreen mode
 * @hwnd: Window handle to attach DirectDraw to (HWND is Windows-specific, use SDL_Window* in SDL2)
 * @x: Screen width in pixels
 * @y: Screen height in pixels
 *
 * Sets up DirectDraw with exclusive fullscreen mode at the specified resolution.
 * Creates primary and back buffer surfaces for double buffering.
 *
 * Returns: 0 on success, negative error code on failure
 *
 * TODO: Modern GCC/MinGW - HWND parameter is Windows-specific
 * In SDL2, this would become: int dd_init(SDL_Window *window, int x, int y)
 * or the window creation could be part of this function
 */
int dd_init(HWND hwnd,int x,int y);

/**
 * dd_init_windowed - Initialize DirectDraw in windowed mode
 * @hwnd: Window handle to attach DirectDraw to (HWND is Windows-specific, use SDL_Window* in SDL2)
 * @x: Window width in pixels
 * @y: Window height in pixels
 *
 * Sets up DirectDraw for windowed rendering with a clipper for window boundaries.
 *
 * Returns: 0 on success, negative error code on failure
 *
 * TODO: Modern GCC/MinGW - HWND parameter is Windows-specific
 */
int dd_init_windowed(HWND hwnd,int x,int y);

/**
 * dd_init_sprites - Initialize sprite cache system
 *
 * Prepares the sprite caching system for loading and managing game sprites.
 */
void dd_init_sprites();

/**
 * dd_change - Change display resolution
 * @x: New screen width
 * @y: New screen height
 *
 * Switches the display mode to a new resolution.
 *
 * Returns: 0 on success, negative error code on failure
 */
int dd_change(int x,int y);

/**
 * dd_savescreen - Save current screen to a file
 *
 * Captures the current display buffer and saves it as a screenshot.
 */
void dd_savescreen(void);

/**
 * dd_ticker - Update the DirectDraw tick counter
 *
 * Increments the global tick counter used for animation timing.
 */
void dd_ticker(void);

/**
 * restore_palette - Restore the display palette
 *
 * Reloads the color palette after it has been lost or corrupted.
 */
void restore_palette(void);

/**
 * dd_isvisible - Check if the display window is visible
 *
 * Determines whether the game window is currently visible to the user.
 *
 * Returns: Non-zero if visible, 0 if not visible
 */
int dd_isvisible(void);

/**
 * dd_deinit - Clean up DirectDraw in fullscreen mode
 *
 * Releases all DirectDraw surfaces and resources, restores original display mode.
 */
void dd_deinit(void);

/**
 * dd_deinit_windowed - Clean up DirectDraw in windowed mode
 *
 * Releases DirectDraw resources used for windowed rendering.
 */
void dd_deinit_windowed(void);

/**
 * dd_shadow_clear - Clear all shadow rendering data
 *
 * Removes all shadow effects from the shadow buffer.
 */
void dd_shadow_clear(void);

/**
 * dd_shadow - Render a shadow sprite
 * @nr: Sprite number to render as shadow
 * @xpos: X position on screen
 * @ypos: Y position on screen
 * @xoff: X offset within sprite
 * @yoff: Y offset within sprite
 *
 * Draws a darkened version of a sprite to create a shadow effect.
 */
void dd_shadow(int nr,int xpos,int ypos,int xoff,int yoff);
