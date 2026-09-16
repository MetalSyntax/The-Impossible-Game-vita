#include "menu.h"
#include "audio.h"
#include "save.h"
#include "utils/logger.h"

#include <stdio.h>
#include <string.h>

static GameState g_state = STATE_MAIN_MENU;
static int g_subpage = 0;
static int g_selected_btn = 0;
static int g_pressed_btn = -1;

// Textures
static Texture t_bg;
static Texture t_btn_start, t_btn_start_p;
static Texture t_btn_howtoplay, t_btn_howtoplay_p;
static Texture t_btn_medals, t_btn_medals_p;
static Texture t_btn_stats, t_btn_stats_p;

static Texture t_btn_lvl1, t_btn_lvl1_p;
static Texture t_btn_lvl2, t_btn_lvl2_p, t_btn_lvl2_lock;

static Texture t_pause_hdr;
static Texture t_pause_resume, t_pause_resume_p;
static Texture t_pause_restart, t_pause_restart_p;
static Texture t_pause_menu, t_pause_menu_p;

static Texture t_htp_hdr;
static Texture t_htp_pages[7];
static Texture t_btn_next, t_btn_next_p;
static Texture t_btn_back, t_btn_back_p;
static Texture t_btn_menu, t_btn_menu_p;

static Texture t_stats_pages[3];
static Texture t_progress_bar;
static Texture t_progress_frame;

static Texture t_medals_hdr;
static Texture t_medals_pages[3];
static Texture t_medals_lock;

static Texture t_end_noflags;
static Texture t_end_practice;

#define DRAWABLE DATA_PATH "res/drawable/"

static int hit_test(int tx, int ty, float x, float y, float w, float h) {
    return (tx >= x && tx <= x + w && ty >= y && ty <= y + h);
}

int menu_init(void) {
    t_bg = texture_load(DRAWABLE "menubackgroundl.png");

    t_btn_start = texture_load(DRAWABLE "menustartbuttonl.png");
    t_btn_start_p = texture_load(DRAWABLE "menustartbuttonpressedl.png");
    t_btn_howtoplay = texture_load(DRAWABLE "menuhowtoplaybuttonl.png");
    t_btn_howtoplay_p = texture_load(DRAWABLE "menuhowtoplaybuttonpressedl.png");
    t_btn_medals = texture_load(DRAWABLE "menumedalsbuttonl.png");
    t_btn_medals_p = texture_load(DRAWABLE "menumedalsbuttonpressedl.png");
    t_btn_stats = texture_load(DRAWABLE "menustatsbuttonl.png");
    t_btn_stats_p = texture_load(DRAWABLE "menustatsbuttonpressedl.png");

    t_btn_lvl1 = texture_load(DRAWABLE "menustartlevel1button.png");
    t_btn_lvl1_p = texture_load(DRAWABLE "menustartlevel1buttonpressed.png");
    t_btn_lvl2 = texture_load(DRAWABLE "menustartxboxbutton.png");
    t_btn_lvl2_p = texture_load(DRAWABLE "menustartxboxbuttonpressed.png");
    t_btn_lvl2_lock = texture_load(DRAWABLE "menustartxboxlocked.png");

    t_pause_hdr = texture_load(DRAWABLE "pausedheaderl.png");
    t_pause_resume = texture_load(DRAWABLE "pausedresumebuttonl.png");
    t_pause_resume_p = texture_load(DRAWABLE "pausedresumebuttonpressedl.png");
    t_pause_restart = texture_load(DRAWABLE "pausedexitpracticemodebuttonl.png");
    t_pause_restart_p = texture_load(DRAWABLE "pausedexitpracticemodebuttonpressedl.png");
    t_pause_menu = texture_load(DRAWABLE "pausedmenubuttonl.png");
    t_pause_menu_p = texture_load(DRAWABLE "pausedmenubuttonpressedl.png");

    t_htp_hdr = texture_load(DRAWABLE "howtoplayheader.png");
    for (int i = 0; i < 7; i++) {
        char path[256];
        snprintf(path, sizeof(path), DRAWABLE "howtoplaypage%d.png", i + 1);
        t_htp_pages[i] = texture_load(path);
    }
    t_btn_next = texture_load(DRAWABLE "howtoplaynextbuttonl.png");
    t_btn_next_p = texture_load(DRAWABLE "howtoplaynextbuttonpressedl.png");
    t_btn_back = texture_load(DRAWABLE "statsbackbuttonl.png");
    t_btn_back_p = texture_load(DRAWABLE "statsbackbuttonpressedl.png");
    t_btn_menu = texture_load(DRAWABLE "menumenubuttonl.png");
    t_btn_menu_p = texture_load(DRAWABLE "menumenubuttonpressedl.png");

    t_stats_pages[0] = texture_load(DRAWABLE "statspage1l.png");
    t_stats_pages[1] = texture_load(DRAWABLE "statspage2l.png");
    t_stats_pages[2] = texture_load(DRAWABLE "statspage3l.png");
    t_progress_bar = texture_load(DRAWABLE "progressbar.png");
    t_progress_frame = texture_load(DRAWABLE "statsprogressframe.png");

    t_medals_hdr = texture_load(DRAWABLE "medalsl.png");
    t_medals_pages[0] = texture_load(DRAWABLE "medalspage1l.png");
    t_medals_pages[1] = texture_load(DRAWABLE "medalspage2l.png");
    t_medals_pages[2] = texture_load(DRAWABLE "medalspage3l.png");
    t_medals_lock = texture_load(DRAWABLE "medallockl.png");

    t_end_noflags = texture_load(DRAWABLE "endnoflags.png");
    t_end_practice = texture_load(DRAWABLE "endpracticemode.png");

    l_info("Menu system initialized.");
    return 0;
}

