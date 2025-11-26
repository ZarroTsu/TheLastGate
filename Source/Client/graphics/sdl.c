//
// Created by james on 11/22/2025.
//
#include "sdl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL_image.h>


#include "atlas.h"
#include "../render.h"
#include "loader.h"
#include "../inter.h"
#include "../main.h"
#include "sprite_data.h"
#include "glad/glad.h"
#include "shaders.h"
#include "../log.h"

App app;

// Simple orthographic projection matrix helper
// Creates a 2D orthographic projection from pixel coordinates to NDC
static void create_ortho_matrix(float *matrix, float left, float right, float bottom, float top) {
    // Initialize to identity
    memset(matrix, 0, 16 * sizeof(float));

    matrix[0] = 2.0f / (right - left);
    matrix[5] = 2.0f / (top - bottom);
    matrix[10] = -1.0f;
    matrix[12] = -(right + left) / (right - left);
    matrix[13] = -(top + bottom) / (top - bottom);
    matrix[15] = 1.0f;
}

// Create a model matrix for sprite positioning and scaling
void create_model_matrix(float *matrix, float x, float y, float width, float height) {
    // Initialize to identity
    memset(matrix, 0, 16 * sizeof(float));

    // Scale by sprite dimensions, then translate to position
    matrix[0] = width; // Scale X
    matrix[5] = height; // Scale Y
    matrix[10] = 1.0f; // Scale Z (unused)
    matrix[12] = x; // Translate X
    matrix[13] = y; // Translate Y
    matrix[15] = 1.0f; // Homogeneous coordinate
}

// Cached projection matrix (created once)
float projection_matrix[16];
int projection_initialized = 0;

#define TILE 32
#define MAXEFFECT 1024

static struct FontCache font_cache[10];
static SpriteData *sprite_data = NULL;
static GLuint minimap_gl_texture = 0; // OpenGL texture for minimap (128x128)

GLuint gamma_shader;
// Cached uniform locations (set once at initialization)
GLint gamma_uProjection = -1;
GLint gamma_uUV0 = -1;
GLint gamma_uUV1 = -1;
GLint gamma_uModel = -1;
GLint gamma_uTexture = -1;
GLint gamma_uGammaScale = -1;
GLint gamma_uGammaEffect = -1;
GLint gamma_uShadeEffect = -1;
GLint gamma_uRed = -1;
GLint gamma_uGreen = -1;
GLint gamma_uInvis = -1;
GLint gamma_uGrey = -1;
GLint gamma_uInfra = -1;
GLint gamma_uWater = -1;
GLint gamma_uShadow = -1;
GLint gamma_uUseInstancing = -1;

static GLuint solid_shader;
// Cached uniform locations for solid color shader
static GLint solid_uProjection = -1;
static GLint solid_uModel = -1;
static GLint solid_uColor = -1;

extern char path[];

extern void *conv_load(int nr, int *xs, int *ys);

extern FILE *load_pnglib(int nr);

