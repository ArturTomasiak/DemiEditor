#pragma once
#include "../includes.h"

typedef struct {
    char* content;
    uint64_t length;
    uint64_t allocated_memory;
} Buffer;

Buffer buffer_create();
void buffer_delete(Buffer* buffer);
void buffer_insert_char(Buffer* buffer, char val, uint64_t pos);
void buffer_delete_char(Buffer* buffer, uint64_t pos);
void buffer_insert_string(Buffer* buffer, const char* str, uint64_t pos);