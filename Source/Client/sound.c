#include "sound.h"

#include <math.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_audio.h>
#include <string.h>


int domusic = 0;
int dosound = 1;

// This should be a big enough cache for now.
#define MAX_CACHED_SOUNDS 64
typedef struct {
    char filename[256];
    Mix_Chunk *chunk;
} CachedSound;

static CachedSound sound_cache[MAX_CACHED_SOUNDS];
static int cache_count = 0;

// Track which chunks are playing on which channels for cleanup
static Mix_Chunk *channel_chunks[10] = {NULL};

// Callback to clear channel tracking when playback finishes
// Note: We don't free chunks here because they're cached
static void channel_finished(int channel) {
    if (channel >= 0 && channel < 10) {
        channel_chunks[channel] = NULL;
    }
}

int init_sound() {
    if (Mix_OpenAudio(44100, AUDIO_F32SYS, 2, 2048) < 0) return -1;
    Mix_AllocateChannels(10);
    Mix_ChannelFinished(channel_finished);

    // Initialize channel tracking
    for (int i = 0; i < 10; i++) {
        channel_chunks[i] = NULL;
    }

    cache_count = 0;

    return 0;
}

// Get a sound from cache or load it if not cached
static Mix_Chunk *get_cached_sound(const char *file) {
    // Search cache for existing entry
    for (int i = 0; i < cache_count; i++) {
        if (strcmp(sound_cache[i].filename, file) == 0) {
            return sound_cache[i].chunk;
        }
    }

    // Not in cache, load it
    Mix_Chunk *chunk = Mix_LoadWAV(file);
    if (!chunk) return NULL;

    // Add to cache if there's room
    if (cache_count < MAX_CACHED_SOUNDS) {
        strncpy(sound_cache[cache_count].filename, file, sizeof(sound_cache[cache_count].filename) - 1);
        sound_cache[cache_count].filename[sizeof(sound_cache[cache_count].filename) - 1] = '\0';
        sound_cache[cache_count].chunk = chunk;
        cache_count++;
    }
    // If cache is full, we still return the chunk but don't cache it
    // This is a fallback - ideally MAX_CACHED_SOUNDS should be large enough

    return chunk;
}

void ds_pan_to_sdl_mixer(int pan, Uint8 *l_out, Uint8 *r_out) {
    float left = 1.0f;
    float right = 1.0f;

    if (pan < 0) {
        // shift left → attenuate right
        float att = powf(10.0f, (pan / 100.0f) / 20.0f); // DS uses 0.01 dB units
        right = att;
    } else if (pan > 0) {
        // shift right → attenuate left
        float att = powf(10.0f, (-pan / 100.0f) / 20.0f);
        left = att;
    }

    // Convert 0.0–1.0 → 0–255
    *l_out = (Uint8) (left * 255.0f);
    *r_out = (Uint8) (right * 255.0f);
}

int play_sound(char *file, int vol, int p) {
    if (!dosound) return 0;

    // Get sound from cache (or load and cache it)
    Mix_Chunk *chunk = get_cached_sound(file);
    if (!chunk) return -1;

    int channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel < 0) {
        return -1;
    }

    // Track this chunk for reference (not for freeing, since it's cached)
    if (channel >= 0 && channel < 10) {
        channel_chunks[channel] = chunk;
    }

    // Set volume (convert from DirectSound decibels to SDL_mixer 0-128 range)
    float att = powf(10.0f, vol / 2000.0f);
    int sdl_volume = (int)(att * MIX_MAX_VOLUME);

    if (sdl_volume < 0) sdl_volume = 0;
    if (sdl_volume > MIX_MAX_VOLUME) sdl_volume = MIX_MAX_VOLUME;

    Mix_Volume(channel, sdl_volume);

    // Set panning
    Uint8 left, right;
    ds_pan_to_sdl_mixer(p, &left, &right);
    Mix_SetPanning(channel, left, right);

    return channel;
}

void cleanup_sound(void) {
    // Stop all channels
    Mix_HaltChannel(-1);

    // Clear channel tracking
    for (int i = 0; i < 10; i++) {
        channel_chunks[i] = NULL;
    }

    // Free all cached sound chunks
    for (int i = 0; i < cache_count; i++) {
        if (sound_cache[i].chunk) {
            Mix_FreeChunk(sound_cache[i].chunk);
            sound_cache[i].chunk = NULL;
        }
        sound_cache[i].filename[0] = '\0';
    }
    cache_count = 0;

    Mix_CloseAudio();
}