int sdl_init(const int windowed) {
    const int windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | (!windowed ? SDL_WINDOW_BORDERLESS : 0);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG("Couldn't initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    // Request OpenGL 3.3 Core Profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;

    if (!windowed) {
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(0, &mode);
        width = mode.w - 2; // You have to do it by 2, or it creates a tearing effect when you walk.
        height = mode.h - 2; // This obnoxious hack makes OpenGL not try to force it into fullscreen esque behaviour.
    }

    app.window = SDL_CreateWindow("Last Gate SDL", windowed ? SDL_WINDOWPOS_UNDEFINED : 0, windowed ? SDL_WINDOWPOS_UNDEFINED : 0, width,
                                  height, windowFlags);

    SDL_GetWindowSize(app.window, &app_state.window_size[0], &app_state.window_size[1]);

    if (!app.window) {
        LOG("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
        return -1;
    }

    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

    // Create OpenGL context
    app.gl_context = SDL_GL_CreateContext(app.window);
    if (!app.gl_context) {
        LOG("Failed to create OpenGL context: %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_MakeCurrent(app.window, app.gl_context);

    // Load OpenGL functions with GLAD
    if (!gladLoadGL()) {
        LOG("Failed to load OpenGL functions with GLAD\n");
        return -1;
    }

    LOG("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Disable vsync - let game's internal frame limiter handle timing
    // VSync at 60 FPS conflicts with game's target FPS, causing frame skip issues
    SDL_GL_SetSwapInterval(0); // 0 = vsync off, 1 = vsync on

    // Set up OpenGL state
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load gamma correction shader
    gamma_shader = load_program(TLG_Shader_Effect);
    if (!gamma_shader) {
        LOG("Failed to load gamma shader\n");
        return -1;
    }

    // Cache uniform locations (query once, not every frame!)
    gamma_uProjection = glGetUniformLocation(gamma_shader, "uProjection");
    gamma_uUV0 = glGetUniformLocation(gamma_shader, "uUV0");
    gamma_uUV1 = glGetUniformLocation(gamma_shader, "uUV1");
    gamma_uModel = glGetUniformLocation(gamma_shader, "uModel");
    gamma_uTexture = glGetUniformLocation(gamma_shader, "uTexture");
    gamma_uGammaScale = glGetUniformLocation(gamma_shader, "uGammaScale");
    gamma_uGammaEffect = glGetUniformLocation(gamma_shader, "uGammaEffect");
    gamma_uShadeEffect = glGetUniformLocation(gamma_shader, "uShadeEffect");

    gamma_uRed = glGetUniformLocation(gamma_shader, "uRed");
    gamma_uGreen = glGetUniformLocation(gamma_shader, "uGreen");
    gamma_uInvis = glGetUniformLocation(gamma_shader, "uInvis");
    gamma_uGrey = glGetUniformLocation(gamma_shader, "uGrey");
    gamma_uInfra = glGetUniformLocation(gamma_shader, "uInfra");
    gamma_uWater = glGetUniformLocation(gamma_shader, "uWater");
    gamma_uShadow = glGetUniformLocation(gamma_shader, "uShadow");
    gamma_uUseInstancing = glGetUniformLocation(gamma_shader, "uUseInstancing");
    gamma_uModel = glGetUniformLocation(gamma_shader, "uModel");

    LOG("Cached gamma shader uniform locations: proj=%d, model=%d, tex=%d, gamma=%d, instancing=%d\n",
        gamma_uProjection, gamma_uModel, gamma_uTexture, gamma_uGammaScale, gamma_uUseInstancing);

    // Load solid color shader for UI primitives (boxes, bars)
    solid_shader = load_program(TLG_Shader_Solid);
    if (!solid_shader) {
        LOG("Failed to load solid color shader\n");
        return -1;
    }

    // Cache solid shader uniform locations
    solid_uProjection = glGetUniformLocation(solid_shader, "uProjection");
    solid_uModel = glGetUniformLocation(solid_shader, "uModel");
    solid_uColor = glGetUniformLocation(solid_shader, "uColor");
    LOG("Cached solid shader uniform locations: proj=%d, model=%d, color=%d\n",
        solid_uProjection, solid_uModel, solid_uColor);

    // Create VAO/VBO for sprite quad rendering
    // Quad vertices: position (x, y, z) + texcoord (u, v)
    // We'll use a single quad and update it per sprite with transforms
    float quad_vertices[] = {
        // positions        // texture coords
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top left
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top right
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom left

        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom left
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top right
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f // bottom right
    };

    glGenVertexArrays(1, &app.quad_vao);
    glGenBuffers(1, &app.quad_vbo);

    glBindVertexArray(app.quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, app.quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    LOG("OpenGL quad VAO/VBO created\n");

    // Allocate CPU-side batch buffer
    app.batch_buffer = malloc(MAX_BATCH_SIZE * sizeof(SpriteInstance));
    if (!app.batch_buffer) {
        LOG("Failed to allocate batch buffer\n");
        return -1;
    }
    app.batch_count = 0;

    // Create instance VBO for batched rendering
    glGenBuffers(1, &app.instance_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, app.instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_BATCH_SIZE * sizeof(SpriteInstance), NULL, GL_DYNAMIC_DRAW);

    // Configure instance attributes in VAO
    glBindVertexArray(app.quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, app.instance_vbo);

    // Model matrix (4 vec4s = mat4, locations 2-5)
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(2 + i);
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance),
                              (void *) (i * 4 * sizeof(float)));
        glVertexAttribDivisor(2 + i, 1); // Advance once per instance
    }

    // UV0 (location 6)
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance),
                          (void *) offsetof(SpriteInstance, uv0));
    glVertexAttribDivisor(6, 1);

    // UV1 (location 7)
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance),
                          (void *) offsetof(SpriteInstance, uv1));
    glVertexAttribDivisor(7, 1);

    // Gamma scale (location 8)
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance),
                          (void *) offsetof(SpriteInstance, gamma_scale));
    glVertexAttribDivisor(8, 1);

    // Shade effect (location 9)
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance),
                          (void *) offsetof(SpriteInstance, shade_effect));
    glVertexAttribDivisor(9, 1);

    // Gamma effect (location 10)
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(10, 1, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance),
                          (void *) offsetof(SpriteInstance, gamma_effect));
    glVertexAttribDivisor(10, 1);

    // Flags (location 11)
    glEnableVertexAttribArray(11);
    glVertexAttribIPointer(11, 1, GL_INT, sizeof(SpriteInstance),
                           (void *) offsetof(SpriteInstance, flags));
    glVertexAttribDivisor(11, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LOG("Sprite batch system initialized (max %d sprites per batch)\n", MAX_BATCH_SIZE);

    init_atlas();

    for (int n = 1; n < 10; n++) {
        char path[256];
        sprintf(path, "resources/cursor%d.png", n);
        cursors[n] = load_cursor_from_png(path, 0, 0);
    }

    return 0;
}

void sdl_deinit(void) {
    if (app.batch_buffer) {
        free(app.batch_buffer);
        app.batch_buffer = NULL;
    }

    if (app.instance_vbo) {
        glDeleteBuffers(1, &app.instance_vbo);
        app.instance_vbo = 0;
    }

    if (app.quad_vao) {
        glDeleteVertexArrays(1, &app.quad_vao);
        app.quad_vao = 0;
    }

    if (app.quad_vbo) {
        glDeleteBuffers(1, &app.quad_vbo);
        app.quad_vbo = 0;
    }

    if (minimap_gl_texture) {
        glDeleteTextures(1, &minimap_gl_texture);
        minimap_gl_texture = 0;
    }

    if (gamma_shader) {
        glDeleteProgram(gamma_shader);
        gamma_shader = 0;
    }

    if (solid_shader) {
        glDeleteProgram(solid_shader);
        solid_shader = 0;
    }

    if (app.gl_context) {
        SDL_GL_DeleteContext(app.gl_context);
        app.gl_context = NULL;
    }

    if (app.window) {
        SDL_DestroyWindow(app.window);
        app.window = NULL;
    }

    SDL_Quit();
}

/**
 * sdl_init_sprites - Initialize sprite management system
 *
 * Allocates sprite table and sets memory limits for texture caching.
 * Should be called after sdl_init().
 */
void sdl_init_sprites(void) {
    if (!sprite_data) {
        sprite_data = calloc(MAX_SPRITES, sizeof(SpriteData));
        if (!sprite_data) {
            LOG("Failed to allocate sprite table\n");
            return;
        }
    }
}

