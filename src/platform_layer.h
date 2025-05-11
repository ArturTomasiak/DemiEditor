#pragma once

#include "includes.h"
#include "character/buffer.h"

typedef enum {
    UTF8,
    UTF16BE,
    UTF16LE,
    UTF32BE,
    UTF32LE,
    PDF
} Encoding;

typedef struct {
    Encoding encoding;
    wchar_t* path;
    _Bool    loaded;
    _Bool    saved_progress;
} DemiFile;

void platform_delete_file(DemiFile* file);
DemiFile platform_open_file(Buffer* restrict buffer);
void platform_save_file(DemiFile* restrict demifile, Buffer* restrict buffer);
uint64_t platform_validate_string(wchar_t* restrict string);