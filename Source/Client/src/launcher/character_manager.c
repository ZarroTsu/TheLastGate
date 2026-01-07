#include "character_manager.h"

#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>

#include "inter.h"
#include "launcher.h"
#include "options.h"
#include "log/log.h"
#include "config/config.h"

void load_character_from_file(const char *file_path) {
    char buf[250];
    int file = open(file_path, O_RDONLY | O_BINARY);
    int flag = 0;
    if (file == -1) {
        log_error("Could not open file %s", file_path);
        snprintf(buf, sizeof(buf), "Could not open file %s", file_path);
        launcher_set_fyi(buf);
        return;
    }

    if (lseek(file, 0, SEEK_END) > (long) (sizeof(struct pdata) + sizeof(struct key))) flag = 1;
    lseek(file, 0, SEEK_SET);
    read(file, &okey, sizeof(struct key));
    if (read(file, &pdata, sizeof(struct pdata)) != sizeof(struct pdata) || flag) {
        pdata.hide = 1;
        pdata.show_names = 1;
        pdata.show_proz = 1;
        pdata.show_stats = 0;
        pdata.show_bars = 0;
        pdata.cname[0] = 0;
        pdata.ref[0] = 0;
        pdata.desc[0] = 0;
        pdata.changed = 0;

        for (int i = 0; i < 20; i++) {
            pdata.xbutton[i].skill_nr = -1;
            strcpy(pdata.xbutton[i].name, "-");
        }
    }

    add_to_previous_characters(okey.name, file_path);

    close(file);
    pdata.changed = 1;
}

void save_character_to_file(const char *file_path) {
    char buf[250];

    int file = open(file_path, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, 0600);
    if (file == -1) {
        log_error("Could not save file %s", file_path);
        snprintf(buf, sizeof(buf), "Could not save file %s", file_path);
        launcher_set_fyi(buf);
        return;
    }

    write(file, &okey, sizeof(struct key));
    write(file, &pdata, sizeof(struct pdata));
    close(file);

    add_to_previous_characters(okey.name, file_path);

    log_info("Saved file as %s", file_path);
    snprintf(buf, sizeof(buf), "Saved file as %s", file_path);
    launcher_set_fyi(buf);
}

void create_new_character() {
    okey.usnr = 0;
    okey.race = 0;
    okey.pass1 = 0;
    okey.pass2 = 0;
    pdata.changed = 1;
    strcpy(okey.name, "New Account");
    pdata.cname[0] = 0;
}

void okey_to_class_gender(int *out_class, int *out_gender) {
    int class = 0;
    int gender = 0;
    switch (okey.race) {
        case 4: // templar M
            class = 0;
            gender = 0;
            break;
        case 5: // templar F
            class = 0;
            gender = 1;
            break;

        case 6: // mercenary M
            class = 1;
            gender = 0;
            break;
        case 7: // mercenary F
            class = 1;
            gender = 1;
            break;

        case 8: // harakim M
            class = 2;
            gender = 0;
            break;
        case 9: // harakim F
            class = 2;
            gender = 1;
            break;

        case 10: // seyan M
            class = 6;
            gender = 0;
            break;
        case 11: // seyan F
            class = 6;
            gender = 1;
            break;


        case 12: // arch templar M
            class = 3;
            gender = 0;
            break;
        case 13: // arch templar F
            class = 3;
            gender = 1;
            break;

        case 14: // pugilist M
            class = 4;
            gender = 0;
            break;
        case 15: // pugilist F
            class = 4;
            gender = 1;
            break;

        case 16: // warrior M
            class = 5;
            gender = 0;
            break;
        case 17: // warrior F
            class = 5;
            gender = 1;
            break;

        case 18: // sorcerer M
            class = 7;
            gender = 0;
            break;
        case 19: // sorcerer F
            class = 7;
            gender = 1;
            break;

        case 20: // summoner M
            class = 8;
            gender = 0;
            break;
        case 21: // summoner F
            class = 8;
            gender = 1;
            break;

        case 22: // arch harakim M
            class = 9;
            gender = 0;
            break;
        case 23: // arch harakim F
            class = 9;
            gender = 1;
            break;

        //

        case 2:
            class = 10;
            gender = 0;
            break; // braver M
        case 3:
            class = 10;
            gender = 1;
            break; // braver F

        case 1554: class = 11;
            gender = 0;
            break;

        default: class = 0;
            gender = 0;
            break;
    }

    if (out_class != NULL) *out_class = class;
    if (out_gender != NULL) *out_gender = gender;
}

