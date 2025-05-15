#include "buffer.h"

Buffer buffer_create(wchar_t* str, uint32_t length, uint32_t allocated_memory) {
    Buffer buffer;
    buffer.length = 0;
    buffer.allocated_memory = allocated_memory;
    buffer.content = malloc(buffer.allocated_memory * sizeof(wchar_t));
    if (!buffer.content)
        fatal_error(err_memory_allocation);
    if (str) {
        memcpy(buffer.content, str, length * sizeof(wchar_t));
        buffer.length = length;
    }
    return buffer;
}

void buffer_delete(Buffer* restrict buffer) {
    free(buffer->content);
    buffer->length = 0;
    buffer->allocated_memory = 0;
}

void buffer_insert_char(Buffer* restrict buffer, wchar_t val, uint64_t pos) {
    if (pos > buffer->length)
        pos = buffer->length;
    if (++buffer->length >= buffer->allocated_memory) {
        buffer->allocated_memory += 500;
        buffer->content = realloc(buffer->content, buffer->allocated_memory * sizeof(wchar_t));
        if (!buffer->content) {
            fatal_error(err_memory_allocation);
            return;
        }
    }
    wmemmove(buffer->content + pos + 1, buffer->content + pos, buffer->length - pos);
    buffer->content[pos] = val;
    buffer->content[buffer->length] = '\0';
}

void buffer_delete_char(Buffer* restrict buffer, uint64_t pos) {
    if (pos > buffer->length)
        pos = buffer->length;
    wmemmove(buffer->content + pos - 1, buffer->content + pos, buffer->length - pos);
    buffer->length--;
    buffer->content[buffer->length] = '\0';
}

void buffer_insert_string(Buffer* restrict buffer, const wchar_t* str, uint64_t pos, uint32_t len) {
    if (!str || !*str)
        return;
    if (pos > buffer->length)
        pos = buffer->length;
    if (len >= 1) {
        buffer->length += len;
        buffer->allocated_memory += len;
        buffer->content = realloc(buffer->content, buffer->allocated_memory * sizeof(wchar_t));
        if (!buffer->content) {
            fatal_error(err_memory_allocation);
            return;
        }
        wmemmove(buffer->content + pos + len, buffer->content + pos, buffer->length - pos - len);
        memcpy(buffer->content + pos, str, len * sizeof(wchar_t));
    }
    buffer->content[buffer->length] = '\0';
}

void buffer_replace_content(Buffer* restrict buffer, const wchar_t* str, uint64_t len) {
    if (!str || !*str)
        return;
    if (len >= 1) {
        buffer->length = len;
        if (buffer->length >= buffer->allocated_memory) {
            buffer->allocated_memory = len;
            buffer->content = realloc(buffer->content, buffer->allocated_memory * sizeof(wchar_t));
            if (!buffer->content) {
                fatal_error(err_memory_allocation);
                return;
            }
        }
        memcpy(buffer->content, str, len * sizeof(wchar_t));
    }
    buffer->content[buffer->length] = '\0';
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