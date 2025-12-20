#include <time.h>

#include "security.h"
#include "config/config.h"
#include "log/log.h"

#ifndef _WIN32
#include <sys/stat.h>
#endif
#include <stdio.h>
#include <stdlib.h>

void security_id_load(void) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%sunique.dat", g_config.runtime.pref_path);

    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        log_info("No existing unique ID found, generating new one");
        unique1 = (rand() << 16) + rand();
        unique2 = (rand() << 16) + rand();
        if (unique1 == 0) unique1 = 1;
        if (unique2 == 0) unique2 = 1;
        security_id_save();
        return;
    }

    size_t read1 = fread(&unique1, sizeof(unsigned int), 1, fp);
    size_t read2 = fread(&unique2, sizeof(unsigned int), 1, fp);
    fclose(fp);

    if (read1 != 1 || read2 != 1) {
        log_warning("Failed to read unique ID, generating new one");
        unique1 = (rand() << 16) + rand();
        unique2 = (rand() << 16) + rand();
        if (unique1 == 0) unique1 = 1;
        if (unique2 == 0) unique2 = 1;
        security_id_save();
    } else {
        log_info("Loaded unique ID: %08X-%08X", unique1, unique2);
    }
}

void security_id_save(void) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%sunique.dat", g_config.runtime.pref_path);

    FILE *fp = fopen(filepath, "wb");
    if (!fp) {
        log_error("Failed to save unique ID to %s", filepath);
        return;
    }

    fwrite(&unique1, sizeof(unsigned int), 1, fp);
    fwrite(&unique2, sizeof(unsigned int), 1, fp);
    fclose(fp);

    log_info("Saved unique ID to %s", filepath);
}

void security_id_get(unsigned int *out_id1, unsigned int *out_id2) {
    if (out_id1) *out_id1 = unique1;
    if (out_id2) *out_id2 = unique2;
}

void security_id_set(unsigned int id1, unsigned int id2) {
    unique1 = id1;
    unique2 = id2;
    security_id_save();
}