/**
 * sdl_batch_begin - Start a new sprite batch
 *
 * Resets the batch counter to prepare for accumulating sprites.
 * Call this at the start of each frame.
 */
void sdl_batch_begin(void) {
    app.batch_count = 0;
}

/**
 * sdl_batch_flush - Render all batched sprites
 *
 * Uploads accumulated sprite data to GPU and renders in a single instanced draw call.
 * Call this at the end of each frame or when switching textures/render states.
 */
void sdl_batch_flush(void) {
    if (app.batch_count == 0) return;

    sdl_start_scaling();
    // Upload instance data to GPU
    glBindBuffer(GL_ARRAY_BUFFER, app.instance_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, app.batch_count * sizeof(SpriteInstance), app.batch_buffer);

    // Set projection matrix once for all sprites
    glUseProgram(gamma_shader);
    glUniformMatrix4fv(gamma_uProjection, 1, GL_FALSE, projection_matrix);
    glUniform1i(gamma_uTexture, 0); // Texture unit 0
    glUniform1i(gamma_uShadow, 0); // Ensure shadow uniform is off for batch
    glUniform1i(gamma_uUseInstancing, 1); // Enable instanced rendering mode

    // Bind atlas texture (all batched sprites use atlas)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tile_atlas);

    // Render all sprites with single instanced draw call
    glBindVertexArray(app.quad_vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, app.batch_count);
    glBindVertexArray(0);
    sdl_stop_scaling();
    // Reset batch for next frame
    app.batch_count = 0;

    // printf("Flushed batch: %d sprites\n", app.batch_count);
}

/**
 * get_pixel - Read pixel from SDL_Surface at coordinates
 * @surface: Source surface
 * @x: X coordinate
 * @y: Y coordinate
 *
 * Returns: Pixel value as Uint32
 */
static Uint32 get_pixel(SDL_Surface *surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *) surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1: return *p;
        case 2: return *(Uint16 *) p;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
        case 4: return *(Uint32 *) p;
        default: return 0;
    }
}

/**
 * calculate_average_color - Calculate average RGB color of surface
 * @surface: Source surface
 *
 * Returns: Average color in RGB565 format
 */
static unsigned short calculate_average_color(SDL_Surface *surface) {
    Uint32 total_r = 0, total_g = 0, total_b = 0;
    int pixel_count = 0; // Count only non-transparent pixels

    SDL_LockSurface(surface);

    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint8 r, g, b;
            Uint32 pixel = get_pixel(surface, x, y);
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);

            // Skip magenta transparency pixels (255, 0, 255)
            if (r == 255 && g == 0 && b == 255) {
                continue;
            }

            total_r += r;
            total_g += g;
            total_b += b;
            pixel_count++;
        }
    }

    SDL_UnlockSurface(surface);

    // If all pixels were transparent, return black
    if (pixel_count == 0) return 0;

    // Calculate average
    Uint8 avg_r = total_r / pixel_count;
    Uint8 avg_g = total_g / pixel_count;
    Uint8 avg_b = total_b / pixel_count;

    // Convert to RGB555 (DirectDraw format)
    return ((avg_r >> 3) << 10) | ((avg_g >> 3) << 5) | (avg_b >> 3);
}

/**
 * extract_alpha_channel - Extract partially transparent pixels
 * @surface: Source surface with alpha channel
 * @alphacnt_ptr: Output parameter for alpha data byte count
 *
 * Scans surface for pixels with partial transparency (0 < alpha < 255)
 * and stores them in format: x, y, r, g, b, a (6 bytes per pixel)
 *
 * Returns: Allocated alpha data array, or NULL if no alpha pixels found
 */
static unsigned char *extract_alpha_channel(SDL_Surface *surface, int *alphacnt_ptr) {
    unsigned char *alpha = NULL;
    int alphacnt = 0;
    int alphamax = 0;

    // Check if surface has alpha channel
    if (!(surface->format->format == SDL_PIXELFORMAT_RGBA32 ||
          surface->format->format == SDL_PIXELFORMAT_ARGB32 ||
          surface->format->Amask != 0)) {
        *alphacnt_ptr = 0;
        return NULL;
    }

    SDL_LockSurface(surface);

    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            Uint8 r, g, b, a;
            Uint32 pixel = get_pixel(surface, x, y);
            SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);

            // Store pixels with partial transparency
            if (a > 0 && a < 255) {
                // Grow buffer if needed
                if (alphacnt >= alphamax - 8) {
                    alphamax += 256;
                    alpha = realloc(alpha, alphamax);
                }

                alpha[alphacnt++] = (unsigned char) x;
                alpha[alphacnt++] = (unsigned char) y;
                alpha[alphacnt++] = r;
                alpha[alphacnt++] = g;
                alpha[alphacnt++] = b;
                alpha[alphacnt++] = a;
            }
        }
    }

    SDL_UnlockSurface(surface);

    // Shrink to actual size
    if (alpha && alphacnt > 0) {
        alpha = realloc(alpha, alphacnt);
    }

    *alphacnt_ptr = alphacnt;
    return alpha;
}

/**
 * sdl_load_sprite - Load a sprite into the atlas if it hasn't been loaded yet.
 *
 * @nr Sprite index number (0-MAX_SPRITES)
 *
 * Uses the loader.c file to load a surface from either a file or a .dat library.
 * That file will always be formatted using SOURCE_SPRITE_PIXEL_FORMAT, should always
 * have an alpha channel.
 */
