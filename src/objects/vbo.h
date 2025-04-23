#pragma once

#include "..\includes.h"

typedef struct {
    uint32_t renderer_id;
} vertex_buffer_object;

vertex_buffer_object vbo_create(const void* data, uint32_t size);
void vbo_delete(vertex_buffer_object* vbo);
void vbo_bind(const vertex_buffer_object* vbo);
void vbo_unbind();