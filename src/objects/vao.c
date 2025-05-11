#include "vao.h"

VertexArrayObject vao_create() {
    VertexArrayObject vao = {0};
    glGenVertexArrays(1, &vao.renderer_id);
    return vao;
}

void vao_delete(VertexArrayObject* vao) {
    glDeleteVertexArrays(1, &vao->renderer_id);
    vao->renderer_id = 0;
}

extern inline void vao_bind(const VertexArrayObject* vao) {
    glBindVertexArray(vao->renderer_id);
}

extern inline void vao_unbind() {
    glBindVertexArray(0);
}

vertex_buffer_layout vao_create_layout() {
    vertex_buffer_layout layout = {0};
    layout.elements = NULL;
    return layout;
}

void vao_delete_layout(vertex_buffer_layout* layout) {
    if (layout->elements) {
        free(layout->elements);
        layout->elements = NULL;
    }
    layout->element_count = 0;
    layout->stride = 0;
}

void vao_add_element(vertex_buffer_layout* layout, uint32_t count, uint32_t type, uint32_t type_size, _Bool normalized) {
    layout->element_count++;
    layout->stride += count * type_size;
    layout->elements = realloc(layout->elements, sizeof(layout_element) * layout->element_count);
    if (!layout->elements)
        fatal_error(err_memory_allocation);
    layout->elements[layout->element_count - 1].count = count;
    layout->elements[layout->element_count - 1].type = type;
    layout->elements[layout->element_count - 1].type_size = type_size;
    layout->elements[layout->element_count - 1].normalized = normalized;
}

void vao_add_buffer(const VertexBufferObject* vbo, const vertex_buffer_layout* layout, const VertexArrayObject* vao) {
    vao_bind(vao);
    uint64_t offset = 0;
    for (uint32_t i = 0; i < layout->element_count; i++) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 
            layout->elements[i].count, 
            layout->elements[i].type, 
            layout->elements[i].normalized, 
            layout->stride, 
            (const void *) offset
        );
        offset += layout->elements[i].type_size * layout->elements[i].count;
    }
}