void sdl_load_sprite(const int nr) {
    if (nr < 0 || nr >= MAX_SPRITES) return;
    if (!sprite_data) sdl_init_sprites();

    if (sprite_data[nr].gl_texture) {
        return;
    }

    SDL_Surface *surface = nullptr;
    int width = 0, height = 0;

    // Try to load from various sources
    surface = load_from_file(nr);
    if (!surface) surface = load_from_png_lib(nr);
    if (!surface) surface = load_from_gfx_lib(nr);

    // Fallback to sprite #35 if not found
    if (!surface && nr != 35) {
        LOG("Sprite %d not found, using fallback sprite #35\n", nr);
        surface = load_from_file(nr);
        if (!surface) surface = load_from_png_lib(nr);
        if (!surface) surface = load_from_gfx_lib(nr);
    }

    if (!surface) {
        LOG("Failed to load sprite %d\n", nr);
        return;
    }

    width = surface->w;
    height = surface->h;

    sprite_data[nr].surface = surface;

    unsigned int atlas = add_to_atlas(&sprite_data[nr]);
    sprite_data[nr].gl_texture = atlas;


    // Extract alpha channel for pixel-perfect blending
    int alphacnt = 0;
    unsigned char *alpha = extract_alpha_channel(surface, &alphacnt);

    // Calculate average color for lighting effects
    unsigned short avgcol = calculate_average_color(surface);

    // Store sprite data
    sprite_data[nr].alphacnt = alphacnt;
    sprite_data[nr].pixel_width = width;
    sprite_data[nr].pixel_height = height;
    sprite_data[nr].xs = (unsigned char) (width / TILE);
    sprite_data[nr].ys = (unsigned char) (height / TILE);
    sprite_data[nr].avgcol = avgcol;
}

static int first_render = 1;

void calculate_gamma_shader_params(int effect, float *shade_effect, float *gamma_effect, bool *grey, bool *infra,
                                   bool *water, bool *red, bool *green, bool *invis) {
    // Default: no lighting effect (0 means shader won't apply darkening)
    *shade_effect = 0.0f;
    *gamma_effect = 0.0f;

    // Initialize all effect bools to false
    *red = false;
    *green = false;
    *invis = false;
    *grey = false;
    *infra = false;
    *water = false;

    if (effect & 16) {
        effect -= 16;
        *red = true;
    } //red border
    if (effect & 32) {
        effect -= 32;
        *green = true;
    } //green border
    if (effect & 64) {
        effect -= 64;
        *invis = true;
    } //blackened out
    if (effect & 128) {
        effect -= 128;
        *grey = true;
    } //grey scale
    if (effect & 256) {
        effect -= 256;
        *infra = true;
    } //grey scale
    if (effect & 512) {
        effect -= 512;
        *water = true;
    } //grey scale

    // Only apply lighting darkening if effect is non-zero
    if (effect > 0) {
        float gammaEffect = (float) GAMMA_EFFECT;
        *shade_effect = (float) effect;
        *gamma_effect = gammaEffect;
    }
}

void sdl_copysprite(int nr, int effect, int x, int y, int xoff, int yoff) {
    if (nr == 0) return;

    sdl_load_sprite(nr);
    if (!sprite_data[nr].gl_texture) {
        if (first_render) {
            LOG("ERROR: No GL texture for sprite %d\n", nr);
        }
        return;
    }

    // Only batch sprites that are in the atlas
    if (!sprite_data[nr].loaded_in_atlas) {
        // Fall back to immediate rendering for non-atlas sprites
        // TODO: Implement separate batch for non-atlas sprites
        LOG("Warning: sprite %d not in atlas, skipping batching\n", nr);
        return;
    }

    // Initialize projection matrix once
    if (!projection_initialized) {
        create_ortho_matrix(projection_matrix, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);
        projection_initialized = 1;
        LOG("Initialized projection matrix\n");
    }

    if (first_render) {
        LOG("First render: sprite=%d, effect=%d, pos=(%d,%d), offset=(%d,%d)\n",
            nr, effect, x, y, xoff, yoff);
        LOG("Sprite dimensions: %dx%d pixels\n", sprite_data[nr].pixel_width, sprite_data[nr].pixel_height);
        first_render = 0;
    }

    unsigned int xs = sprite_data[nr].xs;
    unsigned int ys = sprite_data[nr].ys;

    // Isometric coordinate conversion
    unsigned int rx = (x / 2) + (y / 2) - (xs * 16) + 32 + X_OFFSET - ((RENDER_DISTANCE - 34) / 2 * 32);
    if (x < 0 && x & 1) rx--;
    if (y < 0 && y & 1) rx--;
    unsigned int ry = (x / 4) - (y / 4) + Y_OFFSET - ys * 32;
    if (x < 0 && x & 3) ry--;
    if (y < 0 && y & 3) ry++;

    rx += xoff;
    ry += yoff;

    // Flush batch if full
    if (app.batch_count >= MAX_BATCH_SIZE) {
        sdl_batch_flush();
    }

    // Pack effect flags into integer
    float gamma_scale = app_state.gamma / 5000.0f;
    float shade_effect, gamma_effect;
    bool grey, infra, water, red, green, invis;
    calculate_gamma_shader_params(effect, &shade_effect, &gamma_effect, &grey, &infra, &water, &red, &green, &invis);

    int flags = 0;
    if (red) flags |= (1 << 0); // FLAG_RED
    if (green) flags |= (1 << 1); // FLAG_GREEN
    if (invis) flags |= (1 << 2); // FLAG_INVIS
    if (grey) flags |= (1 << 3); // FLAG_GREY
    if (infra) flags |= (1 << 4); // FLAG_INFRA
    if (water) flags |= (1 << 5); // FLAG_WATER

    // Add sprite to batch
    SpriteInstance *inst = &app.batch_buffer[app.batch_count++];
    create_model_matrix(inst->model, rx, ry, sprite_data[nr].pixel_width, sprite_data[nr].pixel_height);
    inst->uv0[0] = sprite_data[nr].uv0.u;
    inst->uv0[1] = sprite_data[nr].uv0.v;
    inst->uv1[0] = sprite_data[nr].uv1.u;
    inst->uv1[1] = sprite_data[nr].uv1.v;
    inst->gamma_scale = gamma_scale;
    inst->shade_effect = shade_effect;
    inst->gamma_effect = gamma_effect;
    inst->flags = flags;
}

