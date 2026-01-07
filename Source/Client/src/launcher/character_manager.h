#pragma once

EXTERN_C_BEGIN
#define MAX_PREVIOUS_CHARACTERS 20

typedef struct {
    char name[40];
    char file_path[512];
} PreviousCharacterEntry;

typedef struct {
    PreviousCharacterEntry entries[MAX_PREVIOUS_CHARACTERS];
    int count;
} PreviousCharactersList;

void load_character_from_file(const char *file_path);
void save_character_to_file(const char *file_path);
void create_new_character();

void okey_to_class_gender(int *out_class, int *out_gender);

int class_gender_to_okey(int race, int gender);

/* Previous Characters List Management */
void init_previous_characters(void);
void add_to_previous_characters(const char *name, const char *file_path);
void remove_from_previous_characters(const char *file_path);
void save_previous_characters(void);
void load_previous_characters(void);
const PreviousCharactersList* get_previous_characters(void);
EXTERN_C_END