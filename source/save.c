#include "save.h"
#include "utils/logger.h"

#include <stdio.h>
#include <string.h>

#define SAVE_FILE_PATH DATA_PATH "save.dat"

GameSaveData g_save_data;

void save_init(void) {
    memset(&g_save_data, 0, sizeof(GameSaveData));

    FILE *f = fopen(SAVE_FILE_PATH, "rb");
    if (f) {
        size_t read = fread(&g_save_data, 1, sizeof(GameSaveData), f);
        fclose(f);
        if (read == sizeof(GameSaveData)) {
            l_info("Save data loaded successfully: jumps=%d, attempts=%d",
                   g_save_data.numbJumps, g_save_data.numbAttempts);
            return;
        }
    }
    l_info("No existing save data found, initialized to defaults.");
}

void save_write(void) {
    FILE *f = fopen(SAVE_FILE_PATH, "wb");
    if (f) {
        fwrite(&g_save_data, 1, sizeof(GameSaveData), f);
        fclose(f);
        l_info("Save data written to %s", SAVE_FILE_PATH);
    } else {
        l_error("Failed to open %s for writing!", SAVE_FILE_PATH);
    }
}
