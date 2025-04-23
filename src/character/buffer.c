#include "buffer.h"

Buffer buffer_create() {
    Buffer buffer;
    buffer.length = 0;
    buffer.allocated_memory = 250;
    buffer.content = malloc(buffer.allocated_memory);
    if (!buffer.content)
        win32_err(err_memory_allocation);
    return buffer;
}

void buffer_delete(Buffer* buffer) {
    free(buffer->content);
    buffer->length = 0;
    buffer->allocated_memory = 0;
}

void buffer_insert_char(Buffer* buffer, char val, uint64_t pos) {
    if (pos > buffer->length)
        pos = buffer->length;
    if (++buffer->length >= buffer->allocated_memory) {
        buffer->allocated_memory += 500;
        buffer->content = realloc(buffer->content, buffer->allocated_memory);
        if (!buffer->content)
            win32_err(err_memory_allocation);
    }
    memmove(buffer->content + pos + 1, buffer->content + pos, buffer->length - pos);
    buffer->content[pos] = val;
}

void buffer_delete_char(Buffer* buffer, uint64_t pos) {
    if (buffer->length == 0)
        return;
    memmove(buffer->content + pos - 1, buffer->content + pos, buffer->length - pos);
    buffer->length--;
}

void buffer_insert_string(Buffer* buffer, const char* str, uint64_t pos) {
    if (!str || !*str)
        return;
    if (pos > buffer->length)
        pos = buffer->length;
    uint64_t len = strlen(str);
    buffer->length += len;
    if (buffer->length >= buffer->allocated_memory) {
        while (buffer-> length >= buffer->allocated_memory)
            buffer->allocated_memory += len;
        buffer->content = realloc(buffer->content, buffer->allocated_memory);
        if (!buffer->content)
            win32_err(err_memory_allocation);
    }
    memmove(buffer->content + pos + len, buffer-> content + pos, buffer->length - pos + 1 - len);
    memcpy(buffer->content + pos, str, len);
}