/*
 * Copyright (C) 2022-2024 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "utils/logger.h"

#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/threadmgr.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <string.h>

#ifndef DATA_PATH
#define DATA_PATH "ux0:data/theimpossiblegame/"
#endif

#define COLOR_RED    "\x1B[38;5;196m"
#define COLOR_PINK   "\x1B[38;5;212m"
#define COLOR_ORANGE "\x1B[38;5;202m"
#define COLOR_BLUE   "\x1B[38;5;32m"
#define COLOR_GREEN  "\x1B[32m"
#define COLOR_CYAN   "\x1B[36m"

#define COLOR_END    "\033[0m"

static SceKernelLwMutexWork _log_mutex;
static atomic_bool _log_mutex_ready = ATOMIC_VAR_INIT(false);

// Buffer A is used to adjust the format string.
static char buffer_a[2048];
// Buffer B is used to compile the final log using the updated format string.
static char buffer_b[2048];
// Buffer C holds the plain (sin colores ANSI) version for el archivo .log.
static char buffer_c[2048];
static char msg_body[1792];

// Archivo de log incremental (.log, 001-999).
static SceUID _log_fd = -1;
static char _log_path[256] = "";
static atomic_bool _log_init_tried = ATOMIC_VAR_INIT(false);

static void _log_file_write(const char *s) {
    if (_log_fd < 0 || !s || !*s)
        return;
    size_t len = strlen(s);
    if (len > 0)
        sceIoWrite(_log_fd, s, (int)len);
}

const char *logger_current_path(void) {
    return _log_path;
}

void logger_init(void) {
    // Si ya hay un archivo abierto, no crear otro (uno por ejecucion).
    if (_log_fd >= 0) {
        return;
    }
    bool expected = false;
    if (!atomic_compare_exchange_strong(&_log_init_tried, &expected, true)) {
        return; // otro hilo ya esta creando el archivo
    }

    // Crear directorios: DATA_PATH y DATA_PATH/logs (ignorar EEXIST).
    {
        char base_dir[256];
        sceClibSnprintf(base_dir, sizeof(base_dir), "%s", DATA_PATH);
        size_t blen = strlen(base_dir);
        // sceIoMkdir falla con trailing '/', quitarlo.
        while (blen > 1 && base_dir[blen - 1] == '/') {
            base_dir[blen - 1] = '\0';
            blen--;
        }
        sceIoMkdir(base_dir, 0777);
        char logs_dir[256];
        sceClibSnprintf(logs_dir, sizeof(logs_dir), "%s/logs", base_dir);
        sceIoMkdir(logs_dir, 0777);
    }

    // Buscar el primer hueco libre: log_001.log ... log_999.log
    int slot = 0;
    SceIoStat st;
    for (int i = 1; i <= LOGGER_MAX_FILES; i++) {
        sceClibSnprintf(_log_path, sizeof(_log_path),
                        "%slogs/log_%03d.log", DATA_PATH, i);
        if (sceIoGetstat(_log_path, &st) < 0) {
            slot = i;
            break;
        }
    }
    if (slot == 0) {
        // Todos ocupados (001-999): wrap, sobrescribir el 001.
        sceClibSnprintf(_log_path, sizeof(_log_path),
                        "%slogs/log_001.log", DATA_PATH);
    }

    _log_fd = sceIoOpen(_log_path,
                        SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC,
                        0777);
    if (_log_fd < 0) {
        _log_path[0] = '\0';
        // Permitir reintento posterior (p.ej. tras fios_init).
        atomic_store(&_log_init_tried, false);
        sceClibPrintf("Error: no se pudo crear el archivo de log\n");
        return;
    }

    {
        char header[256];
        sceClibSnprintf(header, sizeof(header),
                        "The Impossible Game (PS Vita) - log %s\n",
                        _log_path);
        _log_file_write(header);
    }
}

void logger_shutdown(void) {
    if (_log_fd >= 0) {
        _log_file_write("--- fin del log ---\n");
        sceIoClose(_log_fd);
        _log_fd = -1;
    }
}

void _log_print(int t, const char* fmt, ...) {
    if (!atomic_load_explicit(&_log_mutex_ready, memory_order_relaxed)) {
        int ret = sceKernelCreateLwMutex(&_log_mutex, "log_lock", 0, 0, NULL);
        if (ret < 0) {
            sceClibPrintf("Error: failed to create log mutex: 0x%x\n", ret);
            return;
        }
        atomic_store_explicit(&_log_mutex_ready, true, memory_order_relaxed);
    }
    sceKernelLockLwMutex(&_log_mutex, 1, NULL);

    const char *tag = NULL;
    switch (t) {
        case LT_DEBUG:
            sceClibSnprintf(buffer_a, sizeof(buffer_a), " %s• debug%s    %s\n",
                            COLOR_PINK, COLOR_END, fmt);
            tag = "debug"; break;
        case LT_INFO:
            sceClibSnprintf(buffer_a, sizeof(buffer_a), " %sℹ info%s     %s\n",
                            COLOR_BLUE, COLOR_END, fmt);
            tag = "info"; break;
        case LT_WARN:
            sceClibSnprintf(buffer_a, sizeof(buffer_a), " %s⚠ warning%s  %s\n",
                            COLOR_ORANGE, COLOR_END, fmt);
            tag = "warning"; break;
        case LT_ERROR:
            sceClibSnprintf(buffer_a, sizeof(buffer_a), " %s⨯ error%s    %s\n",
                            COLOR_RED, COLOR_END, fmt);
            tag = "error"; break;
        case LT_FATAL:
            sceClibSnprintf(buffer_a, sizeof(buffer_a), " %s! fatal%s    %s\n",
                            COLOR_RED, COLOR_END, fmt);
            tag = "fatal"; break;
        case LT_SUCCESS:
            sceClibSnprintf(buffer_a, sizeof(buffer_a), " %s! success%s  %s\n",
                            COLOR_GREEN, COLOR_END, fmt);
            tag = "success"; break;
        case LT_WAIT:
            sceClibSnprintf(buffer_a, sizeof(buffer_a), " %s… waiting%s  %s\n",
                            COLOR_CYAN, COLOR_END, fmt);
            tag = "waiting"; break;
        default:
            if (atomic_load_explicit(&_log_mutex_ready, memory_order_relaxed)) {
                sceKernelUnlockLwMutex(&_log_mutex, 1);
            }
            return;
    }

    va_list list;
    va_start(list, fmt);
    va_list copy;
    va_copy(copy, list);
    sceClibVsnprintf(buffer_b, sizeof(buffer_b), buffer_a, list);
    va_end(list);
    // Version plana sin colores ANSI para el archivo .log
    sceClibVsnprintf(msg_body, sizeof(msg_body), fmt, copy);
    va_end(copy);
    sceClibSnprintf(buffer_c, sizeof(buffer_c), "[%s] %s\n", tag, msg_body);
    sceClibPrintf(buffer_b);
    _log_file_write(buffer_c);

    if (atomic_load_explicit(&_log_mutex_ready, memory_order_relaxed)) {
        sceKernelUnlockLwMutex(&_log_mutex, 1);
    }
}

void game_log(const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    char buf[1024];
    sceClibVsnprintf(buf, sizeof(buf), fmt, list);
    va_end(list);
    sceClibPrintf("%s", buf);
    _log_file_write(buf);
}