void menu_shutdown(void) {
    texture_free(&t_bg);
    texture_free(&t_btn_start); texture_free(&t_btn_start_p);
    texture_free(&t_btn_howtoplay); texture_free(&t_btn_howtoplay_p);
    texture_free(&t_btn_medals); texture_free(&t_btn_medals_p);
    texture_free(&t_btn_stats); texture_free(&t_btn_stats_p);

    texture_free(&t_btn_lvl1); texture_free(&t_btn_lvl1_p);
    texture_free(&t_btn_lvl2); texture_free(&t_btn_lvl2_p); texture_free(&t_btn_lvl2_lock);

    texture_free(&t_pause_hdr);
    texture_free(&t_pause_resume); texture_free(&t_pause_resume_p);
    texture_free(&t_pause_restart); texture_free(&t_pause_restart_p);
    texture_free(&t_pause_menu); texture_free(&t_pause_menu_p);

    texture_free(&t_htp_hdr);
    for (int i = 0; i < 7; i++) texture_free(&t_htp_pages[i]);
    texture_free(&t_btn_next); texture_free(&t_btn_next_p);
    texture_free(&t_btn_back); texture_free(&t_btn_back_p);
    texture_free(&t_btn_menu); texture_free(&t_btn_menu_p);

    for (int i = 0; i < 3; i++) texture_free(&t_stats_pages[i]);
    texture_free(&t_progress_bar);
    texture_free(&t_progress_frame);

    texture_free(&t_medals_hdr);
    for (int i = 0; i < 3; i++) texture_free(&t_medals_pages[i]);
    texture_free(&t_medals_lock);

    texture_free(&t_end_noflags);
    texture_free(&t_end_practice);
}

void menu_set_state(GameState state) {
    g_state = state;
    g_subpage = 0;
    g_selected_btn = 0;
    g_pressed_btn = -1;

    if (state == STATE_MAIN_MENU || state == STATE_LEVEL_SELECT ||
        state == STATE_HOW_TO_PLAY || state == STATE_STATS || state == STATE_MEDALS) {
        audio_play_music(BGM_MENU, 1);
    }
}

