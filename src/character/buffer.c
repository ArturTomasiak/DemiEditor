#include "buffer.h"

Buffer buffer_create() {
    Buffer buffer;
    buffer.length = 0;
    buffer.allocated_memory = 250;
    buffer.content = malloc(buffer.allocated_memory);
    if (!buffer.content)
        error(err_memory_allocation);
    return buffer;
}

void buffer_delete(Buffer* restrict buffer) {
    free(buffer->content);
    buffer->length = 0;
    buffer->allocated_memory = 0;
}

void buffer_insert_char(Buffer* restrict buffer, char val, uint64_t pos) {
    if (pos > buffer->length)
        pos = buffer->length;
    if (++buffer->length >= buffer->allocated_memory - 1) {
        buffer->allocated_memory += 500;
        buffer->content = realloc(buffer->content, buffer->allocated_memory);
        if (!buffer->content) {
            error(err_memory_allocation);
            return;
        }
    }
    memmove(buffer->content + pos + 1, buffer->content + pos, buffer->length - pos);
    buffer->content[pos] = val;
}

void buffer_delete_char(Buffer* restrict buffer, uint64_t pos) {
    if (pos > buffer->length)
        pos = buffer->length;
    memmove(buffer->content + pos - 1, buffer->content + pos, buffer->length - pos);
    buffer->length--;
}

void buffer_insert_string(Buffer* restrict buffer, const char* str, uint64_t pos, uint32_t len) {
    if (!str || !*str)
        return;
    if (pos > buffer->length)
        pos = buffer->length;
    if (len >= 1) {
        buffer->length += len;
        if (buffer->length >= buffer->allocated_memory) {
            while (buffer-> length >= buffer->allocated_memory)
                buffer->allocated_memory += len;
            buffer->content = realloc(buffer->content, buffer->allocated_memory);
            if (!buffer->content) {
                error(err_memory_allocation);
                return;
            }
        }
        memmove(buffer->content + pos + len, buffer->content + pos, buffer->length - pos - len);
        memcpy(buffer->content + pos, str, len);
    }
}

void buffer_replace_content(Buffer* restrict buffer, const char* str, uint64_t len) {
    if (!str || !*str)
        return;
    if (len >= 1) {
        buffer->length = len;
        if (buffer->length >= buffer->allocated_memory) {
            buffer->allocated_memory = len + 250;
            buffer->content = realloc(buffer->content, buffer->allocated_memory);
            if (!buffer->content) {
                error(err_memory_allocation);
                return;
            }
        }
        memcpy(buffer->content, str, len);
    }
}

BufferStack buffer_stack_create() {
    BufferStack stack;
    stack.buffer = malloc(10 * sizeof(Buffer));
    stack.length = 0;
    return stack;
}

void buffer_stack_delete(BufferStack* stack) {
    free(stack->buffer);
    stack->length = 0;
}