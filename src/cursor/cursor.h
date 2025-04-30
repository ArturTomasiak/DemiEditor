#pragma once
#include "../includes.h"
#include "../shader/shader.h"
#include "../character/character.h"
#include "../character/buffer.h"

typedef struct {
    uint64_t position;
    uint64_t last_blink;
    uint64_t blink_rate;
    uint16_t font_pixels;
    _Bool    visible;
    float x;
    float y;
    Character character;
} Cursor;

Cursor cursor_create();
void cursor_update_position(Cursor* restrict cursor, Buffer* restrict buffer, CharacterMap* restrict char_map, float start_x, float start_y, float nl_height);
void cursor_update_blink(Cursor* restrict cursor);
void cursor_render(Cursor* restrict cursor, Shader* restrict shader);