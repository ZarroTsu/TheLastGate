#pragma once
#include <stdbool.h>

extern char *pref_path;

/* Client uniqueness values */
extern int unique1;
extern int unique2;

/* Initializes common security based functionality */
void init_security(void);
void shutdown_security(void);

/* Methods for ensuring a single instance is running */
bool security_try_lock();
bool security_acquire_lock(const char *path);
void security_release_lock(void);

/* Methods for tagging client uniqueness */
void security_id_load(void);
void security_id_save(void);
void security_id_get(unsigned int *out_id1, unsigned int *out_id2);
void security_id_set(unsigned int id1, unsigned int id2);