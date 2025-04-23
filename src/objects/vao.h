#pragma once
#include "../includes.h"
#include "vbo.h"

typedef struct {
    uint32_t renderer_id;
} vertex_array_object;

vertex_array_object vao_create();
void vao_delete(vertex_array_object* vao);
void vao_bind(const vertex_array_object* vao);
void vao_unbind();

typedef struct {
    uint32_t count;
    uint32_t type;
    uint32_t type_size;
    _Bool normalized;
} layout_element;

typedef struct {
    layout_element* elements;
    uint32_t element_count;
    uint32_t stride;
} vertex_buffer_layout;

vertex_buffer_layout vao_create_layout();
void vao_delete_layout(vertex_buffer_layout* layout);
void vao_add_element(vertex_buffer_layout* layout, uint32_t count, uint32_t type, uint32_t type_size, _Bool normalized);
void vao_add_buffer(const vertex_buffer_object* vbo, const vertex_buffer_layout* layout, const vertex_array_object* vao);