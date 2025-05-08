#include "cursor.h"
#include "../math/math.h"

Cursor cursor_create() {
    Cursor cursor = {0};
    cursor.visible = 1;
    return cursor;
}

void cursor_update_position(Cursor* restrict cursor, Buffer* restrict buffer, CharacterMap* restrict character_map, float x, float y, float nl_height) {
    float start_x = x;
    for (uint64_t i = 0; i < cursor->position && i < buffer->length; i++) {
        Character character = character_map->character[(int32_t)buffer->content[i]];
        if (buffer->content[i] == L'\n') {
            y -= nl_height;
            x = start_x;
        }
        else
            x += character.advance;
    }
    cursor->x = x;
    cursor->y = y;
}

void cursor_update_blink(Cursor* restrict cursor) {
    uint64_t current_time = GetTickCount64();
    if (current_time - cursor->last_blink >= cursor->blink_rate) {
        cursor->visible = !cursor->visible;
        cursor->last_blink = current_time;
    }
}

void cursor_render(Cursor* restrict cursor, Shader* restrict shader) { 
    if (!cursor->visible) return;

    float translate[16] = {0};
    float scale[16] = {0};
    float transform[16] = {0};
    float x = cursor->x + cursor->character.bearing.x;
    float y = cursor->y - (cursor->font_pixels - cursor->character.bearing.y);
    
    math_identity_f4x4(scale, 1.0f);
    math_scale_f4x4(scale, cursor->font_pixels, cursor->font_pixels, 0.0f);
    math_translate_f4x4(translate, x, y, 0.0f);
    math_multiply_f4x4(transform, translate, scale);

    shader_set_uniform3f(shader, "text_color", 1.0f, 1.0f, 1.0f);
    shader_set_uniformmat4f(shader, "transforms", transform, 1);
    shader_set_uniform1i(shader, "letter_map", cursor->character.ch);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}