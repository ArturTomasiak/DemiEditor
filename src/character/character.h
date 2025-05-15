#pragma once
#include "../includes.h"

typedef struct {
    uint32_t x;
    uint32_t y;
} Size;

typedef struct {
    int32_t x;
    int32_t y;
} Bearing;

typedef struct {
    wchar_t ch;
    Size size;
    Bearing bearing;
    uint32_t advance;
    _Bool processed;
} Character;

typedef struct {
    Character* character;
    uint32_t length;
} CharacterMap;