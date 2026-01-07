#pragma once

#define SAVE_VERSION 6  /* TLGExtended.dat format version */

EXTERN_C_BEGIN
extern struct key okey;

void load_options(void);
void save_options(void);
void load_extended_options(void);
void save_extended_options(void);
EXTERN_C_END