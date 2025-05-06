#pragma once
#include "../includes.h"

typedef struct {
    char* content;
    uint64_t length;
    uint64_t allocated_memory;
} Buffer;

typedef struct {
    Buffer* buffer;
    uint64_t length;
} BufferStack;

Buffer buffer_create();
void buffer_delete(Buffer* restrict buffer);
void buffer_insert_char(Buffer* restrict buffer, char val, uint64_t pos);
void buffer_delete_char(Buffer* restrict buffer, uint64_t pos);
void buffer_insert_string(Buffer* restrict buffer, const char* str, uint64_t pos, uint32_t len);
void buffer_replace_content(Buffer* restrict buffer, const char* str, uint64_t len);

BufferStack buffer_stack_create();
void buffer_stack_delete(BufferStack* stack);