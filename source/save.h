#ifndef SAVE_H
#define SAVE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int numbJumps;
    int numbAttempts;
    int medals[12];
    int progress_practice[5];
    int progress_noflag[5];
} GameSaveData;

extern GameSaveData g_save_data;

void save_init(void);
void save_write(void);

#ifdef __cplusplus
}
#endif

#endif // SAVE_H
