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

## Fase 7: Validación en Consola Real (En curso — 2026-09-12)
- Primer log real (`logs/log_001.log`) obtenido de la consola física.
- **Bug 7 (confirmado por log, corregido): Silencio total de audio.**
  - Síntoma reportado: no suena nada en el juego.
  - Log línea 16: `[error] Failed to open audio out port: 0x80260008`. Ese código
    es `SCE_AUDIO_OUT_ERROR_INVALID_SAMPLE_FREQ` (confirmado contra
    `psp2/audioout.h` del VitaSDK instalado).
  - Causa: `audio_init()` (`source/audio.c`) abría el puerto con
    `SCE_AUDIO_OUT_PORT_TYPE_MAIN`, que exige 48000Hz exactos según la propia
    documentación del header, pero los `.ogg` del juego están a 44100Hz
    (`AUDIO_RATE`). Al fallar `sceAudioOutOpenPort`, `audio_init()` retornaba
    `-1` **antes** de arrancar el hilo de mezcla — de ahí el silencio total, no
    un crash. El log seguía mostrando `[info] Playing BGM: ...` porque
    `audio_play_music` solo decodifica el stream con `stb_vorbis` sin comprobar
    si el hilo de audio llegó a arrancar.
  - Fix: cambiado a `SCE_AUDIO_OUT_PORT_TYPE_BGM`, que sí acepta 44100Hz (evita
    tener que resamplear). También corregido `source/main.c` para que loguee
    `warning` en vez de `success` si `audio_init()` falla, así el log no
    miente sobre el estado real.
  - **Pendiente de confirmar en hardware** tras este fix.
- **Freeze reportado (sin confirmar todavía): el juego parece congelarse al
  arrancar un nivel.**
  - El log se corta en la línea 75 (`Playing BGM: soundtrack.ogg`), justo
    después de `audio_play_music(BGM_SOUNDTRACK, 1)` en `source/main.c:161` y
    antes de la primera `ImpossibleGame_drawFrame()` de la sesión de juego.
  - Como el hilo de audio nunca llegaba a arrancar (bug de arriba), el freeze
    **no puede ser causado por el mezclador de audio** — no había nada
    corriendo ahí que pudiera bloquear. La correlación temporal con el sonido
    es casualidad de orden de logueo, no causa.
  - Hipótesis más probable: el freeze está dentro de `libimpossible.so`, en
    `ImpossibleGame_initGame` o en la primera `ImpossibleGame_drawFrame` (la
    rutina de generación de obstáculos `initLevelData`, ya marcada como
    sospechosa en la Fase 2, es un candidato natural).
  - No hay `.psp2dmp` de este hang (un loop infinito no genera dump, solo un
    acceso a memoria inválido lo hace) — para confirmar la causa real hace
    falta adjuntar GDB en vivo mientras está congelado (vía el toolkit
    standalone) y sacar PC/LR + backtrace, o instrumentar temporalmente
    `initGame`/`drawFrame` con logs adicionales.
  - Los FPS mal leídos por el plugin instalado son consistentes con este mismo
    freeze (el juego deja de producir frames nuevos), no un bug aparte.

## Fase 8: Freeze en consola real (En curso — 2026-09-12)
- **Bug 7 (audio) confirmado arreglado:** `logs/log_002.log` en consola real
  muestra `[success] Audio initialized.` sin error y los 4 SFX cargados
  correctamente (44100Hz vía puerto BGM). El usuario confirma que ahora sí
  suena.
- **Freeze confirmado, reproducible siempre en el mismo punto:** con audio ya
  andando, el usuario confirma que la música suena pero la pantalla queda en
  negro/congelada ("no se ve nada") al arrancar un nivel. `log_002.log` se
  corta exactamente en la misma línea que `log_001.log` (justo después de
  `audio_play_music(BGM_SOUNDTRACK, 1)`, antes de la primera
  `ImpossibleGame_drawFrame()`), descartando definitivamente que el audio sea
  la causa (el mezclador corre en su propio hilo, desacoplado del loop
  principal).
- **Instrumentación temporal agregada en `source/main.c`** (marcada con
  comentarios `dbg:`, a quitar una vez confirmada la causa real) para
  bisectar sin necesitar GDB todavía:
  - Log antes/después de `ImpossibleGame_initGame(...)`.
  - Log al entrar por primera vez a la rama `STATE_GAMEPLAY`.
  - Log antes/después de la primera `ImpossibleGame_drawFrame(...)`.
  - Objetivo: ver en el próximo log real cuál de estas 4 líneas es la última
    que aparece, para saber si el hang está en `initGame`, en el propio loop
    C antes de llamar a `drawFrame`, o dentro de `drawFrame` (candidato más
    probable: la rutina `initLevelData` marcada como sospechosa en la Fase 2).

