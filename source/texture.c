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

void draw_sprite_part(GLuint tex, float x, float y, float w, float h, float u0, float v0, float u1, float v1, float r, float g, float b, float a) {
    float vertices[8] = {
        x,     y,
        x + w, y,
        x,     y + h,
        x + w, y + h
    };

    float texcoords[8] = {
        u0, v0,
        u1, v0,
        u0, v1,
        u1, v1
    };

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

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

void draw_rect(float x, float y, float w, float h, float r, float g, float b, float a) {
    float vertices[8] = {
        x,     y,
        x + w, y,
        x,     y + h,
        x + w, y + h
    };

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_TEXTURE_2D);
}
