#ifndef AUDIO_H
#define AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SFX_BOOP = 0,
    SFX_EXPLOSION,
    SFX_FIREWORKS,
    SFX_MEDAL,
    SFX_COUNT
} SfxId;

typedef enum {
    BGM_NONE = 0,
    BGM_MENU,
    BGM_SOUNDTRACK,
    BGM_PRACTICE,
    BGM_SOUNDTRACK2, // Chaoz Fantasy (nivel 2, Level Pack)
    BGM_SOUNDTRACK3, // Heaven (nivel 3, Level Pack)
    BGM_SOUNDTRACK4, // Phazd (nivel 4, Level Pack)
    BGM_COUNT
} BgmTrack;

int audio_init(void);
void audio_shutdown(void);

void audio_play_music(BgmTrack track, int loop);
void audio_stop_music(void);
void audio_set_music_volume(float vol);
void audio_restart_music(void);

void audio_play_sfx(SfxId id);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H