## Fase 10: Diagnóstico de `log_005.log` — Audio y 60 FPS correctos, pero pantalla negra permanente (2026-09-15)
- **Hallazgos clave en `log_005.log`:**
  1. El juego ya no se congela: los primeros 5 frames del menú se swapean con éxito (`dbg: frame 0-4 swapped (glError=0x0, state=0)`).
  2. El usuario navegó el menú a ciegas e inició el nivel 0 (`calling ImpossibleGame_initGame(level=0)` retornado con éxito, y primera llamada a `ImpossibleGame_drawFrame` retornó `0`).
  3. El hilo principal corre estable a 60 FPS con música (`menutrack.ogg` y luego `soundtrack.ogg`).
  4. Sin embargo, la pantalla permanece totalmente negra tanto en el menú como durante el gameplay.
- **Causa raíz confirmada mediante desensamblado de `libvitaGL.a` (`vgl.o` y `mem_utils.o`):**
  1. **Asfixia de memoria del pool interno de vitaGL (`_newlib_heap_size_user` = 256 MB):**
     - En `source/main.c`, `_newlib_heap_size_user` estaba configurado en 256 MB, reservando prácticamente toda la memoria de usuario disponible del sistema para el heap de newlib (`malloc`).
     - Al llamar a `vglInitExtended(0, 960, 544, 8 * 1024 * 1024, ...)`, vitaGL consulta la memoria libre del OS mediante `sceKernelGetFreeMemorySize`. Al quedar menos de 8 MB (`ram_threshold`), la rutina de cálculo fija `ram_pool_size = 0`.
     - Con `ram_pool_size == 0`, `vgl_mem_init` **no inicializa el pool de RAM de vitaGL**.
     - En consecuencia:
       - `vitashark` (cuyo asignador está enlazado a `vglMalloc`) no puede reservar memoria para compilar dinámicamente los shaders Cg del pipeline fijo (FFP). Sin shader compilado, `glDrawArrays` no dibuja nada.
       - `glClear` no puede reservar los vértices de borrado (`clear_vertices = gpu_alloc_mapped_for_cpu`), por lo que ni siquiera el color de fondo se renderiza.
       - `glTexImage2D` no puede reservar memoria GPU en RAM para las texturas.
  2. **Comportamiento del retorno de `vglInitExtended`:**
     - En vitaGL, `vglInitExtended` retorna `res_fallback` (`GL_TRUE` si la resolución tuvo que ser reducida/downscaled, `GL_FALSE` si se inicializó a la resolución nativa 960x544). Por ende, `if (!res)` arrojaba erróneamente un log de `vglInitExtended failed!` cuando en realidad la función retornó `GL_FALSE` esperado para resolución nativa.
- **Correcciones aplicadas:**
  1. **Reducción de `_newlib_heap_size_user` a 64 MB (`source/main.c`):** Libera ~190 MB de memoria de usuario para el pool de RAM de vitaGL, garantizando memoria abundante para compilación de shaders por `vitashark`, texturas y buffers de render.
  2. **Telemetría de memoria en `gl_init()` (`source/utils/glutil.c`):** Se eliminó la comprobación errónea de `!res` y se añadieron logs de diagnóstico que reportan la RAM libre pre-GL vía kernel, así como `vglMemFree` y `vglMemTotal` tanto para RAM como para VRAM.
  3. **Limpieza y normalización de estado GLES1 en `main.c` y `menu.c`:**
     - En `main.c` (`STATE_GAMEPLAY`): Se añadieron llamadas explícitas a `glDisable(GL_DEPTH_TEST)`, `glDisable(GL_CULL_FACE)`, `glDisable(GL_LIGHTING)`, `glColor4f(1.0f, 1.0f, 1.0f, 1.0f)` y `glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE)` para asegurar que el pipeline fijo no tenga estados residuales ni modulación de color errónea.
     - En `menu.c` (`menu_render`): Se añadieron `glColor4f(1, 1, 1, 1)` y `glTexEnvi`.
     - En `main.c`: Se añadió logging de transición de estados (`GameState`).
  4. **Recompilación exitosa:** Generados `eboot.bin` (643 KB) y `theimpossiblegame.vpk` (724 KB).

