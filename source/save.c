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
            // libimpossible.so only ever stores 1 in a medal slot; anything else is
            // corruption from older builds and would fire retroactive trophies.
            for (int i = 0; i < 12; i++) {
                if (g_save_data.medals[i] != 0 && g_save_data.medals[i] != 1) {
                    l_warn("Save medal[%d]=%d corrupt, reset to 0", i, g_save_data.medals[i]);
                    g_save_data.medals[i] = 0;
                }
            }
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
