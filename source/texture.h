#ifndef TEXTURE_H
#define TEXTURE_H

#include <vitasdk.h>
#include <vitaGL.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    GLuint id;
    int width;
    int height;
} Texture;

Texture texture_load(const char *filename);
void texture_free(Texture *tex);

void draw_sprite(GLuint tex, float x, float y, float w, float h, float r, float g, float b, float a);
void draw_sprite_centered(GLuint tex, float cx, float cy, float w, float h, float r, float g, float b, float a);
void draw_sprite_part(GLuint tex, float x, float y, float w, float h, float u0, float v0, float u1, float v1, float r, float g, float b, float a);
void draw_rect(float x, float y, float w, float h, float r, float g, float b, float a);

#ifdef __cplusplus
}
#endif

#endif // TEXTURE_H
