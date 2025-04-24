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
    float    line_spacing;
    _Bool    visible;
    float x;
    float y;
    Character character;
} Cursor;

Cursor cursor_create();
void cursor_update_position(Cursor* cursor, Buffer* buffer, CharacterMap* char_map, float start_x, float start_y);
void cursor_update_blink(Cursor* cursor);
void cursor_render(Cursor* cursor, Shader* shader);