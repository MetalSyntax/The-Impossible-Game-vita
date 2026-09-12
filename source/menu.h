#ifndef MENU_H
#define MENU_H

#include <vitasdk.h>
#include "texture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STATE_MAIN_MENU = 0,
    STATE_LEVEL_SELECT = 1,
    STATE_PAUSE = 2,
    STATE_HOW_TO_PLAY = 3,
    STATE_STATS = 4,
    STATE_MEDALS = 5,
    STATE_VICTORY = 6,
    STATE_GAMEPLAY = 7
} GameState;

int menu_init(void);
void menu_shutdown(void);

void menu_set_state(GameState state);
GameState menu_get_state(void);

// Returns 1 if state changed to GAMEPLAY and a level should be started (out_level set)
// Returns 0 otherwise
int menu_update(SceCtrlData *pad, SceCtrlData *old_pad, SceTouchData *touch, int *out_level);
void menu_render(void);

#ifdef __cplusplus
}
#endif

#endif // MENU_H
