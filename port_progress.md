# Registro de Progreso — The Impossible Game (PS Vita)

## Fase 1: Configuración y Preparación (Completada — 2026-09-11)
- Repo creado desde `soloader-boilerplate`, `.gitignore` anti-DMCA configurado.
- APK `05133-the-impossible-1.5.2.apk` copiado, analizado y extraído.
- ABI detectada: `armeabi`, `armeabi-v7a` (elegida: `armeabi-v7a`, arquitectura ARMv7 nativa para Cortex-A9 de PS Vita).
- GLES: Pipeline fijo GLES1.1 (`glVertexPointer`, `glTexCoordPointer`, `glDrawArrays`, blending).

## Fase 2: Decompilación (Completada — 2026-09-11 / 2026-09-12)
- jadx: Decompilación completa del APK con `--show-bad-code`, revelando el ciclo de vida de `ImpossibleGame.java` y los menús `s.java`, `v.java`, `u.java`, `C0000r.java`, `w.java`, `t.java`, `q.java`.
- Binario nativo analizado con `nm -D` y `objdump -d`: identificados todos los exports JNI reales y desmontada la rutina de generación de obstáculos en C `initLevelData`.

## Fase 3: Análisis del Motor Real (Completada — 2026-09-12)
- **Comparación con ports hermanos:** Confirmado que **no comparte motor** con ningún port hermano (motor propietario de FlukeDude).
- **Ciclo de vida nativo:**
  - `initLibrary(width, height)`: inicializa resolución de pantalla (ajustado a 960x544).
  - `initGame(level, use_vertex_arrays, unk, fps, tex_normal, tex_4444)`: carga el nivel (0: Fire Aura, 1: Original Xbox Level) y enlaza las texturas del atlas de juego.
  - `drawFrame(...)`: ciclo de renderizado GLES1 y simulación física por frame; retorna bitmask con eventos de juego (pausa, victoria, bandera de práctica, sonidos de fuegos artificiales, explosión, medallas).
  - Getters/Setters de persistencia: `getNumbAttempts`, `getNumbJumps`, `getMedalUnlocked`, `getProgress`, etc.
- **Imports:** Confirmado que `libimpossible.so` no realiza llamadas callbacks a Java (0 llamadas a JNIEnv). Dependencias de sistema y matemáticas (`cosf`, `sinf`, `round`, `truncf`, `lrand48`, `clock_gettime`) y GLES1.

## Fase 4: Bootstrap del Loader y Correcciones Técnicas (Completada — 2026-09-12)
- **Bug 1: Submódulo FalsoJNI ausente:** El directorio `lib/falso_jni/` estaba vacío por clonación sin submódulos. Se incorporaron los archivos de FalsoJNI para proporcionar las estructuras `jvm` y `jni`.
- **Bug 2: Redefinición de símbolos EGL con vitaGL:** `source/reimpl/egl.c` causaba colisión de linkeo (`multiple definition of eglQueryContext`, etc.) con la implementación nativa de EGL ya provista en `libvitaGL.a`. Solución: excluido de la compilación en `CMakeLists.txt`.
- **Bug 3: Símbolo `game_log` ausente en logger:** `FalsoJNI_Logger.c` requería `game_log(...)`. Implementada la función variádica en `source/utils/logger.c`.
- **Bug 4: Llamada incondicional a `JNI_OnLoad`:** El boilerplate original llamaba a `JNI_OnLoad` sin comprobar si existía, lo que provocaría un crash inmediato por puntero nulo ya que `libimpossible.so` no exporta dicha función. Corregido con comprobación previa `if (JNI_OnLoad)`.
- **Bug 5: Identificador de JNIEnv en FalsoJNI:** Se corrigió el uso de `&jni` como `JNIEnv` global de FalsoJNI.
- **Bug 6: Gotchas de Toolchain CMake / VitaSDK:**
  - Incompatibilidad de políticas de CMake moderno resuelta con `-DCMAKE_POLICY_VERSION_MINIMUM=3.5`.
  - Rutas con espacios resueltas mediante compilación limpia en `/tmp/tig-build`.
  - Corrección de `sceAudioOutClosePort` por `sceAudioOutReleasePort` en el SDK de Vita.

