#pragma once
#include "../includes.h"

typedef struct {
    uint32_t x;
    uint32_t y;
} Size;

typedef struct {
    uint32_t x;
    uint32_t y;
} Bearing;

typedef struct {
    char ch;
    Size size;
    Bearing bearing;
    uint32_t advance;
} Character;

typedef struct {
    Character* character;
    uint32_t length;
} CharacterMap;