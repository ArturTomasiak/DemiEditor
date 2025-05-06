#pragma once
#include "../includes.h"
#include "string.h"
#ifndef demidebug
#endif

typedef struct {
    int32_t location;
    const char* name;    
} shader_uniform_cache;

typedef struct {
    uint32_t renderer_id;
    uint32_t cache_length;
    shader_uniform_cache* cache;
} Shader;

Shader shader_create(const char* vertex_shader_path, const char* fragment_shader_path, int32_t arr_limit);
void shader_delete(Shader* shader);
void shader_bind(const Shader* shader);
void shader_unbind();
void shader_set_uniform1i(Shader* shader, const char* name, int32_t value);
void shader_set_uniform1iv(Shader* shader, const char* name, int32_t* matrix, uint32_t length);
void shader_set_uniform1f(Shader* shader, const char* name, float value);
void shader_set_uniform3f(Shader* shader, const char* name, float v0, float v1, float v2);
void shader_set_uniform4f(Shader* shader, const char* name, float v0, float v1, float v2, float v3);
void shader_set_uniformmat4f(Shader* shader, const char* name, float* matrix, uint32_t length);

// static classes
// char* file_content(const char* location);
// uint32_t shader_compile(uint32_t type, const char* source);
// int32_t get_uniform_location(Shader* shader, const char* name);