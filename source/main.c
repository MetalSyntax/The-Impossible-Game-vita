#include "utils/init.h"
#include "utils/glutil.h"
#include "utils/logger.h"
#include "utils/dialog.h"

#include "audio.h"
#include "texture.h"
#include "save.h"
#include "menu.h"

#include <psp2/kernel/threadmgr.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>

#include <falso_jni/FalsoJNI.h>
#include <so_util/so_util.h>

int _newlib_heap_size_user = 64 * 1024 * 1024;
unsigned int sceUserMainThreadStackSize = 2 * 1024 * 1024;

#ifdef USE_SCELIBC_IO
int sceLibcHeapSize = 4 * 1024 * 1024;
#endif

so_module so_mod;

// JNI function pointers from libimpossible.so
static jint (*ImpossibleGame_drawFrame)(
    void *env, void *obj,
    jboolean touch1_active, jint touch1_x, jint touch1_y,
    jboolean touch2_active, jint touch2_x, jint touch2_y,
    jboolean key_jump, jboolean key_next, jboolean key_prev,
    jboolean trackball, jboolean invincible, jint quality, jboolean render) = NULL;

static jboolean (*ImpossibleGame_getMedalUnlocked)(void *env, void *obj, jint medal) = NULL;
static jint (*ImpossibleGame_getNumbAttempts)(void *env, void *obj) = NULL;
static jint (*ImpossibleGame_getNumbJumps)(void *env, void *obj) = NULL;
static jint (*ImpossibleGame_getProgress)(void *env, void *obj, jint level, jboolean practice) = NULL;

static void (*ImpossibleGame_initGame)(
    void *env, void *obj,
    jint level, jboolean use_vertex_arrays, jboolean unk, jfloat fps, jint tex1, jint tex2) = NULL;

static void (*ImpossibleGame_initLibrary)(void *env, void *obj, jint width, jint height) = NULL;
static void (*ImpossibleGame_setMedalUnlocked)(void *env, void *obj, jint medal, jboolean unlocked) = NULL;
static void (*ImpossibleGame_setNumbAttempts)(void *env, void *obj, jint attempts) = NULL;
static void (*ImpossibleGame_setNumbJumps)(void *env, void *obj, jint jumps) = NULL;
static void (*ImpossibleGame_setProgress)(void *env, void *obj, jint level, jboolean practice, jint progress) = NULL;

