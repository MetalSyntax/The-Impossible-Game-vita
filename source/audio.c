#include "audio.h"
#include "utils/logger.h"

#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include stb_vorbis
#include "../lib/stb_vorbis.c"

#define AUDIO_SAMPLES 1024
#define AUDIO_RATE 44100
#define MAX_ACTIVE_SFX 8

typedef struct {
    short *data;
    int num_samples; // total short samples (frames * channels)
    int channels;
    int sample_rate;
} DecodedSfx;

typedef struct {
    DecodedSfx *sfx;
    int cursor;
    int active;
} ActiveSfx;

static DecodedSfx g_sfx[SFX_COUNT];
static ActiveSfx g_active_sfx[MAX_ACTIVE_SFX];

static stb_vorbis *g_bgm_stream = NULL;
static BgmTrack g_current_bgm = BGM_NONE;
static int g_bgm_loop = 1;
static float g_bgm_volume = 1.0f;

static SceUID g_audio_thid = -1;
static volatile int g_audio_running = 0;
static int g_audio_port = -1;

static SceKernelLwMutexWork g_audio_mutex;

static const char *sfx_files[SFX_COUNT] = {
    DATA_PATH "res/raw/boop.ogg",
    DATA_PATH "res/raw/explosion.ogg",
    DATA_PATH "res/raw/fireworks.ogg",
    DATA_PATH "res/raw/medal.ogg"
};

static const char *bgm_files[] = {
    "",
    DATA_PATH "res/raw/menutrack.ogg",
    DATA_PATH "res/raw/soundtrack.ogg",
    DATA_PATH "res/raw/practicetrack.ogg"
};

static int audio_thread(SceSize args, void *argp) {
    short out_buf[AUDIO_SAMPLES * 2];
    short bgm_buf[AUDIO_SAMPLES * 2];
    int mix_buf[AUDIO_SAMPLES * 2];

    while (g_audio_running) {
        sceClibMemset(mix_buf, 0, sizeof(mix_buf));

        sceKernelLockLwMutex(&g_audio_mutex, 1, NULL);

        // Process BGM
        if (g_bgm_stream) {
            int read = stb_vorbis_get_samples_short_interleaved(
                g_bgm_stream, 2, bgm_buf, AUDIO_SAMPLES * 2);

            if (read > 0) {
                int count = read * 2;
                int vol = (int)(g_bgm_volume * 256.0f);
                for (int i = 0; i < count; i++) {
                    mix_buf[i] += (bgm_buf[i] * vol) >> 8;
                }
            }

            // Loop or restart if end reached
            if (read < AUDIO_SAMPLES) {
                if (g_bgm_loop) {
                    stb_vorbis_seek_start(g_bgm_stream);
                } else {
                    stb_vorbis_close(g_bgm_stream);
                    g_bgm_stream = NULL;
                    g_current_bgm = BGM_NONE;
                }
            }
        }

        // Process active SFX
        for (int i = 0; i < MAX_ACTIVE_SFX; i++) {
            if (!g_active_sfx[i].active || !g_active_sfx[i].sfx) continue;

            DecodedSfx *sfx = g_active_sfx[i].sfx;
            int cursor = g_active_sfx[i].cursor;

            for (int s = 0; s < AUDIO_SAMPLES; s++) {
                if (cursor >= sfx->num_samples) {
                    g_active_sfx[i].active = 0;
                    break;
                }

                if (sfx->channels == 1) {
                    short val = sfx->data[cursor++];
                    mix_buf[s * 2] += val;
                    mix_buf[s * 2 + 1] += val;
                } else {
                    mix_buf[s * 2] += sfx->data[cursor++];
                    mix_buf[s * 2 + 1] += sfx->data[cursor++];
                }
            }
            g_active_sfx[i].cursor = cursor;
        }

        sceKernelUnlockLwMutex(&g_audio_mutex, 1);

        // Clamp to 16-bit PCM
        for (int i = 0; i < AUDIO_SAMPLES * 2; i++) {
            int val = mix_buf[i];
            if (val > 32767) val = 32767;
            else if (val < -32768) val = -32768;
            out_buf[i] = (short)val;
        }

        sceAudioOutOutput(g_audio_port, out_buf);
    }

    return sceKernelExitDeleteThread(0);
}

