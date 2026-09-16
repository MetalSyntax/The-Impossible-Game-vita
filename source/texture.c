#include "texture.h"
#include "utils/logger.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#include "../lib/stb_image.h"

#include <stdio.h>
#include <stdlib.h>

Texture texture_load(const char *filename) {
    Texture tex = {0, 0, 0};

    FILE *f = fopen(filename, "rb");
    if (!f) {
        l_error("Cannot open file: %s", filename);
        return tex;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *buffer = (unsigned char *)malloc(size);
    if (!buffer) {
        fclose(f);
        return tex;
    }

    fread(buffer, 1, size, f);
    fclose(f);

    int w = 0, h = 0, channels = 0;
    unsigned char *data = stbi_load_from_memory(buffer, (int)size, &w, &h, &channels, 4);
    free(buffer);

    if (!data) {
        l_error("stbi_load failed for %s: %s", filename, stbi_failure_reason());
        return tex;
    }

    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    tex.width = w;
    tex.height = h;

    l_info("Loaded texture: %s (%dx%d)", filename, w, h);
    return tex;
}

void texture_free(Texture *tex) {
    if (tex && tex->id) {
        glDeleteTextures(1, &tex->id);
        tex->id = 0;
        tex->width = 0;
        tex->height = 0;
    }
}

static float s_sprite_vertices[8];
static float s_sprite_texcoords[8];

void draw_sprite_part(GLuint tex, float x, float y, float w, float h, float u0, float v0, float u1, float v1, float r, float g, float b, float a) {
    s_sprite_vertices[0] = x;     s_sprite_vertices[1] = y;
    s_sprite_vertices[2] = x + w; s_sprite_vertices[3] = y;
    s_sprite_vertices[4] = x;     s_sprite_vertices[5] = y + h;
    s_sprite_vertices[6] = x + w; s_sprite_vertices[7] = y + h;

    s_sprite_texcoords[0] = u0; s_sprite_texcoords[1] = v0;
    s_sprite_texcoords[2] = u1; s_sprite_texcoords[3] = v0;
    s_sprite_texcoords[4] = u0; s_sprite_texcoords[5] = v1;
    s_sprite_texcoords[6] = u1; s_sprite_texcoords[7] = v1;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, s_sprite_vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, s_sprite_texcoords);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void draw_sprite(GLuint tex, float x, float y, float w, float h, float r, float g, float b, float a) {
    draw_sprite_part(tex, x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f, r, g, b, a);
}

void draw_sprite_centered(GLuint tex, float cx, float cy, float w, float h, float r, float g, float b, float a) {
    draw_sprite(tex, cx - w * 0.5f, cy - h * 0.5f, w, h, r, g, b, a);
}

static float s_rect_vertices[8];

void draw_rect(float x, float y, float w, float h, float r, float g, float b, float a) {
    s_rect_vertices[0] = x;     s_rect_vertices[1] = y;
    s_rect_vertices[2] = x + w; s_rect_vertices[3] = y;
    s_rect_vertices[4] = x;     s_rect_vertices[5] = y + h;
    s_rect_vertices[6] = x + w; s_rect_vertices[7] = y + h;

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, s_rect_vertices);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_TEXTURE_2D);
}
