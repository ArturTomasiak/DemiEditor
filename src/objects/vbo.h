#pragma once

#include "..\includes.h"

typedef struct {
    uint32_t renderer_id;
} VertexBufferObject;

VertexBufferObject vbo_create(const void* data, uint32_t size);
void vbo_delete(VertexBufferObject* vbo);
void vbo_bind(const VertexBufferObject* vbo);
void vbo_unbind();