## Fase 11: Próximo Paso
- Probar el nuevo `eboot.bin` / `theimpossiblegame.vpk` en la consola física.
- Comprobar en el siguiente log (`log_006.log`) las líneas:
  - `Pre-GL free user RAM: ~19X MB`
  - `Post-GL vitaGL RAM: free=... MB, total=... MB`
- Confirmar que la pantalla del menú y los gráficos del nivel ya son visibles a 60 FPS estables.

## Fase 12: Pantalla negra persistente pese al fix de memoria — causa raíz real:
## `libvitaGL.a` global de `~/vitasdk` rota (2026-09-15/16)

- **`log_006.log` llega casi vacío (solo el header):** builds previos habían sido compilados sin
  `CMAKE_BUILD_TYPE=Debug` real (el `CMakeCache.txt` copiado por el toolkit queda vacío porque el
  build real ocurre en un directorio temporal — no es indicativo). Confirmado con
  `psvita-toolkit build --preset debug` + verificación de strings de diagnóstico en el `.elf`.
- **`log_007.log` con logging completo confirma que el fix de RAM de la Fase 10 sí funcionó**
  (`Pre-GL free user RAM: 284 MB`, `Post-GL vitaGL RAM: free=233 MB`) — pero la pantalla seguía
  100% negra (confirmado con captura de pantalla nativa de la consola: solo el overlay de
  CPU/FPS de un plugin, nada del juego).
- **Aislamiento sistemático de la causa** (cada paso con evidencia real de consola, no
  suposición):
  1. Prueba de pantalla roja sólida forzada (180 frames de `glClear`+swap) tras `gl_init()`:
     sigue en negro, `glGetError()=0x0` todo el tiempo — descarta shaders/texturas/estado GL del
     juego.
  2. Deshabilitar el plugin de overlay CPU/FPS: sin cambios — descarta compositing externo.
  3. Wrapper de linker (`-Wl,--wrap=sceDisplaySetFrameBuf`) para loguear cada llamada real:
     siempre retorna éxito (`0x0`), con `base`/`pitch`/`fmt`/`w`/`h` correctos y alternando entre
     los dos buffers del swapchain — descarta un problema de presentación a nivel de sistema.
  4. Lectura directa de los bytes del framebuffer en la dirección `base` reportada: **todo
     ceros**, incluso después de docenas de `glClear` a rojo sólido — el clear nunca llega a
     escribir memoria real, pese a que cada capa (GL, GXM, `sceDisplaySetFrameBuf`) reporta
     éxito.
  5. `vglUseCachedMem(GL_TRUE)` (agregado en Fase 10) removido como sospechoso de invalidar
     vértices del clear interno de vitaGL: sin cambios — descartado.
  6. Callback de logging de `vitashark` (`shark_install_log_cb`) a verbosidad máxima durante la
     prueba de rojo: **cero mensajes** — vitaGL usa binarios GXP precompilados para el pipeline
     fijo básico, vitashark ni siquiera se invoca en este camino.
  7. **Prueba decisiva:** linkear temporalmente contra la vitaGL de stock (`libvitaGL.a.bak-presplash`,
     hallada junto a la `libvitaGL.a` activa en `~/vitasdk/arm-vita-eabi/lib/`) en vez de la
     activa — sin tocar nada compartido (copia local en `lib/vitagl_test/`, `link_directories()`
     temporal). **Mismo resultado: negro total**, ni siquiera aparece el splashscreen 3D
     automático de vitaGL (confirmado con el símbolo `VGL_CONTEXT_SPLASHSCREEN` presente en el
     binario). Esto descarta que sea un problema del juego o de shaders — algo more profundo en
     **ambas** variantes de vitaGL instaladas globalmente está roto para este port.