int class_gender_to_okey(const int race, int gender) {
    static const int race_map[12][2] = {
        {4, 5}, /* 0: Templar M/F */
        {6, 7}, /* 1: Mercenary M/F */
        {8, 9}, /* 2: Harakim M/F */
        {12, 13}, /* 3: Arch-Templar M/F */
        {14, 15}, /* 4: Skald/Pugilist M/F */
        {16, 17}, /* 5: Warrior M/F */
        {10, 11}, /* 6: Seyan M/F */
        {18, 19}, /* 7: Sorcerer M/F */
        {20, 21}, /* 8: Summoner M/F */
        {22, 23}, /* 9: Arch-Harakim M/F */
        {2, 3}, /* 10: Braver M/F */
        {1554, 1554} /* 11: Lycanthrope (M only) */
    };

    if (race < 0 || race >= 12) return 4; /* Default to Templar M */
    if (gender < 0 || gender > 1) gender = 0;

    return race_map[race][gender];
}

static PreviousCharactersList g_previous_characters = {0};

const PreviousCharactersList* get_previous_characters(void) {
    return &g_previous_characters;
}

void add_to_previous_characters(const char *name, const char *file_path) {
    int i;
    int found_index = -1;

    if (name == NULL || file_path == NULL) {
        return;
    }

    /* Search for existing entry with matching file_path */
    for (i = 0; i < g_previous_characters.count; i++) {
        if (strcmp(g_previous_characters.entries[i].file_path, file_path) == 0) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        /* Entry exists - update name if different and move to front */
        PreviousCharacterEntry temp = g_previous_characters.entries[found_index];

        /* Update name */
        strncpy(temp.name, name, sizeof(temp.name) - 1);
        temp.name[sizeof(temp.name) - 1] = '\0';

        /* Shift entries down to make room at index 0 */
        for (i = found_index; i > 0; i--) {
            g_previous_characters.entries[i] = g_previous_characters.entries[i - 1];
        }

        /* Place at front */
        g_previous_characters.entries[0] = temp;
    } else {
        /* Entry doesn't exist - add new entry */
        PreviousCharacterEntry new_entry;

        /* Copy name and file_path */
        strncpy(new_entry.name, name, sizeof(new_entry.name) - 1);
        new_entry.name[sizeof(new_entry.name) - 1] = '\0';

        strncpy(new_entry.file_path, file_path, sizeof(new_entry.file_path) - 1);
        new_entry.file_path[sizeof(new_entry.file_path) - 1] = '\0';

        /* If list is full, we'll overwrite the last entry */
        if (g_previous_characters.count >= MAX_PREVIOUS_CHARACTERS) {
            /* Shift all entries down (oldest one at end will be lost) */
            for (i = MAX_PREVIOUS_CHARACTERS - 1; i > 0; i--) {
                g_previous_characters.entries[i] = g_previous_characters.entries[i - 1];
            }
            g_previous_characters.entries[0] = new_entry;
            /* count stays at MAX_PREVIOUS_CHARACTERS */
        } else {
            /* List not full - shift entries down and increment count */
            for (i = g_previous_characters.count; i > 0; i--) {
                g_previous_characters.entries[i] = g_previous_characters.entries[i - 1];
            }
            g_previous_characters.entries[0] = new_entry;
            g_previous_characters.count++;
        }
    }
}

void remove_from_previous_characters(const char *file_path) {
    int i;
    int found_index = -1;

    if (file_path == NULL) {
        return;
    }

    /* Search for entry with matching file_path */
    for (i = 0; i < g_previous_characters.count; i++) {
        if (strcmp(g_previous_characters.entries[i].file_path, file_path) == 0) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        /* Shift all entries after it up one position */
        for (i = found_index; i < g_previous_characters.count - 1; i++) {
            g_previous_characters.entries[i] = g_previous_characters.entries[i + 1];
        }
        g_previous_characters.count--;
    }
}

