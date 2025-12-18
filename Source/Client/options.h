#pragma once

#define SAVE_VERSION 5  /* TLGExtended.dat format version */

extern struct key okey;

void load_options(void);
void save_options(void);
void load_extended_options(void);
void save_extended_options(void);