void sdl_copyspritex(int nr, int x, int y, int effect) {
    if (nr == 0) return;

    sdl_load_sprite(nr);
    if (!sprite_data[nr].gl_texture) return;

    // Only batch sprites that are in the atlas
    if (!sprite_data[nr].loaded_in_atlas) {
        // Fall back to immediate rendering for non-atlas sprites
        return;
    }

    // Initialize projection matrix once
    if (!projection_initialized) {
        create_ortho_matrix(projection_matrix, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);
        projection_initialized = 1;
    }

    // Flush batch if full
    if (app.batch_count >= MAX_BATCH_SIZE) {
        sdl_batch_flush();
    }

    // Pack effect flags into integer
    float gamma_scale = app_state.gamma / 5000.0f;
    float shade_effect, gamma_effect;
    bool grey, infra, water, red, green, invis;
    calculate_gamma_shader_params(effect, &shade_effect, &gamma_effect, &grey, &infra, &water, &red, &green, &invis);

    int flags = 0;
    if (red) flags |= (1 << 0); // FLAG_RED
    if (green) flags |= (1 << 1); // FLAG_GREEN
    if (invis) flags |= (1 << 2); // FLAG_INVIS
    if (grey) flags |= (1 << 3); // FLAG_GREY
    if (infra) flags |= (1 << 4); // FLAG_INFRA
    if (water) flags |= (1 << 5); // FLAG_WATER

    // Add sprite to batch (x, y are already screen coordinates)
    SpriteInstance *inst = &app.batch_buffer[app.batch_count++];
    create_model_matrix(inst->model, x, y, sprite_data[nr].pixel_width, sprite_data[nr].pixel_height);
    inst->uv0[0] = sprite_data[nr].uv0.u;
    inst->uv0[1] = sprite_data[nr].uv0.v;
    inst->uv1[0] = sprite_data[nr].uv1.u;
    inst->uv1[1] = sprite_data[nr].uv1.v;
    inst->gamma_scale = gamma_scale;
    inst->shade_effect = shade_effect;
    inst->gamma_effect = gamma_effect;
    inst->flags = flags;
}

