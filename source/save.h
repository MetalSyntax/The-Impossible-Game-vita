#ifndef SAVE_H
#define SAVE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int numbJumps;
    int numbAttempts;
    int medals[6];
    int progress_practice[2];
    int progress_noflag[2];
} GameSaveData;

extern GameSaveData g_save_data;

void save_init(void);
void save_write(void);

#ifdef __cplusplus
}
#endif

#endif // SAVE_H
