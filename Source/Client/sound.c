#include "sound.h"

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_audio.h>


int domusic = 0;
int dosound = 1;

// Track which chunks are playing on which channels for cleanup
static Mix_Chunk *channel_chunks[10] = {NULL};

// Callback to free sound chunks when they finish playing
static void channel_finished(int channel) {
    if (channel >= 0 && channel < 10 && channel_chunks[channel]) {
        Mix_FreeChunk(channel_chunks[channel]);
        channel_chunks[channel] = NULL;
    }
}

int init_sound() {
    if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) < 0) return -1;
    Mix_AllocateChannels(10);
    Mix_ChannelFinished(channel_finished);

    // Initialize channel tracking
    for (int i = 0; i < 10; i++) {
        channel_chunks[i] = NULL;
    }

    return 0;
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

    Mix_Chunk *chunk = Mix_LoadWAV(file);
    if (!chunk) return -1;

    int channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel < 0) {
        // Failed to play - free the chunk immediately
        Mix_FreeChunk(chunk);
        return -1;
    }

    // Track this chunk so it can be freed when playback finishes
    if (channel >= 0 && channel < 10) {
        // Free any previous chunk on this channel (shouldn't happen, but safety first)
        if (channel_chunks[channel]) {
            Mix_FreeChunk(channel_chunks[channel]);
        }
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
    // Stop all channels and free any remaining chunks
    Mix_HaltChannel(-1);
    for (int i = 0; i < 10; i++) {
        if (channel_chunks[i]) {
            Mix_FreeChunk(channel_chunks[i]);
            channel_chunks[i] = NULL;
        }
    }
    Mix_CloseAudio();
}