int audio_init(void) {
    sceKernelCreateLwMutex(&g_audio_mutex, "audio_mutex", 0, 0, NULL);

    // SCE_AUDIO_OUT_PORT_TYPE_MAIN exige 48000Hz exactos; nuestros .ogg estan a
    // 44100Hz. Usamos el puerto BGM, que si acepta 44100Hz, para no tener que
    // resamplear en el hilo de mezcla.
    g_audio_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, AUDIO_SAMPLES, AUDIO_RATE, SCE_AUDIO_OUT_MODE_STEREO);
    if (g_audio_port < 0) {
        l_error("Failed to open audio out port: 0x%x", g_audio_port);
        return -1;
    }

    // Preload SFX
    for (int i = 0; i < SFX_COUNT; i++) {
        int channels, sample_rate;
        short *decoded = NULL;
        int samples = stb_vorbis_decode_filename(sfx_files[i], &channels, &sample_rate, &decoded);
        if (samples > 0 && decoded) {
            g_sfx[i].data = decoded;
            g_sfx[i].num_samples = samples * channels;
            g_sfx[i].channels = channels;
            g_sfx[i].sample_rate = sample_rate;
            l_info("Loaded SFX %s (%d frames, %d ch, %d Hz)", sfx_files[i], samples, channels, sample_rate);
        } else {
            l_warn("Failed to load SFX %s", sfx_files[i]);
            g_sfx[i].data = NULL;
            g_sfx[i].num_samples = 0;
        }
    }

    sceClibMemset(g_active_sfx, 0, sizeof(g_active_sfx));

    g_audio_running = 1;
    g_audio_thid = sceKernelCreateThread("audio_thread", audio_thread, 0x10000100 - 10, 0x10000, 0, 0, NULL);
    if (g_audio_thid >= 0) {
        sceKernelStartThread(g_audio_thid, 0, NULL);
    } else {
        l_error("Failed to start audio thread: 0x%x", g_audio_thid);
        return -1;
    }

    return 0;
}

void audio_shutdown(void) {
    g_audio_running = 0;
    if (g_audio_thid >= 0) {
        sceKernelWaitThreadEnd(g_audio_thid, NULL, NULL);
        g_audio_thid = -1;
    }

    sceKernelLockLwMutex(&g_audio_mutex, 1, NULL);
    if (g_bgm_stream) {
        stb_vorbis_close(g_bgm_stream);
        g_bgm_stream = NULL;
    }

    for (int i = 0; i < SFX_COUNT; i++) {
        if (g_sfx[i].data) {
            free(g_sfx[i].data);
            g_sfx[i].data = NULL;
        }
    }
    sceKernelUnlockLwMutex(&g_audio_mutex, 1);

    if (g_audio_port >= 0) {
        sceAudioOutReleasePort(g_audio_port);
        g_audio_port = -1;
    }

    sceKernelDeleteLwMutex(&g_audio_mutex);
}

void audio_play_music(BgmTrack track, int loop) {
    if (track <= BGM_NONE || track >= 4) {
        audio_stop_music();
        return;
    }

    sceKernelLockLwMutex(&g_audio_mutex, 1, NULL);

    if (g_current_bgm == track && g_bgm_stream != NULL) {
        // Already playing this track
        g_bgm_loop = loop;
        g_bgm_volume = 1.0f;
        sceKernelUnlockLwMutex(&g_audio_mutex, 1);
        return;
    }

    if (g_bgm_stream) {
        stb_vorbis_close(g_bgm_stream);
        g_bgm_stream = NULL;
    }

    int error = 0;
    g_bgm_stream = stb_vorbis_open_filename(bgm_files[track], &error, NULL);
    if (g_bgm_stream) {
        g_current_bgm = track;
        g_bgm_loop = loop;
        g_bgm_volume = 1.0f;
        l_info("Playing BGM: %s", bgm_files[track]);
    } else {
        l_warn("Failed to open BGM %s (err %d)", bgm_files[track], error);
        g_current_bgm = BGM_NONE;
    }

    sceKernelUnlockLwMutex(&g_audio_mutex, 1);
}

void audio_stop_music(void) {
    sceKernelLockLwMutex(&g_audio_mutex, 1, NULL);
    if (g_bgm_stream) {
        stb_vorbis_close(g_bgm_stream);
        g_bgm_stream = NULL;
    }
    g_current_bgm = BGM_NONE;
    sceKernelUnlockLwMutex(&g_audio_mutex, 1);
}

void audio_set_music_volume(float vol) {
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    g_bgm_volume = vol;
}

void audio_restart_music(void) {
    sceKernelLockLwMutex(&g_audio_mutex, 1, NULL);
    if (g_bgm_stream) {
        stb_vorbis_seek_start(g_bgm_stream);
        g_bgm_volume = 1.0f;
    }
    sceKernelUnlockLwMutex(&g_audio_mutex, 1);
}

void audio_play_sfx(SfxId id) {
    if (id < 0 || id >= SFX_COUNT) return;
    if (!g_sfx[id].data) return;

    sceKernelLockLwMutex(&g_audio_mutex, 1, NULL);

    // Find an empty slot or replace the one furthest along
    int slot = -1;
    for (int i = 0; i < MAX_ACTIVE_SFX; i++) {
        if (!g_active_sfx[i].active) {
            slot = i;
            break;
        }
    }
    if (slot == -1) {
        slot = 0; // Replace slot 0
    }

    g_active_sfx[slot].sfx = &g_sfx[id];
    g_active_sfx[slot].cursor = 0;
    g_active_sfx[slot].active = 1;

    sceKernelUnlockLwMutex(&g_audio_mutex, 1);
}