## Fase 5: Implementación de Sistemas Nativos (Completada — 2026-09-12)
- **Audio (`source/audio.h` y `source/audio.c`):**
  - Motor de audio multihilo dedicado con `SceAudioOut` (44100Hz estéreo de 16-bit).
  - Decodificación OGG mediante `stb_vorbis`.
  - BGM streaming para `menutrack.ogg`, `soundtrack.ogg` y `practicetrack.ogg`.
  - Precarga en memoria de efectos de sonido (`boop.ogg`, `explosion.ogg`, `fireworks.ogg`, `medal.ogg`) con reproducción instantánea sin stuttering.
- **Renderizado y Texturas (`source/texture.h` y `source/texture.c`):**
  - Carga rápida de texturas PNG mediante `stb_image.h`.
  - Soporte de renderizado de sprites 2D (`draw_sprite`, `draw_sprite_centered`, `draw_sprite_part`, `draw_rect`) vía vertex arrays y proyecciones GLES1 ortográficas.
- **Persistencia (`source/save.h` y `source/save.c`):**
  - Almacenamiento en `ux0:data/theimpossiblegame/save.dat` de saltos totales, intentos, medallas desbloqueadas y progreso por nivel (modo normal y modo práctica).
  - Sincronización bidireccional automática con las funciones nativas de `libimpossible.so`.
- **Menús e Interfaz (`source/menu.h` y `source/menu.c`):**
  - Reimplementación nativa en C de los 7 estados de menú:
    - Estado 0: Menú Principal
    - Estado 1: Selección de Nivel (Level 1: Fire Aura, Level 2: Original Xbox Level)
    - Estado 2: Menú de Pausa (Reanudar, Reiniciar / Salir de práctica, Volver al menú)
    - Estado 3: Cómo Jugar (visor de 7 páginas de tutorial)
    - Estado 4: Estadísticas (visor de 3 páginas con barras de progreso)
    - Estado 5: Medallas (visor de 3 páginas con candados dinámicos según estado de medallas)
    - Estado 6: Pantalla de Victoria (Normal / Práctica)
  - Soporte dual completo: control táctil en pantalla (`sceTouch`) + control físico con cruceta (D-Pad), `Cruz` (seleccionar) y `Círculo` (volver).
- **Controles en Juego (`source/main.c`):**
  - Salto: Botón `Cruz`, `D-Pad Arriba` o toque táctil en pantalla.
  - Modo Práctica: `Triángulo` o `R1` para soltar bandera (checkpoint), `Cuadrado` o `L1` para eliminar bandera.
  - Pausa: Botón `Start` o `Círculo`.
  - Frecuencia de muestreo táctil activada explícitamente con `sceTouchSetSamplingState`.

## Fase 6: Empaquetado y Artefactos (Completada — 2026-09-12)
- Metadatos LiveArea (`icon0.png`, `pic0.png`, `bg0.png`, `startup.png`) validados en formato PNG indexado de 8-bit con dimensiones canónicas y limpieza de metadatos de macOS (`._*`).
- Build exitoso: generado `eboot.bin` y empaquetado `theimpossiblegame.vpk` (703 KB).
- Copiado `theimpossiblegame.vpk` a la raíz del repositorio listo para transferir.
- Datos de juego organizados y verificados en `ux0_data/theimpossiblegame/` (`libimpossible.so`, texturas `res/`, audio `res/raw/`, fuentes `assets/`).

## Fase 7: Próximo Paso
- Despliegue y validación en consola real (PS Vita física) vía FTP / VitaShell.
