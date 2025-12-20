#include "character_manager.h"

#include <fcntl.h>
#include <io.h>
#include <stddef.h>
#include <string.h>

#include "inter.h"
#include "launcher.h"
#include "options.h"
#include "log/log.h"

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

int class_gender_to_okey(const int class, int gender) {
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

    if (class < 0 || class >= 12) return 4; /* Default to Templar M */
    if (gender < 0 || gender > 1) gender = 0;

    return race_map[class][gender];
}