void sdl_putc(int xpos, int ypos, int font, int c) {
    if (c > 127 || c < 32) return;
    c -= 32;

    // Flush batch before rendering text (text uses legacy uniform path)
    sdl_batch_flush();

    // Create OpenGL texture for this character if needed
    if (!font_cache[font].char_textures[c]) {
        int nr = 18100 + font;
        sdl_load_sprite(nr);

        // Extract character from font sprite
        SDL_Surface *char_surf = SDL_CreateRGBSurfaceWithFormat(0, 6, 9, 32, SPRITE_PIXEL_FORMAT);
        SDL_LockSurface(sprite_data[nr].surface);
        SDL_LockSurface(char_surf);

        for (int y = 0; y < 9; y++) {
            Uint32 *src_row = (Uint32 *) sprite_data[nr].surface->pixels + (y + 1) * sprite_data[nr].surface->pitch / 4
                              + c * 6;
            Uint32 *dst_row = (Uint32 *) char_surf->pixels + y * char_surf->pitch / 4;
            memcpy(dst_row, src_row, 6 * sizeof(Uint32));
        }

        SDL_UnlockSurface(char_surf);
        SDL_UnlockSurface(sprite_data[nr].surface);

        // Create OpenGL texture from surface
        GLuint gl_texture;
        glGenTextures(1, &gl_texture);
        glBindTexture(GL_TEXTURE_2D, gl_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 6, 9, 0, GL_BGRA, GL_UNSIGNED_BYTE, char_surf->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        font_cache[font].char_textures[c] = gl_texture;
        SDL_FreeSurface(char_surf);
    }

    sdl_start_scaling();
    // Render character using OpenGL (legacy uniform path)
    glUseProgram(gamma_shader);
    glUniform1i(gamma_uUseInstancing, 0); // Disable instanced rendering mode
    glUniformMatrix4fv(gamma_uProjection, 1, GL_FALSE, projection_matrix);

    float model_matrix[16];
    create_model_matrix(model_matrix, xpos, ypos, 6, 9);
    glUniformMatrix4fv(gamma_uModel, 1, GL_FALSE, model_matrix);
    glUniform1i(gamma_uTexture, 0);

    float gamma_scale = app_state.gamma / 5000.0f;
    glUniform1f(gamma_uGammaScale, gamma_scale);
    glUniform2f(gamma_uUV0, 0.0f, 0.0f);
    glUniform2f(gamma_uUV1, 1.0f, 1.0f);
    // Reset all effect uniforms to prevent bleeding from previous sprites
    glUniform1f(gamma_uShadeEffect, 0.0f);
    glUniform1f(gamma_uGammaEffect, 0.0f);
    glUniform1i(gamma_uRed, 0);
    glUniform1i(gamma_uGreen, 0);
    glUniform1i(gamma_uInvis, 0);
    glUniform1i(gamma_uGrey, 0);
    glUniform1i(gamma_uInfra, 0);
    glUniform1i(gamma_uWater, 0);
    glUniform1i(gamma_uShadow, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font_cache[font].char_textures[c]);
    glBindVertexArray(app.quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    sdl_stop_scaling();
}

void sdl_gputc(int xpos, int ypos, int font, int c) {
    if (c > 127 || c < 32) return;
    c -= 32;

    // Flush batch before rendering text (text uses legacy uniform path)
    sdl_batch_flush();

    // Windowed mode?
    ypos += 4;

    // Create OpenGL texture for this character if needed
    if (!font_cache[font].char_textures[c]) {
        int nr = 18100 + font;
        sdl_load_sprite(nr);

        // Extract character from font sprite
        SDL_Surface *char_surf = SDL_CreateRGBSurfaceWithFormat(0, 6, 9, 32, SPRITE_PIXEL_FORMAT);
        SDL_LockSurface(sprite_data[nr].surface);
        SDL_LockSurface(char_surf);

        for (int y = 0; y < 9; y++) {
            Uint32 *src_row = (Uint32 *) sprite_data[nr].surface->pixels + (y + 1) * sprite_data[nr].surface->pitch / 4
                              + c * 6;
            Uint32 *dst_row = (Uint32 *) char_surf->pixels + y * char_surf->pitch / 4;
            memcpy(dst_row, src_row, 6 * sizeof(Uint32));
        }

        SDL_UnlockSurface(char_surf);
        SDL_UnlockSurface(sprite_data[nr].surface);

        // Create OpenGL texture from surface
        GLuint gl_texture;
        glGenTextures(1, &gl_texture);
        glBindTexture(GL_TEXTURE_2D, gl_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 6, 9, 0, GL_BGRA, GL_UNSIGNED_BYTE, char_surf->pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        font_cache[font].char_textures[c] = gl_texture;
        SDL_FreeSurface(char_surf);
    }

    sdl_start_scaling();
    // Render character using OpenGL (legacy uniform path)
    glUseProgram(gamma_shader);
    glUniform1i(gamma_uUseInstancing, 0); // Disable instanced rendering mode
    glUniformMatrix4fv(gamma_uProjection, 1, GL_FALSE, projection_matrix);

    float model_matrix[16];
    create_model_matrix(model_matrix, xpos, ypos, 6, 9);
    glUniformMatrix4fv(gamma_uModel, 1, GL_FALSE, model_matrix);
    glUniform1i(gamma_uTexture, 0);

    float gamma_scale = app_state.gamma / 5000.0f;
    glUniform1f(gamma_uGammaScale, gamma_scale);
    glUniform2f(gamma_uUV0, 0.0f, 0.0f);
    glUniform2f(gamma_uUV1, 1.0f, 1.0f);

    // Reset all effect uniforms to prevent bleeding from previous sprites
    glUniform1f(gamma_uShadeEffect, 0.0f);
    glUniform1f(gamma_uGammaEffect, 0.0f);
    glUniform1i(gamma_uRed, 0);
    glUniform1i(gamma_uGreen, 0);
    glUniform1i(gamma_uInvis, 0);
    glUniform1i(gamma_uGrey, 0);
    glUniform1i(gamma_uInfra, 0);
    glUniform1i(gamma_uWater, 0);
    glUniform1i(gamma_uShadow, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font_cache[font].char_textures[c]);
    glBindVertexArray(app.quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    sdl_stop_scaling();
}

void sdl_puttext(int x, int y, int font, char *text) {
    while (*text) {
        sdl_putc(x, y, (char) font, *text);
        text++;
        x += 6;
    }
}

void sdl_gputtext(int x, int y, int font, char *text, int xoff, int yoff) {
    int text_length = strlen(text);

    unsigned int rx = (x / 2) + (y / 2) + 32 - ((text_length * 5) / 2) + X_OFFSET - ((RENDER_DISTANCE - 34) / 2 * 32);
    if (x < 0 && (x & 1)) rx--;
    if (y < 0 && (y & 1)) rx--;
    unsigned int ry = (x / 4) - (y / 4) + Y_OFFSET - 64;
    if (x < 0 && (x & 3)) ry--;
    if (y < 0 && (y & 3)) ry++;

    rx += xoff;
    ry += yoff;

    while (*text) {
        if (rx < 0 || rx > SCREEN_WIDTH - 7 || ry < 0 || ry > SCREEN_HEIGHT - 10) return;
        sdl_gputc(rx, ry, font, *text);
        text++;
        rx += 6;
    }
}

void sdl_xputtext(int x, int y, int font, char *format, va_list args) {
    char buf[1024];
    vsnprintf(buf, sizeof(buf), format, args);
    sdl_puttext(x, y, font, buf);
}

void sdl_showbox(int xf, int yf, int xs, int ys, unsigned short col) {
    // Flush batch before rendering UI primitives
    sdl_batch_flush();

    // Convert RGB565 color to float RGBA
    float r = ((col & 0xF800) >> 11) / 31.0f;
    float g = ((col & 0x07E0) >> 5) / 63.0f;
    float b = (col & 0x001F) / 31.0f;

    sdl_start_scaling();
    glUseProgram(solid_shader);
    glUniformMatrix4fv(solid_uProjection, 1, GL_FALSE, projection_matrix);
    glUniform4f(solid_uColor, r, g, b, 1.0f);

    // Draw 4 rectangles to form the outline (1 pixel thick)
    float model_matrix[16];

    // Top edge
    create_model_matrix(model_matrix, xf, yf, xs, 1);
    glUniformMatrix4fv(solid_uModel, 1, GL_FALSE, model_matrix);
    glBindVertexArray(app.quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Bottom edge
    create_model_matrix(model_matrix, xf, yf + ys - 1, xs, 1);
    glUniformMatrix4fv(solid_uModel, 1, GL_FALSE, model_matrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Left edge
    create_model_matrix(model_matrix, xf, yf, 1, ys);
    glUniformMatrix4fv(solid_uModel, 1, GL_FALSE, model_matrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Right edge
    create_model_matrix(model_matrix, xf + xs - 1, yf, 1, ys);
    glUniformMatrix4fv(solid_uModel, 1, GL_FALSE, model_matrix);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    sdl_stop_scaling();
}

void sdl_showbar(int xf, int yf, int xs, int ys, unsigned short col) {
    // Flush batch before rendering UI primitives
    sdl_batch_flush();

    // Convert RGB565 color to float RGBA
    float r = ((col & 0xF800) >> 11) / 31.0f;
    float g = ((col & 0x07E0) >> 5) / 63.0f;
    float b = (col & 0x001F) / 31.0f;

    sdl_start_scaling();
    glUseProgram(solid_shader);
    glUniformMatrix4fv(solid_uProjection, 1, GL_FALSE, projection_matrix);
    glUniform4f(solid_uColor, r, g, b, 1.0f);

    // Draw filled rectangle
    float model_matrix[16];
    create_model_matrix(model_matrix, xf, yf, xs, ys);
    glUniformMatrix4fv(solid_uModel, 1, GL_FALSE, model_matrix);

    glBindVertexArray(app.quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    sdl_stop_scaling();
}

void sdl_shadow(int nr, int xpos, int ypos, int xoff, int yoff) {
    // Check sprite range
    int ok = 0, disp = 14;
    if ((nr >= 2000 && nr < 16336) || nr > 17360) ok = 1;
    if (!ok) return;

    sdl_load_sprite(nr);
    if (!sprite_data[nr].gl_texture) return; // Check if sprite is loaded

    // Flush batch before rendering shadows (shadows use legacy uniform path)
    sdl_batch_flush();

    // Convert world coords to screen coords (same isometric math as copysprite)
    int xs = sprite_data[nr].xs;
    int ys = sprite_data[nr].ys;
    int rx = (xpos / 2) + (ypos / 2) - (xs * 16) + 32 + X_OFFSET -
             ((RENDER_DISTANCE - 34) / 2 * 32);
    if (xpos < 0 && (xpos & 1)) rx--;
    if (ypos < 0 && (ypos & 1)) rx--;
    int ry = (xpos / 4) - (ypos / 4) + Y_OFFSET - ys * 32;
    if (xpos < 0 && (xpos & 3)) ry--;
    if (ypos < 0 && (ypos & 3)) ry++;

    rx += xoff;
    ry += yoff;
    ry += ys * 32 - disp; // Offset shadow downward

    sdl_start_scaling();
    // Render shadow using shader (no texture creation needed!)
    glUseProgram(gamma_shader);
    glUniform1i(gamma_uUseInstancing, 0); // Disable instanced rendering mode (legacy path)
    glUniformMatrix4fv(gamma_uProjection, 1, GL_FALSE, projection_matrix);

    // Enable shadow mode in shader
    glUniform1i(gamma_uShadow, 1);

    // Compress vertically to 25% height (8 pixels per 32-pixel tile)
    float model_matrix[16];
    create_model_matrix(model_matrix, rx, ry, sprite_data[nr].pixel_width, ys * 8);
    glUniformMatrix4fv(gamma_uModel, 1, GL_FALSE, model_matrix);
    glUniform1i(gamma_uTexture, 0);

    // Use sprite's UV coordinates from atlas (flipped vertically for shadow)
    glActiveTexture(GL_TEXTURE0);
    if (sprite_data[nr].loaded_in_atlas) {
        glBindTexture(GL_TEXTURE_2D, tile_atlas);
        // Flip V coordinates: swap uv0.v and uv1.v
        glUniform2f(gamma_uUV0, sprite_data[nr].uv0.u, sprite_data[nr].uv1.v);
        glUniform2f(gamma_uUV1, sprite_data[nr].uv1.u, sprite_data[nr].uv0.v);
    } else {
        glBindTexture(GL_TEXTURE_2D, sprite_data[nr].gl_texture);
        // Flip V coordinates: 0 and 1 swapped
        glUniform2f(gamma_uUV0, 0.0f, 1.0f);
        glUniform2f(gamma_uUV1, 1.0f, 0.0f);
    }

    // No gamma adjustment for shadows
    glUniform1f(gamma_uGammaScale, 1.0f);

    // Reset all effect uniforms
    glUniform1f(gamma_uShadeEffect, 0.0f);
    glUniform1f(gamma_uGammaEffect, 0.0f);
    glUniform1i(gamma_uRed, 0);
    glUniform1i(gamma_uGreen, 0);
    glUniform1i(gamma_uInvis, 0);
    glUniform1i(gamma_uGrey, 0);
    glUniform1i(gamma_uInfra, 0);
    glUniform1i(gamma_uWater, 0);

    glBindVertexArray(app.quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Disable shadow mode for subsequent renders
    glUniform1i(gamma_uShadow, 0);
    sdl_stop_scaling();
}

static unsigned char *shadow_map = NULL;

void sdl_shadow_clear(void) {
    if (!shadow_map) {
        shadow_map = calloc(SCREEN_WIDTH * SCREEN_HEIGHT, 1);
    } else {
        memset(shadow_map, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
    }
}

/**
 * sdl_isvisible - Check if SDL window is visible and ready for rendering
 *
 * SDL2 EQUIVALENT OF dd_isvisible():
 * Unlike DirectDraw, SDL2 automatically handles video memory management
 * and surface restoration. Surfaces/textures never become "lost" in SDL2.
 * The driver manages video memory transparently, migrating it as needed.
 *
 * However, we still need to check if the window is in a state where
 * rendering makes sense:
 *
 * WINDOW STATES TO CHECK:
 * - SDL_WINDOW_MINIMIZED: Window is minimized to taskbar/dock
 *   → Skip rendering (not visible to user, waste of CPU/GPU)
 *
 * - SDL_WINDOW_HIDDEN: Window is explicitly hidden via SDL_HideWindow()
 *   → Skip rendering (window intentionally hidden)
 *
 * STATES NOT CHECKED:
 * - SDL_WINDOW_SHOWN: Default state, checked implicitly (not hidden/minimized)
 * - SDL_WINDOW_MAXIMIZED: Still visible, render normally
 * - SDL_WINDOW_INPUT_FOCUS: Not relevant (can render without focus)
 * - SDL_WINDOW_MOUSE_FOCUS: Not relevant (can render without mouse over it)
 *
 * PERFORMANCE NOTE:
 * On some platforms, SDL may pause rendering when minimized automatically.
 * This check provides explicit control and avoids wasted CPU cycles.
 *
 * USAGE:
 * Called at the start of each frame (engine.c:show_screen()) before rendering:
 *   if (!dd_isvisible()) return;  // DirectDraw version
 *   if (!sdl_isvisible()) return; // SDL2 version
 *
 * Returns: 1 if window is visible and ready for rendering
 *          0 if window is minimized/hidden (skip rendering)
 */
int sdl_isvisible(void) {
    if (!app.window) return 0;

    Uint32 flags = SDL_GetWindowFlags(app.window);

    // Skip rendering if minimized or hidden
    if (flags & (SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN)) {
        return 0;
    }

    return 1;
}

void sdl_show_map(unsigned short *src, int xo, int yo, int magnify) {
    // Flush sprite batch before rendering minimap
    sdl_batch_flush();

    // Create OpenGL texture on first call
    if (!minimap_gl_texture) {
        glGenTextures(1, &minimap_gl_texture);
        glBindTexture(GL_TEXTURE_2D, minimap_gl_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    // Allocate CPU buffer for RGBA conversion (128x128x4 bytes)
    Uint32 *pixels = malloc(128 * 128 * sizeof(Uint32));
    if (!pixels) return;

    // Convert RGB555 source data to RGBA8888
    for (int y = 0; y < 128; y++) {
        for (int x = 0; x < 128; x++) {
            int src_x = (x / magnify) + xo;
            int src_y = (y / magnify) + yo;

            // Clamp to map bounds
            if (src_x < 0 || src_x >= MAPX_MAX ||
                src_y < 0 || src_y >= MAPY_MAX) {
                pixels[y * 128 + x] = 0xFF000000; // Black (opaque)
                continue;
            }

            // Read from source (transposed indexing: src[x_coord * MAPX_MAX + y_coord])
            unsigned short pixel555 = src[src_x * MAPX_MAX + src_y];

            // Convert RGB555 to RGB888
            // RGB555: 0RRRRRGGGGGBBBBB (high bit unused)
            Uint8 r = ((pixel555 >> 10) & 0x1F) << 3; // 5 bits -> 8 bits
            Uint8 g = ((pixel555 >> 5) & 0x1F) << 3; // 5 bits -> 8 bits
            Uint8 b = (pixel555 & 0x1F) << 3; // 5 bits -> 8 bits

            // Expand to full 8-bit range (replicate high bits to low bits)
            r |= (r >> 5); // Fill lower 3 bits
            g |= (g >> 5); // Fill lower 3 bits
            b |= (b >> 5); // Fill lower 3 bits

            // RGBA8888 format: 0xAABBGGRR (little-endian)
            pixels[x * 128 + y] = (0xFF << 24) | (b << 16) | (g << 8) | r;
        }
    }

    sdl_start_scaling();
    // Upload to GPU
    glBindTexture(GL_TEXTURE_2D, minimap_gl_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 128, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    free(pixels);

    // Render minimap using gamma shader (with all effects disabled)
    glUseProgram(gamma_shader);
    glUniform1i(gamma_uUseInstancing, 0); // Disable instanced rendering mode
    glUniformMatrix4fv(gamma_uProjection, 1, GL_FALSE, projection_matrix);

    // Create model matrix for position and size
    float model_matrix[16];
    create_model_matrix(model_matrix, 6, 582, 128, 128);
    glUniformMatrix4fv(gamma_uModel, 1, GL_FALSE, model_matrix);

    // Set texture and UV coordinates
    glUniform1i(gamma_uTexture, 0);
    glUniform2f(gamma_uUV0, 0.0f, 0.0f);
    glUniform2f(gamma_uUV1, 1.0f, 1.0f);

    // Disable all shader effects
    float gamma_scale = app_state.gamma / 5000.0f;
    glUniform1f(gamma_uGammaScale, gamma_scale);
    glUniform1f(gamma_uShadeEffect, 0.0f);
    glUniform1f(gamma_uGammaEffect, 0.0f);
    glUniform1i(gamma_uRed, 0);
    glUniform1i(gamma_uGreen, 0);
    glUniform1i(gamma_uInvis, 0);
    glUniform1i(gamma_uGrey, 0);
    glUniform1i(gamma_uInfra, 0);
    glUniform1i(gamma_uWater, 0);
    glUniform1i(gamma_uShadow, 0);

    // Bind minimap texture and render
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, minimap_gl_texture);
    glBindVertexArray(app.quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    sdl_stop_scaling();
}

int sdl_get_avgcol(int nr) {
    return sprite_data[nr].avgcol;
}

void sdl_start_scaling(void) {
    if (!app_state.windowed) {
        int w, h;
        SDL_GetWindowSize(app.window, &w, &h);
        glViewport(0, 0, w, h);
    }
}

void sdl_stop_scaling(void) {
    if (!app_state.windowed) {
        glViewport(0, 0, SCREEN_WIDTH,
                   SCREEN_HEIGHT);
    }
}