int main() {
    // Archivo de log .log incremental (001-999). Se crea antes del primer log.
    logger_init();
    l_info("Starting The Impossible Game (PS Vita)...");
    l_info("Log file: %s", logger_current_path());

    soloader_init_all();

    // Check JNI_OnLoad if available
    int (* JNI_OnLoad)(void *jvm) = (void *)so_symbol(&so_mod, "JNI_OnLoad");
    if (JNI_OnLoad) {
        JNI_OnLoad(&jvm);
    }

    // Resolve native functions (libimpossible.so del Level Pack: 5 niveles,
    // prefijo JNI "impossiblegamelevelpack" en vez de "impossiblegame")
    ImpossibleGame_drawFrame = (void *)so_symbol(&so_mod, "Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_drawFrame");
    ImpossibleGame_getMedalUnlocked = (void *)so_symbol(&so_mod, "Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_getMedalUnlocked");
    ImpossibleGame_getNumbAttempts = (void *)so_symbol(&so_mod, "Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_getNumbAttempts");
    ImpossibleGame_getNumbJumps = (void *)so_symbol(&so_mod, "Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_getNumbJumps");
    ImpossibleGame_getProgress = (void *)so_symbol(&so_mod, "Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_getProgress");
    ImpossibleGame_initGame = (void *)so_symbol(&so_mod, "Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_initGame");
    ImpossibleGame_initLibrary = (void *)so_symbol(&so_mod, "Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_initLibrary");
    ImpossibleGame_setMedalUnlocked = (void *)so_symbol(&so_mod, "Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_setMedalUnlocked");
    ImpossibleGame_setNumbAttempts = (void *)so_symbol(&so_mod, "Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_setNumbAttempts");
    ImpossibleGame_setNumbJumps = (void *)so_symbol(&so_mod, "Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_setNumbJumps");
    ImpossibleGame_setProgress = (void *)so_symbol(&so_mod, "Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_setProgress");

    if (!ImpossibleGame_drawFrame || !ImpossibleGame_initGame || !ImpossibleGame_initLibrary) {
        l_fatal("Failed to resolve essential JNI symbols!");
        fatal_error("Failed to resolve JNI symbols from libimpossible.so");
    }

    l_success("JNI symbols resolved successfully.");

    gl_init();
    l_success("GL initialized.");

    // Enable touch and controls
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);

    // Audio & Save
    if (audio_init() == 0) {
        l_success("Audio initialized.");
    } else {
        l_warn("Audio failed to initialize, continuing without sound.");
    }
    save_init();
    l_success("Save initialized.");

    // Menu
    menu_init();
    l_success("Menu initialized.");

    // Load game textures
    Texture tex_normal = texture_load(DATA_PATH "res/drawable/gametextures.png");
    Texture tex_4444 = texture_load(DATA_PATH "res/drawable/gametextures4444.png");
    l_info("Game textures loaded: tex_normal=%d, tex_4444=%d", tex_normal.id, tex_4444.id);

    // Init native library with Vita native resolution 960x544
    ImpossibleGame_initLibrary(&jni, NULL, 960, 544);
    l_success("Native initLibrary done.");
    gl_swap();
    l_success("First frame swapped, entering main loop.");

    // Sync saved data to native library
    ImpossibleGame_setNumbJumps(&jni, NULL, g_save_data.numbJumps);
    ImpossibleGame_setNumbAttempts(&jni, NULL, g_save_data.numbAttempts);
    for (int i = 0; i < 12; i++) {
        ImpossibleGame_setMedalUnlocked(&jni, NULL, i, g_save_data.medals[i]);
    }
    for (int l = 0; l < 5; l++) {
        ImpossibleGame_setProgress(&jni, NULL, l, 1, g_save_data.progress_practice[l]);
        ImpossibleGame_setProgress(&jni, NULL, l, 0, g_save_data.progress_noflag[l]);
    }

    menu_set_state(STATE_MAIN_MENU);

    SceCtrlData pad, old_pad;
    sceClibMemset(&pad, 0, sizeof(pad));
    sceClibMemset(&old_pad, 0, sizeof(old_pad));

    SceTouchData touch;
    sceClibMemset(&touch, 0, sizeof(touch));

    int current_level = 0;
    int is_practice = 0;
    int save_counter = 0;

    while (1) {
        uint64_t frame_start = sceKernelGetProcessTimeWide();
        old_pad = pad;
        sceCtrlPeekBufferPositive(0, &pad, 1);
        sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);

        GameState state = menu_get_state();

        if (state != STATE_GAMEPLAY) {
            int start_level = 0;
            if (menu_update(&pad, &old_pad, &touch, &start_level)) {
                if (start_level >= 0) {
                    current_level = start_level;
                }
                is_practice = 0;

                // Initialize game session
                ImpossibleGame_initGame(
                    &jni, NULL,
                    current_level,
                    1,     // use_vertex_arrays = true
                    0,     // unk
                    60.0f, // fps
                    tex_normal.id,
                    tex_4444.id
                );

                // Niveles 0 (Fire Aura) y 1 (Xbox) usan el track base; el Level
                // Pack trae soundtrack propio para 2 (Chaoz Fantasy), 3 (Heaven) y 4 (Phazd).
                switch (current_level) {
                    case 2:  audio_play_music(BGM_SOUNDTRACK2, 1); break;
                    case 3:  audio_play_music(BGM_SOUNDTRACK3, 1); break;
                    case 4:  audio_play_music(BGM_SOUNDTRACK4, 1); break;
                    default: audio_play_music(BGM_SOUNDTRACK, 1);  break;
                }
            }
            menu_render();
        } else {
            // Gameplay state
            uint32_t pressed = pad.buttons & ~old_pad.buttons;

            // Pause toggle
            if (pressed & (SCE_CTRL_START | SCE_CTRL_CIRCLE)) {
                audio_play_sfx(SFX_BOOP);
                menu_set_state(STATE_PAUSE);
                gl_swap();
                continue;
            }

            int key_jump = (pad.buttons & (SCE_CTRL_CROSS | SCE_CTRL_UP)) != 0;
            int key_next = (pad.buttons & (SCE_CTRL_TRIANGLE | SCE_CTRL_R1)) != 0; // place flag
            int key_prev = (pad.buttons & (SCE_CTRL_SQUARE | SCE_CTRL_L1)) != 0;   // remove flag

            int touch1_active = 0;
            int touch1_x = 0;
            int touch1_y = 0;

            if (touch.reportNum > 0) {
                touch1_active = 1;
                touch1_x = (touch.report[0].x * 960) / 1920;
                touch1_y = 544 - ((touch.report[0].y * 544) / 1088);
            }

            // Setup GLES1 state for gameplay
            glViewport(0, 0, 960, 544);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrthof(0.0f, 960.0f, 0.0f, 544.0f, 0.0f, 1.0f);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDisable(GL_LIGHTING);

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, tex_normal.id);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glClearColor(0.15f, 0.47f, 0.47f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            // Draw frame & run physics
            int ret = ImpossibleGame_drawFrame(
                &jni, NULL,
                touch1_active, touch1_x, touch1_y,
                0, 0, 0,
                key_jump, key_next, key_prev,
                0, 0, 0, 1
            );

            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);

            // Check return events
            if (ret & 8) {
                if ((ret & 1) == 0) {
                    audio_restart_music();
                }
            }
            if (ret & 2) {
                audio_play_sfx(SFX_BOOP);
                menu_set_state(STATE_PAUSE);
            }
            if (ret & 4) {
                audio_stop_music();
                menu_set_state(STATE_VICTORY);
            }
            if (!is_practice && (ret & 1)) {
                is_practice = 1;
                audio_play_music(BGM_PRACTICE, 1);
            }
            if (ret & 256) {
                audio_play_sfx(SFX_FIREWORKS);
            } else if (ret & 512) {
                audio_play_sfx(SFX_EXPLOSION);
                if (!is_practice) {
                    audio_set_music_volume(0.0f);
                }
            } else if (ret & 1024) {
                audio_play_sfx(SFX_MEDAL);
            }

            // Sync stats
            g_save_data.numbJumps = ImpossibleGame_getNumbJumps(&jni, NULL);
            g_save_data.numbAttempts = ImpossibleGame_getNumbAttempts(&jni, NULL);
            for (int i = 0; i < 12; i++) {
                g_save_data.medals[i] = ImpossibleGame_getMedalUnlocked(&jni, NULL, i);
            }
            for (int l = 0; l < 5; l++) {
                g_save_data.progress_practice[l] = ImpossibleGame_getProgress(&jni, NULL, l, 1);
                g_save_data.progress_noflag[l] = ImpossibleGame_getProgress(&jni, NULL, l, 0);
            }

            // Periodically flush save to disk
            if (++save_counter >= 300) { // ~5 seconds
                save_write();
                save_counter = 0;
            }
        }

        gl_swap();

        // Cap to 60 FPS (~16666 us)
        uint64_t frame_elapsed = sceKernelGetProcessTimeWide() - frame_start;
        if (frame_elapsed < 16666) {
            sceKernelDelayThreadCB((SceUInt)(16666 - frame_elapsed));
        }
    }

    save_write();
    audio_shutdown();
    menu_shutdown();
    texture_free(&tex_normal);
    texture_free(&tex_4444);
    logger_shutdown();

    sceKernelExitDeleteThread(0);
    return 0;
}