static void cleanup_invalid_paths(void) {
    int i;
    int removed_any = 0;
    struct stat st;

    /* Iterate backwards so removal doesn't affect iteration */
    for (i = g_previous_characters.count - 1; i >= 0; i--) {
        /* Check if file exists using stat (cross-platform) */
        if (stat(g_previous_characters.entries[i].file_path, &st) != 0) {
            /* File doesn't exist - remove this entry */
            log_info("Removing invalid character path: %s", g_previous_characters.entries[i].file_path);
            remove_from_previous_characters(g_previous_characters.entries[i].file_path);
            removed_any = 1;
        }
    }

    /* If we removed any entries, save the cleaned list */
    if (removed_any) {
        save_previous_characters();
    }
}

void save_previous_characters(void) {
    int handle;
    unsigned int version = 1;
    int i;
    char filepath[512];

    if (!g_config.runtime.pref_path[0]) {
        log_error("Preferences path not initialized");
        return;
    }

    snprintf(filepath, sizeof(filepath), "%sTLGCharacters.dat", g_config.runtime.pref_path);

    handle = open(filepath, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, 0666);
    if (handle == -1) {
        log_error("Could not save %s", filepath);
        return;
    }

    /* Write version */
    if (write(handle, &version, sizeof(version)) != sizeof(version)) {
        log_error("Failed to write version to %s", filepath);
        close(handle);
        return;
    }

    /* Write count */
    if (write(handle, &g_previous_characters.count, sizeof(g_previous_characters.count)) != sizeof(g_previous_characters.count)) {
        log_error("Failed to write count to %s", filepath);
        close(handle);
        return;
    }

    /* Write entries */
    for (i = 0; i < g_previous_characters.count; i++) {
        if (write(handle, &g_previous_characters.entries[i], sizeof(PreviousCharacterEntry)) != sizeof(PreviousCharacterEntry)) {
            log_error("Failed to write entry %d to %s", i, filepath);
            close(handle);
            return;
        }
    }

    close(handle);
    log_info("Saved %d previous characters to %s", g_previous_characters.count, filepath);
}

void load_previous_characters(void) {
    int handle;
    unsigned int version;
    unsigned int count;
    int i;
    char filepath[512];

    /* Initialize to empty state */
    g_previous_characters.count = 0;
    memset(g_previous_characters.entries, 0, sizeof(g_previous_characters.entries));

    if (!g_config.runtime.pref_path[0]) {
        log_error("Preferences path not initialized");
        return;
    }

    snprintf(filepath, sizeof(filepath), "%sTLGCharacters.dat", g_config.runtime.pref_path);

    handle = open(filepath, O_RDONLY | O_BINARY);
    if (handle == -1) {
        /* File doesn't exist yet - not an error for first run */
        log_info("%s not found, starting with empty list", filepath);
        return;
    }

    /* Read version */
    if (read(handle, &version, sizeof(version)) != sizeof(version)) {
        log_error("Failed to read version from %s", filepath);
        close(handle);
        return;
    }

    /* Validate version */
    if (version < 1) {
        log_error("Invalid version in %s: %u", filepath, version);
        close(handle);
        return;
    }

    /* Read count */
    if (read(handle, &count, sizeof(count)) != sizeof(count)) {
        log_error("Failed to read count from %s", filepath);
        close(handle);
        return;
    }

    /* Validate count */
    if (count > MAX_PREVIOUS_CHARACTERS) {
        log_error("Invalid count in %s: %u (max %d)", filepath, count, MAX_PREVIOUS_CHARACTERS);
        close(handle);
        return;
    }

    /* Read entries */
    for (i = 0; i < (int)count; i++) {
        if (read(handle, &g_previous_characters.entries[i], sizeof(PreviousCharacterEntry)) != sizeof(PreviousCharacterEntry)) {
            log_error("Failed to read entry %d from %s", i, filepath);
            close(handle);
            return;
        }
    }

    g_previous_characters.count = count;
    close(handle);
    log_info("Loaded %d previous characters from %s", g_previous_characters.count, filepath);
}

void init_previous_characters(void) {
    load_previous_characters();
    cleanup_invalid_paths();
}
