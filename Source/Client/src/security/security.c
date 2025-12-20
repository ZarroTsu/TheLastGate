#include "security.h"

#include <SDL2/SDL_filesystem.h>
#include <stdint.h>

#include "log/log.h"

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#else
#include <sys/random.h>
#endif


static uint32_t get_seed() {
    uint32_t seed;

#if defined(_WIN32)
    BCryptGenRandom(NULL, (PUCHAR) &seed, sizeof(seed), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
#else
    getrandom(&seed, sizeof(seed), 0);
#endif
    return seed;
}

int unique1 = 0, unique2 = 0;

char *pref_path = NULL;

void security_init(void) {
    pref_path = SDL_GetPrefPath("TheLastGate", "TheLastGate");
    if (!pref_path) {
        log_error("Failed to get preferences path: %s", SDL_GetError());
        return;
    }
    srand(get_seed());
    log_info("Preferences directory: %s", pref_path);
}

void security_shutdown(void) {
    if (pref_path) {
        SDL_free(pref_path);
        pref_path = NULL;
    }
}