- **Antecedente encontrado en `Sacred-Odyssey-vita/port_progress.md` (Fase 8):** la
  `libvitaGL.a`/`vitaGL.h` global de `~/vitasdk` había sido recompilada a mano con
  `NO_SPLASHSCREEN=1` para otro port y reinstalada sobre la ruta global compartida por **todos**
  los ports de esta máquina (guardando el original en `libvitaGL.a.bak-presplash`) — Sacred-Odyssey
  después migró a vendorizar vitaGL como submodule propio por esta misma razón ("rompía el build
  en silencio" de otros ports). Nunca se revirtió la global.
- **Causa raíz confirmada:** la `libvitaGL.a` global de `~/vitasdk` (en cualquiera de sus dos
  variantes) está rota para este port específico — el motivo exacto (¿corrupción del build a
  mano, mismatch de ABI/flags, algo específico de esta combinación de vitaGL+vitashark+firmware?)
  no se pudo aislar más sin el código fuente original de esa build. **No es un bug de este port
  ni del juego.**
- **Fix aplicado:** vitaGL vendorizada como git submodule propio (`lib/vitagl`, apuntando a
  `https://github.com/Rinnegatamante/vitaGL.git`, mismo patrón que `Sacred-Odyssey-vita` y
  `Asphalt-5-Vita`), compilada desde fuente por `CMakeLists.txt` (`VITAGL_MAKE_FLAGS =
  "SOFTFP_ABI=1 LOG_ERRORS=1 NO_SPLASHSCREEN=1"`), sin tocar la instalación global. Un parche
  mínimo (`vgl.c`: `vglSetShaderAssociationPath` guardado detrás de `#ifdef HAVE_RAZOR`) fue
  necesario porque el `libvitashark.a` instalado en este VitaSDK todavía no exporta
  `shark_set_shader_association_path` (mismo issue ya documentado por Sacred-Odyssey-vita).
  `LOG_ERRORS=1` se dejó activo permanentemente (puenteado a nuestro logger de archivo vía
  `vgl_log_bridge` en `source/utils/glutil.c`, con deduplicado de mensajes consecutivos
  idénticos para no generar I/O por frame) como herramienta de triage a futuro.
- **Confirmado en consola real:** con la vitaGL vendorizada, el splashscreen 3D, la prueba de
  rojo sólido, y **el menú y el gameplay del juego ya se ven correctamente** a 60 FPS estables,
  sin lentitud. Ambos niveles base (Fire Aura, Original Xbox Level) jugables con audio, input
  táctil/físico y guardado funcionando.
- Toda la instrumentación temporal de diagnóstico (sonda de `sceDisplaySetFrameBuf`, prueba de
  rojo sólido, callback de vitashark, logs `dbg:` de estado/frame en `main.c`) fue removida una
  vez confirmada la causa real, dejando solo `vgl_log_bridge` (deduplicado) como capacidad de
  triage permanente.

## Fase 13: Level Pack (05133-Impossible-Level-Pack.apk) integrado y confirmado en hardware real

- **Análisis confirmado** comparando ambos APKs (classes.dex, resources.arsc, `nm -D` de ambos
  `.so`): el Level Pack es otra app (`com.flukedude.impossiblegamelevelpack`) con 5 niveles —
  0 Fire Aura, 1 Xbox (los mismos 2 del juego base), 2 Chaoz Fantasy, 3 Heaven, 4 Phazd (los 3
  nuevos). Su `libimpossible.so` (armeabi-v7a, 59796 bytes) exporta las mismas 11 funciones JNI
  que el base, con prefijo `Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_*`.
- **Imports nuevos del .so del pack** vs. el base: `glClearColor`, `__cxa_finalize`, `raise` — los
  3 ya estaban resueltos en `dynlib.c` de antes, no hizo falta tocar el loader.
- **Cambios de código:**
  - `source/main.c`: símbolos JNI resueltos con el prefijo `impossiblegamelevelpack`; loops de
    sync de save ampliados de medallas 6→12 y progreso 2→5; selección de BGM por nivel
    (0/1 → `soundtrack.ogg` base, 2/3/4 → `soundtrack2/3/4.ogg` del pack).
  - `source/save.h`: `GameSaveData.medals[6]→[12]`, `progress_practice/noflag[2]→[5]` (confirmado
    por las keys `medal1Unlocked..medal12Unlocked` y `noFlagProgress..noFlagProgress5` del
    classes.dex del pack). El save viejo se descarta solo (mismatch de `sizeof`), progreso actual
    en consola se pierde al actualizar — esperado.
  - `source/audio.h` / `source/audio.c`: agregados `BGM_SOUNDTRACK2/3/4` + sus rutas.
  - `source/menu.c`: selector de nivel de 2 a 5 botones (grid 3+2 con volver), stats de 3 a 6
    subpáginas, medallas de 3 a 6 subpáginas. Nuevos assets copiados a
    `ux0_data/theimpossiblegame/res/drawable/`: `menustartlevel2/3/4button(pressed).png`,
    `statspage4/5/6.png` (sin variante landscape "l" en el pack — se usan igual porque el
    renderer ya estira todo a una caja fija 560×260, pérdida de nitidez menor y cosmética),
    `medalspage4/5/6(l).png`.
- **`.so` swapeado en `ux0_data/theimpossiblegame/libimpossible.so`**; el original de 2 niveles
  queda respaldado como `libimpossible.so.base-2level.bak` en la misma carpeta.
- **Build local OK** (`psvita-toolkit build`), sin errores nuevos (solo warnings preexistentes de
  `mktemp`/`FalsoJNI` no relacionados). VPK y `eboot.bin` regenerados en la raíz del repo.
- **Confirmado en consola real por el usuario:** los 5 niveles (Fire Aura, Xbox, Chaoz Fantasy,
  Heaven, Phazd) son jugables con el `.so` del Level Pack, pese a que el pack oficial (su propio
  Java) nunca invoca `initGame(0)`/`initGame(1)` directamente — llamarlas desde nuestro loop en C
  funciona igual. No hizo falta el plan B (cargar ambos `.so`).

## Fase 14: Detección dinámica y compatibilidad universal Base + Level Pack (v1.2.0 — 2026-10-27)

- **Bug reportado por usuarios:** Al instalar únicamente el juego base (`05133-the-impossible-1.5.2.apk`),
  el juego crasheaba al arrancar con `"Failed to resolve JNI symbols from libimpossible.so"`.
- **Causa raíz:** En la versión v1.1.0 se modificó `main.c` con el prefijo estático
  `Java_com_flukedude_impossiblegamelevelpack_ImpossibleGame_*`, por lo que el `libimpossible.so`
  original del juego base (cuyo prefijo es `Java_com_flukedude_impossiblegame_ImpossibleGame_*`)
  fallaba la resolución de símbolos. Además, el menú y las pantallas de estadísticas/medallas
  asumían fijos 5 niveles y 6 subpáginas, intentando cargar texturas inexistentes en el APK base.
- **Solución implementada:**
  - `source/main.c`: Implementada función `resolve_game_symbol()` con fallback dinámico. Detecta
    automáticamente si el `.so` instalado pertenece al Level Pack (`g_is_level_pack = 1`) o al juego
    base (`g_is_level_pack = 0`).
  - `source/main.c`: Sincronización de guardado condicionada según la variante activa (6 medallas y
    2 niveles de progreso para el juego base; 12 medallas y 5 niveles de progreso para el Level Pack).
  - `source/menu.c`: Adaptación de la interfaz según `g_is_level_pack`:
    - Juego Base (2 niveles): Selector de 2 botones originales + botón Menú con navegación D-Pad y
      touch adaptados; pantallas de Estadísticas y Medallas limitadas a 3 subpáginas, evitando
      intentos de carga de texturas faltantes (`statspage4..6`, `medalspage4..6`, etc.).
    - Level Pack (5 niveles): Mantiene la cuadrícula completa de 5 niveles y 6 subpáginas.
  - Correcciones de compilación para VitaSDK / GCC 15:
    - `source/utils/logger.c`: Eliminada macro obsoleta `ATOMIC_VAR_INIT` para compatibilidad con C23.
    - `lib/falso_jni/`: Corregidos tipos de retorno en `GetPrimitiveArrayCritical` y `jda_sizeof`.
    - `CMakeLists.txt`: Añadido flag `-Wl,--no-warn-mismatch` para permitir linkeo en toolchains con
      librerías precompiladas hardfp.
- **Resultado:** El mismo ejecutable y VPK corre de forma transparente y nativa tanto con el APK
  base (`05133-the-impossible-1.5.2.apk`) como con la expansión (`05133-Impossible-Level-Pack.apk`).

## Fase 15: Corrección de ABI Float (hardfp vs softfp) y Silencio/Fondos Blancos (v1.2.1 — 2026-10-27)

- **Bug reportado en v1.2.0:** Todos los mundos se ven en blanco (los obstáculos y el fondo son quads blancos planos) y no hay ningún tipo de sonido en el juego.
- **Causa raíz:**
  1. En v1.2.0 se introdujo `-Wl,--no-warn-mismatch` en `CMakeLists.txt` forzando el enlace entre el proyecto compilado con `-mfloat-abi=softfp` y la toolchain moderna de VitaSDK (GCC 15.2.0), cuyas librerías de sistema (`libm.a`, `libc.a`, `crt0.o`) están construidas con `-mfloat-abi=hard` (pasando flotantes en registros VFP `s0-s15`).
  2. **Silencio total en audio:** `stb_vorbis` (en `source/audio.c`) llamaba a funciones de `libm.a` (`sin`, `cos`, `pow`, `floor`) pasando los argumentos en registros generales `r0-r3`. Las funciones matemáticas hardfp leían los registros VFP no inicializados, arrojando NaNs/ceros, silenciando por completo la decodificación de audio OGG.
  3. **Fondos y obstáculos blancos en niveles:**
     - En `source/main.c`, la firma de `ImpossibleGame_initGame` tenía `jfloat fps` en el 4º argumento. En modo hardfp, `fps` se ubicaba en el registro VFP `s0`, alterando la distribución de parámetros en el stack. `libimpossible.so` (compilada originalmente en softfp) esperaba `fps` en `[sp, 4]`, `tex1` (`gametextures.png`) en `[sp, 8]`, y `tex2` en `[sp, 12]`. Debido al desplazamiento, `tex1` recibía `tex2` y `tex2` recibía basura/0, dejando las texturas del juego sin enlazar.
     - Las llamadas de `libimpossible.so` a `glColor4f`, `glClearColor`, `glOrthof`, `glTexEnvf`, etc., pasaban argumentos flotantes en `r0-r3`, los cuales eran leídos incorrectamente por VitaGL hardfp.
- **Solución implementada:**
  1. **Migración a hardfp nativo en el ejecutable:** `CMakeLists.txt` unificado a `-mfloat-abi=hard` sin flags de mismatch, eliminando desajustes con `libm.a`, `libc.a`, `libvitashark.a` y VitaGL.
  2. **Bridge wrappers de ABI en `source/dynlib.c`:**
     - `libimpossible.so` únicamente importa 10 funciones que utilizan floats: `sinf`, `cosf`, `truncf`, `round`, `glClearColor`, `glColor4f`, `glTexEnvf`, `glPointSize`, `glTranslatef`, `glRotatef`.
     - Se crearon wrappers intermedios (`*_soloader`) que reciben los floats como `uint32_t` / `uint64_t` (asegurando lectura desde `r0-r3` en la convención ARM AAPCS softfp del SO) y los reenvían como floats nativos hardfp (`s0-s15`) a las implementaciones reales de VitaGL y newlib.
  3. **Alineación de stack en `ImpossibleGame_initGame` (`source/main.c`):**
     - Declaración actualizada con `uint32_t fps_bits` para preservar la ranura exacta de 32 bits en el stack requerida por `libimpossible.so`.
  4. **Activación permanente del logger:** `source/utils/logger.h` ahora preserva `l_info`, `l_warn` y `l_success` para facilitar diagnósticos en consola física.
- **Resultado:**
  - Build limpio de `eboot.bin` (723,589 bytes) y `theimpossiblegame.vpk` (864,333 bytes).
  - Audio OGG funcional con decodificación de punto flotante exacta.
  - Texturas de los niveles enlazadas adecuadamente sin quads blancos.

## Fase 16: Trofeos nativos (v1.3.0 — 2026-11-06)

- **Mapeo:** el motor ya trae 12 medallas (`getMedalUnlocked(i)`): medalla i -> trofeo i+1, 0 = platino
  (lo da el SO). Medallas confirmadas de `medalspage1..6l.png`: 500 intentos, 999 saltos, y por cada
  nivel (1, Xbox, 2, 3, 4) práctica + sin banderas.
- **Hook:** `source/main.c`, en el sync de stats por frame: transición 0->1 de una medalla ->
  `trophies_unlock(i+1)`. Al arrancar se re-envían las medallas del save (desbloqueo retroactivo).
- **Implementación:** plantilla de la skill `psvita-trophies` (`source/trophies.c`, comm_id fijo
  `TIGV00001`; la primera versión copiada de PoP leía `sceAppMgrAppParamGetString(12)`, que devuelve
  el TITLE_ID). TRP: `extras/scripts/generate_trophies.py` (íconos + `trophies.json`) y `make_trp.py`.
  vitaGL ya usaba `vglInitExtended` (deja memoria para el setup dialog).
- **Build:** Docker `vitasdk/vitasdk` (GCC 15.2 hardfp, Release): `eboot.bin` 776,608 bytes,
  `theimpossiblegame.vpk` 953,352 bytes; TRP presente en el VPK con hash de header OK, `APP_VER 01.30`.
  La VitaSDK local de macOS es GCC 10 softfp y no sirve para este port.
- **Pendiente:** confirmar en consola (pantalla de instalación de trofeos + notificación al ganar una medalla).
