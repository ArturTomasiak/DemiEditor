#pragma once
#include "../includes.h"

typedef struct {
    uint32_t renderer_id;
    int32_t width, height, bits_per_pixel; 
} PngTexture;

PngTexture texture_create(const char* restrict file_path);
void texture_delete(PngTexture* restrict texture);
void texture_bind(uint32_t slot, const PngTexture* restrict texture);
void texture_unbind();
