#pragma once

void load_save_from_file(const char *file_path);
void save_to_file(const char *file_path);
void create_new_character();

void okey_to_class_gender(int *out_class, int *out_gender);

int class_gender_to_okey(int class, int gender);