GameState menu_get_state(void) {
    return g_state;
}

int menu_update(SceCtrlData *pad, SceCtrlData *old_pad, SceTouchData *touch, int *out_level) {
    uint32_t pressed = pad->buttons & ~old_pad->buttons;

    int tx = -1, ty = -1;
    int touching = 0;
    if (touch && touch->reportNum > 0) {
        touching = 1;
        tx = (touch->report[0].x * 960) / 1920;
        ty = (touch->report[0].y * 544) / 1088;
    }

    switch (g_state) {
        case STATE_MAIN_MENU: {
            // 4 buttons:
            // 0: Start (x=160, y=260, w=300, h=100)
            // 1: How to play (x=500, y=260, w=300, h=100)
            // 2: Medals (x=160, y=390, w=300, h=100)
            // 3: Stats (x=500, y=390, w=300, h=100)
            float b0_x = 160, b0_y = 260, b0_w = 300, b0_h = 100;
            float b1_x = 500, b1_y = 260, b1_w = 300, b1_h = 100;
            float b2_x = 160, b2_y = 390, b2_w = 300, b2_h = 100;
            float b3_x = 500, b3_y = 390, b3_w = 300, b3_h = 100;

            if (touching) {
                if (hit_test(tx, ty, b0_x, b0_y, b0_w, b0_h)) g_pressed_btn = 0;
                else if (hit_test(tx, ty, b1_x, b1_y, b1_w, b1_h)) g_pressed_btn = 1;
                else if (hit_test(tx, ty, b2_x, b2_y, b2_w, b2_h)) g_pressed_btn = 2;
                else if (hit_test(tx, ty, b3_x, b3_y, b3_w, b3_h)) g_pressed_btn = 3;
                else g_pressed_btn = -1;
            } else if (g_pressed_btn != -1) {
                audio_play_sfx(SFX_BOOP);
                if (g_pressed_btn == 0) menu_set_state(STATE_LEVEL_SELECT);
                else if (g_pressed_btn == 1) menu_set_state(STATE_HOW_TO_PLAY);
                else if (g_pressed_btn == 2) menu_set_state(STATE_MEDALS);
                else if (g_pressed_btn == 3) menu_set_state(STATE_STATS);
                g_pressed_btn = -1;
            }

            // D-Pad navigation
            if (pressed & SCE_CTRL_RIGHT) g_selected_btn ^= 1;
            if (pressed & SCE_CTRL_LEFT)  g_selected_btn ^= 1;
            if (pressed & SCE_CTRL_DOWN)  g_selected_btn ^= 2;
            if (pressed & SCE_CTRL_UP)    g_selected_btn ^= 2;

            if (pressed & SCE_CTRL_CROSS) {
                audio_play_sfx(SFX_BOOP);
                if (g_selected_btn == 0) menu_set_state(STATE_LEVEL_SELECT);
                else if (g_selected_btn == 1) menu_set_state(STATE_HOW_TO_PLAY);
                else if (g_selected_btn == 2) menu_set_state(STATE_MEDALS);
                else if (g_selected_btn == 3) menu_set_state(STATE_STATS);
            }
            break;
        }

        case STATE_LEVEL_SELECT: {
            float b0_x = 160, b0_y = 220, b0_w = 300, b0_h = 100;
            float b1_x = 500, b1_y = 220, b1_w = 300, b1_h = 100;
            float b_back_x = 380, b_back_y = 400, b_back_w = 200, b_back_h = 70;

            if (touching) {
                if (hit_test(tx, ty, b0_x, b0_y, b0_w, b0_h)) g_pressed_btn = 0;
                else if (hit_test(tx, ty, b1_x, b1_y, b1_w, b1_h)) g_pressed_btn = 1;
                else if (hit_test(tx, ty, b_back_x, b_back_y, b_back_w, b_back_h)) g_pressed_btn = 2;
                else g_pressed_btn = -1;
            } else if (g_pressed_btn != -1) {
                audio_play_sfx(SFX_BOOP);
                if (g_pressed_btn == 0) {
                    *out_level = 0;
                    g_state = STATE_GAMEPLAY;
                    return 1;
                } else if (g_pressed_btn == 1) {
                    *out_level = 1;
                    g_state = STATE_GAMEPLAY;
                    return 1;
                } else if (g_pressed_btn == 2) {
                    menu_set_state(STATE_MAIN_MENU);
                }
                g_pressed_btn = -1;
            }

            if (pressed & SCE_CTRL_LEFT)  g_selected_btn = 0;
            if (pressed & SCE_CTRL_RIGHT) g_selected_btn = 1;
            if (pressed & SCE_CTRL_DOWN)  g_selected_btn = 2;
            if (pressed & SCE_CTRL_UP && g_selected_btn == 2) g_selected_btn = 0;

            if (pressed & SCE_CTRL_CIRCLE) {
                audio_play_sfx(SFX_BOOP);
                menu_set_state(STATE_MAIN_MENU);
            } else if (pressed & SCE_CTRL_CROSS) {
                audio_play_sfx(SFX_BOOP);
                if (g_selected_btn == 0) {
                    *out_level = 0;
                    g_state = STATE_GAMEPLAY;
                    return 1;
                } else if (g_selected_btn == 1) {
                    *out_level = 1;
                    g_state = STATE_GAMEPLAY;
                    return 1;
                } else if (g_selected_btn == 2) {
                    menu_set_state(STATE_MAIN_MENU);
                }
            }
            break;
        }

        case STATE_PAUSE: {
            float b_res_x = 280, b_res_y = 200, b_res_w = 400, b_res_h = 80;
            float b_rst_x = 280, b_rst_y = 300, b_rst_w = 400, b_rst_h = 80;
            float b_men_x = 280, b_men_y = 400, b_men_w = 400, b_men_h = 80;

            if (touching) {
                if (hit_test(tx, ty, b_res_x, b_res_y, b_res_w, b_res_h)) g_pressed_btn = 0;
                else if (hit_test(tx, ty, b_rst_x, b_rst_y, b_rst_w, b_rst_h)) g_pressed_btn = 1;
                else if (hit_test(tx, ty, b_men_x, b_men_y, b_men_w, b_men_h)) g_pressed_btn = 2;
                else g_pressed_btn = -1;
            } else if (g_pressed_btn != -1) {
                audio_play_sfx(SFX_BOOP);
                if (g_pressed_btn == 0) {
                    g_state = STATE_GAMEPLAY;
                } else if (g_pressed_btn == 1) {
                    *out_level = -1; // Flag to restart current level
                    g_state = STATE_GAMEPLAY;
                    return 1;
                } else if (g_pressed_btn == 2) {
                    menu_set_state(STATE_MAIN_MENU);
                }
                g_pressed_btn = -1;
            }

            if (pressed & SCE_CTRL_UP && g_selected_btn > 0) g_selected_btn--;
            if (pressed & SCE_CTRL_DOWN && g_selected_btn < 2) g_selected_btn++;

            if (pressed & (SCE_CTRL_START | SCE_CTRL_CIRCLE)) {
                audio_play_sfx(SFX_BOOP);
                g_state = STATE_GAMEPLAY;
            } else if (pressed & SCE_CTRL_CROSS) {
                audio_play_sfx(SFX_BOOP);
                if (g_selected_btn == 0) g_state = STATE_GAMEPLAY;
                else if (g_selected_btn == 1) {
                    *out_level = -1;
                    g_state = STATE_GAMEPLAY;
                    return 1;
                } else if (g_selected_btn == 2) {
                    menu_set_state(STATE_MAIN_MENU);
                }
            }
            break;
        }

        case STATE_HOW_TO_PLAY: {
            float b_prev_x = 100, b_prev_y = 440, b_prev_w = 200, b_prev_h = 70;
            float b_men_x  = 380, b_men_y  = 440, b_men_w  = 200, b_men_h  = 70;
            float b_next_x = 660, b_next_y = 440, b_next_w = 200, b_next_h = 70;

            if (touching) {
                if (hit_test(tx, ty, b_prev_x, b_prev_y, b_prev_w, b_prev_h)) g_pressed_btn = 0;
                else if (hit_test(tx, ty, b_men_x, b_men_y, b_men_w, b_men_h)) g_pressed_btn = 1;
                else if (hit_test(tx, ty, b_next_x, b_next_y, b_next_w, b_next_h)) g_pressed_btn = 2;
                else g_pressed_btn = -1;
            } else if (g_pressed_btn != -1) {
                audio_play_sfx(SFX_BOOP);
                if (g_pressed_btn == 0 && g_subpage > 0) g_subpage--;
                else if (g_pressed_btn == 1) menu_set_state(STATE_MAIN_MENU);
                else if (g_pressed_btn == 2) {
                    if (g_subpage < 6) g_subpage++;
                    else menu_set_state(STATE_MAIN_MENU);
                }
                g_pressed_btn = -1;
            }

            if (pressed & SCE_CTRL_LEFT && g_subpage > 0) g_subpage--;
            if (pressed & (SCE_CTRL_RIGHT | SCE_CTRL_CROSS)) {
                if (g_subpage < 6) g_subpage++;
                else menu_set_state(STATE_MAIN_MENU);
            }
            if (pressed & SCE_CTRL_CIRCLE) menu_set_state(STATE_MAIN_MENU);
            break;
        }

        case STATE_STATS:
        case STATE_MEDALS: {
            float b_prev_x = 100, b_prev_y = 440, b_prev_w = 200, b_prev_h = 70;
            float b_men_x  = 380, b_men_y  = 440, b_men_w  = 200, b_men_h  = 70;
            float b_next_x = 660, b_next_y = 440, b_next_w = 200, b_next_h = 70;

            if (touching) {
                if (hit_test(tx, ty, b_prev_x, b_prev_y, b_prev_w, b_prev_h)) g_pressed_btn = 0;
                else if (hit_test(tx, ty, b_men_x, b_men_y, b_men_w, b_men_h)) g_pressed_btn = 1;
                else if (hit_test(tx, ty, b_next_x, b_next_y, b_next_w, b_next_h)) g_pressed_btn = 2;
                else g_pressed_btn = -1;
            } else if (g_pressed_btn != -1) {
                audio_play_sfx(SFX_BOOP);
                if (g_pressed_btn == 0 && g_subpage > 0) g_subpage--;
                else if (g_pressed_btn == 1) menu_set_state(STATE_MAIN_MENU);
                else if (g_pressed_btn == 2 && g_subpage < 2) g_subpage++;
                g_pressed_btn = -1;
            }

            if (pressed & SCE_CTRL_LEFT && g_subpage > 0) g_subpage--;
            if (pressed & SCE_CTRL_RIGHT && g_subpage < 2) g_subpage++;
            if (pressed & SCE_CTRL_CIRCLE) menu_set_state(STATE_MAIN_MENU);
            break;
        }

        case STATE_VICTORY: {
            if (touching || (pressed & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE | SCE_CTRL_START))) {
                audio_play_sfx(SFX_BOOP);
                menu_set_state(STATE_MAIN_MENU);
            }
            break;
        }

        default:
            break;
    }

    return 0;
}

