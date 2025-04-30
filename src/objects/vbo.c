#include "vbo.h"

VertexBufferObject vbo_create(const void* data, uint32_t size) {
    VertexBufferObject vbo;
    glGenBuffers(1, &vbo.renderer_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.renderer_id);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    return vbo;
}

void vbo_delete(VertexBufferObject* vbo) {
    glDeleteBuffers(1, &vbo->renderer_id);
    vbo->renderer_id = 0;
}

void vbo_bind(const VertexBufferObject* vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo->renderer_id);
}

void vbo_unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}