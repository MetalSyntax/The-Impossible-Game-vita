# Plan de Port — The Impossible Game (PS Vita)

> Actualizado con análisis exhaustivo del binario nativo, decompilación Java y bootstrap completado.

## 0. Contexto

- **Juego:** The Impossible Game
- **Paquete Java:** `com.flukedude.impossiblegame`
- **APK original:** `05133-the-impossible-1.5.2.apk`
- **TITLEID asignado:** `PSVTIG001`

**¿Motor conocido?** 
- Confirmado con análisis de símbolos y decompilación: **Motor propio e independiente de FlukeDude** (no comparte motor con ningún port hermano como Gameloft, Cocos2d-x ni Gamevil).
- Arquitectura:
  - `libimpossible.so` (47 KB, ARMv7 nativo): contiene la física completa del cubo, colisiones, lógica de niveles (`initLevelData` con obstáculos generados en código C), rotación, saltos y renderizado del juego activo con OpenGL ES 1.1 (`drawFrame`).
  - Capa de UI y Audio: En Android estaba en Java (`s.java`, `v.java`, `u.java`, `C0000r.java`, `w.java`, `t.java`, `q.java`), ha sido reimplementada nativamente en C (`menu.c`, `audio.c`, `texture.c`, `save.c`) para máximo rendimiento y latencia cero en PS Vita.

## 1. Detección y Arquitectura

- **ABI:** `armeabi-v7a` (ARMv7, Cortex-A9 nativo de Vita sin emulación ni traducción).
- **Pipeline Gráfico:** OpenGL ES 1.1 (pipeline fijo: `glVertexPointer`, `glTexCoordPointer`, `glDrawArrays`, blending, proyecciones ortográficas) ejecutado de forma nativa vía `vitaGL`.
- **Resolución:** 960x544 nativa de PS Vita.

## 2. .so Analizados (ABI armeabi-v7a)

- `libimpossible.so` (47 KB).

## 3. Exports JNI Confirmados (`nm -D`)

Los símbolos JNI siguen la convención estándar `Java_*` (no requirió `RegisterNatives`):
- `Java_com_flukedude_impossiblegame_ImpossibleGame_initLibrary(JNIEnv *env, jobject obj, jint width, jint height)`
- `Java_com_flukedude_impossiblegame_ImpossibleGame_initGame(JNIEnv *env, jobject obj, jint level, jboolean use_vertex_arrays, jboolean unk, jfloat fps, jint tex1, jint tex2)`
- `Java_com_flukedude_impossiblegame_ImpossibleGame_drawFrame(JNIEnv *env, jobject obj, jboolean touch1_active, jint touch1_x, jint touch1_y, jboolean touch2_active, jint touch2_x, jint touch2_y, jboolean key_jump, jboolean key_next, jboolean key_prev, jboolean trackball, jboolean invincible, jint quality, jboolean render)`
- `Java_com_flukedude_impossiblegame_ImpossibleGame_getMedalUnlocked(JNIEnv *env, jobject obj, jint medal)`
- `Java_com_flukedude_impossiblegame_ImpossibleGame_setMedalUnlocked(JNIEnv *env, jobject obj, jint medal, jboolean unlocked)`
- `Java_com_flukedude_impossiblegame_ImpossibleGame_getNumbAttempts(JNIEnv *env, jobject obj)`
- `Java_com_flukedude_impossiblegame_ImpossibleGame_setNumbAttempts(JNIEnv *env, jobject obj, jint attempts)`
- `Java_com_flukedude_impossiblegame_ImpossibleGame_getNumbJumps(JNIEnv *env, jobject obj)`
- `Java_com_flukedude_impossiblegame_ImpossibleGame_setNumbJumps(JNIEnv *env, jobject obj, jint jumps)`
- `Java_com_flukedude_impossiblegame_ImpossibleGame_getProgress(JNIEnv *env, jobject obj, jint level, jboolean practice)`
- `Java_com_flukedude_impossiblegame_ImpossibleGame_setProgress(JNIEnv *env, jobject obj, jint level, jboolean practice, jint progress)`

**Nota crítica de imports:** `libimpossible.so` tiene 0 callbacks hacia Java/JNIEnv. Solo importa funciones matemáticas estándar (`cosf`, `sinf`, `round`, `truncf`, `lrand48`, `memcpy`), reloj (`clock_gettime`) y llamadas GLES1 (`glDrawArrays`, `glVertexPointer`, `glTexCoordPointer`, `glBindTexture`, etc.), todas resueltas exitosamente en `dynlib.c`.

## 4. Checklist

- [x] Repo creado desde soloader-boilerplate, git init, .gitignore anti-DMCA.
- [x] APK decompilado (jadx) y .so analizado exhaustivamente (`nm`, `objdump`, bytecode dex).
- [x] Análisis del motor real (confirmado motor indie FlukeDude, desacoplada lógica nativa vs UI Java).
- [x] Bootstrap del loader: `so_file_load`, `so_relocate`, `so_resolve`, primer build exitoso.
- [x] Tabla JNI (FalsoJNI): resueltas funciones nativas con firma JNI exacta.
- [x] Gráficos: Pipeline GLES1 nativo con vitaGL (960x544), soporte de texturas normales y 4444.
- [x] Input: Mapeo de botones físicos Vita (`Cruz`/`Arriba` = Salto, `Triángulo`/`R1` = Poner bandera, `Cuadrado`/`L1` = Quitar bandera, `Start`/`Círculo` = Pausa) + soporte táctil de pantalla (`sceTouch`).
- [x] Audio: Sistema de audio multihilo con `SceAudioOut` y decodificador OGG `stb_vorbis` (música BGM en streaming, SFX precargados en memoria sin latencia).
- [x] Assets & LiveArea: Assets extraídos en `ux0_data/theimpossiblegame/`, LiveArea PNGs indexados a 8-bit sin metadatos `._`, VPK generado (`theimpossiblegame.vpk`).
- [x] Pruebas en hardware real (instalación de `theimpossiblegame.vpk` y despliegue de datos a `ux0:data/theimpossiblegame/`) — menú y ambos niveles jugables a 60 FPS, con audio, input y guardado confirmados. Ver `port_progress.md` Fase 12 (vitaGL vendorizada como submodule, causa raíz del bug de pantalla negra).

## 5. Despliegue a Consola Real

1. Instalar `theimpossiblegame.vpk` en la PS Vita vía VitaShell o FTP.
2. Copiar el contenido de la carpeta `ux0_data/theimpossiblegame/` a la consola en `ux0:data/theimpossiblegame/`:
   - `ux0:data/theimpossiblegame/libimpossible.so`
   - `ux0:data/theimpossiblegame/res/`
   - `ux0:data/theimpossiblegame/assets/`
3. Asegurar que `kubridge.skprx` y `libshacccg.suprx` estén instalados en la consola.
4. Iniciar el juego desde el LiveArea.