void menu_render(void) {
    glViewport(0, 0, 960, 544);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0.0f, 960.0f, 544.0f, 0.0f, -1.0f, 1.0f); // Top-left is 0,0
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw background
    if (t_bg.id) {
        draw_sprite(t_bg.id, 0, 0, 960, 544, 1, 1, 1, 1);
    }

    switch (g_state) {
        case STATE_MAIN_MENU: {
            // Start button
            GLuint b0 = (g_pressed_btn == 0 || g_selected_btn == 0) ? t_btn_start_p.id : t_btn_start.id;
            draw_sprite(b0, 160, 260, 300, 100, 1, 1, 1, 1);

            // How to play button
            GLuint b1 = (g_pressed_btn == 1 || g_selected_btn == 1) ? t_btn_howtoplay_p.id : t_btn_howtoplay.id;
            draw_sprite(b1, 500, 260, 300, 100, 1, 1, 1, 1);

            // Medals button
            GLuint b2 = (g_pressed_btn == 2 || g_selected_btn == 2) ? t_btn_medals_p.id : t_btn_medals.id;
            draw_sprite(b2, 160, 390, 300, 100, 1, 1, 1, 1);

            // Stats button
            GLuint b3 = (g_pressed_btn == 3 || g_selected_btn == 3) ? t_btn_stats_p.id : t_btn_stats.id;
            draw_sprite(b3, 500, 390, 300, 100, 1, 1, 1, 1);
            break;
        }

        case STATE_LEVEL_SELECT: {
            GLuint b0 = (g_pressed_btn == 0 || g_selected_btn == 0) ? t_btn_lvl1_p.id : t_btn_lvl1.id;
            draw_sprite(b0, 160, 220, 300, 100, 1, 1, 1, 1);

            GLuint b1 = (g_pressed_btn == 1 || g_selected_btn == 1) ? t_btn_lvl2_p.id : t_btn_lvl2.id;
            draw_sprite(b1, 500, 220, 300, 100, 1, 1, 1, 1);

            GLuint bb = (g_pressed_btn == 2 || g_selected_btn == 2) ? t_btn_menu_p.id : t_btn_menu.id;
            draw_sprite(bb, 380, 400, 200, 70, 1, 1, 1, 1);
            break;
        }

        case STATE_PAUSE: {
            draw_rect(0, 0, 960, 544, 0, 0, 0, 0.6f);
            draw_sprite_centered(t_pause_hdr.id, 480, 100, 400, 80, 1, 1, 1, 1);

            GLuint b0 = (g_pressed_btn == 0 || g_selected_btn == 0) ? t_pause_resume_p.id : t_pause_resume.id;
            draw_sprite(b0, 280, 200, 400, 80, 1, 1, 1, 1);

            GLuint b1 = (g_pressed_btn == 1 || g_selected_btn == 1) ? t_pause_restart_p.id : t_pause_restart.id;
            draw_sprite(b1, 280, 300, 400, 80, 1, 1, 1, 1);

            GLuint b2 = (g_pressed_btn == 2 || g_selected_btn == 2) ? t_pause_menu_p.id : t_pause_menu.id;
            draw_sprite(b2, 280, 400, 400, 80, 1, 1, 1, 1);
            break;
        }

        case STATE_HOW_TO_PLAY: {
            draw_sprite_centered(t_htp_hdr.id, 480, 60, 350, 70, 1, 1, 1, 1);

            if (g_subpage >= 0 && g_subpage < 7 && t_htp_pages[g_subpage].id) {
                draw_sprite_centered(t_htp_pages[g_subpage].id, 480, 240, 560, 240, 1, 1, 1, 1);
            }

            if (g_subpage > 0) {
                GLuint b0 = (g_pressed_btn == 0) ? t_btn_back_p.id : t_btn_back.id;
                draw_sprite(b0, 100, 440, 200, 70, 1, 1, 1, 1);
            }

            GLuint b1 = (g_pressed_btn == 1) ? t_btn_menu_p.id : t_btn_menu.id;
            draw_sprite(b1, 380, 440, 200, 70, 1, 1, 1, 1);

            GLuint b2 = (g_pressed_btn == 2) ? t_btn_next_p.id : t_btn_next.id;
            draw_sprite(b2, 660, 440, 200, 70, 1, 1, 1, 1);
            break;
        }

        case STATE_STATS: {
            if (g_subpage >= 0 && g_subpage < 3 && t_stats_pages[g_subpage].id) {
                draw_sprite_centered(t_stats_pages[g_subpage].id, 480, 200, 560, 260, 1, 1, 1, 1);
            }

            // Progress bar for Level 1 or 2
            if (g_subpage == 1 || g_subpage == 2) {
                int lvl = g_subpage - 1;
                float practice_pct = (float)g_save_data.progress_practice[lvl] / 100.0f;
                float normal_pct = (float)g_save_data.progress_noflag[lvl] / 100.0f;

                // Practice mode bar
                draw_sprite(t_progress_frame.id, 380, 190, 220, 28, 1, 1, 1, 1);
                if (practice_pct > 0.0f) {
                    draw_sprite(t_progress_bar.id, 382, 192, 216.0f * practice_pct, 24, 1, 1, 1, 1);
                }

                // Normal mode bar
                draw_sprite(t_progress_frame.id, 380, 250, 220, 28, 1, 1, 1, 1);
                if (normal_pct > 0.0f) {
                    draw_sprite(t_progress_bar.id, 382, 252, 216.0f * normal_pct, 24, 1, 1, 1, 1);
                }
            }

            if (g_subpage > 0) {
                GLuint b0 = (g_pressed_btn == 0) ? t_btn_back_p.id : t_btn_back.id;
                draw_sprite(b0, 100, 440, 200, 70, 1, 1, 1, 1);
            }

            GLuint b1 = (g_pressed_btn == 1) ? t_btn_menu_p.id : t_btn_menu.id;
            draw_sprite(b1, 380, 440, 200, 70, 1, 1, 1, 1);

            if (g_subpage < 2) {
                GLuint b2 = (g_pressed_btn == 2) ? t_btn_next_p.id : t_btn_next.id;
                draw_sprite(b2, 660, 440, 200, 70, 1, 1, 1, 1);
            }
            break;
        }

        case STATE_MEDALS: {
            draw_sprite_centered(t_medals_hdr.id, 480, 60, 350, 70, 1, 1, 1, 1);

            if (g_subpage >= 0 && g_subpage < 3 && t_medals_pages[g_subpage].id) {
                draw_sprite_centered(t_medals_pages[g_subpage].id, 480, 230, 560, 240, 1, 1, 1, 1);

                // Lock icons for locked medals
                int m0 = g_subpage * 2;
                int m1 = m0 + 1;
                if (!g_save_data.medals[m0]) {
                    draw_sprite_centered(t_medals_lock.id, 330, 230, 70, 70, 1, 1, 1, 1);
                }
                if (!g_save_data.medals[m1]) {
                    draw_sprite_centered(t_medals_lock.id, 630, 230, 70, 70, 1, 1, 1, 1);
                }
            }

            if (g_subpage > 0) {
                GLuint b0 = (g_pressed_btn == 0) ? t_btn_back_p.id : t_btn_back.id;
                draw_sprite(b0, 100, 440, 200, 70, 1, 1, 1, 1);
            }

            GLuint b1 = (g_pressed_btn == 1) ? t_btn_menu_p.id : t_btn_menu.id;
            draw_sprite(b1, 380, 440, 200, 70, 1, 1, 1, 1);

            if (g_subpage < 2) {
                GLuint b2 = (g_pressed_btn == 2) ? t_btn_next_p.id : t_btn_next.id;
                draw_sprite(b2, 660, 440, 200, 70, 1, 1, 1, 1);
            }
            break;
        }

        case STATE_VICTORY: {
            draw_rect(0, 0, 960, 544, 0, 0, 0, 0.7f);
            draw_sprite_centered(t_end_noflags.id, 480, 272, 480, 320, 1, 1, 1, 1);
            break;
        }

        default:
            break;
    }
}
