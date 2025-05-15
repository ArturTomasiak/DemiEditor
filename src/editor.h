#pragma once

#include "includes.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

#include "character/character.h"
#include "character/buffer.h"
#include "shader/shader.h"
#include "cursor/cursor.h"
#include "textures/png.h"
#include "objects/vao.h"
#include "objects/vbo.h"
#include "math/math.h"
#include "platform_layer.h"

typedef struct {
    Shader shader;
    VertexArrayObject vao;
    VertexBufferObject vbo;
} IconObjects;

typedef struct {
    PngTexture texture;
    float model[16];
    float translate[16];
    float scale_matrix[16];
    float xpos;
    float ypos;
    float size;
} IconData;

typedef struct {
    VertexArrayObject vao;
    VertexBufferObject vbo;
    Shader shader;

    float font_model[16];
    float line_model[16];
    float scroll_model[16];
} UIBox;

typedef struct { 
    float font_x;
    float font_y;
    float line_x;
    float line_y;
    float scroll_x;
    float scroll_y;
} UIBoxPositions;

typedef enum {
    font,
    line,
    scroll
} SelectedSetting;
    
typedef struct {
    _Bool display;
    IconData close_ico;

    uint8_t cursor_pos;
    _Bool   cursor_display;

    SelectedSetting selected;

    UIBox uibox;
    UIBoxPositions uibox_pos;
    float uibox_width;
    float uibox_height;

    Buffer font_setting;
    Buffer font_input;
    Buffer line_setting;
    Buffer line_input;
    Buffer scroll_setting;
    Buffer scroll_input;

    float font_setting_width;
    float line_setting_width;
    float scroll_setting_width;
    float font_input_width;
    float line_input_width;
    float scroll_input_width;

    float input_spacing;
    float y_spacing;

    float editor_font_size;

    float xpos;
    float font_input_xpos;
    float line_input_xpos;
    float scroll_input_xpos;
    float font_ypos;
    float line_ypos;
    float scroll_ypos;
} Settings;

typedef struct {
    _Bool gl_version_fallback;

    DemiFile file;
    
    uint16_t font_pixels_setting;
    uint16_t font_pixels; 
    float line_spacing_setting;
    float line_spacing;
    float dpi_scale;
    
    int32_t processed_chars;
    int32_t arr_limit;
    
    int32_t* letter_map; 
    float* transforms;
    float* projection;

    float aspect_ratio;
    float height;
    float width;
    float camera_y;
    float camera_x;
    float camera_y_save;
    float camera_x_save;
    float scroll_speed;

    float text_x;
    float text_y;
    float nl_height;
    
    Buffer buffer;
    BufferStack undo;
    BufferStack redo;
    Cursor cursor;
    uint64_t cursor_pos;
    
    CharacterMap character_map;
    Shader shader;

    uint32_t text_texture;
    VertexArrayObject vao;
    VertexBufferObject vbo;
    
    FT_Library ft_lib;
    FT_Face ft_face;

    IconData settings_ico;
    IconData file_ico;
    IconObjects ico_objects;
    Settings settings;
} Editor;

// settings.c 

void settings_render(Editor* restrict editor);
void settings_create(Editor* restrict editor);
void settings_delete(Settings* restrict settings);

void settings_create_buffers (Settings* restrict settings);
void settings_buffer_width(Settings* restrict settings, CharacterMap* restrict character_map);
void settings_close_ico_calculate(Editor* restrict editor);
void settings_ico_calculate(Editor* restrict editor);
void uibox_calculate(Editor* restrict editor);

void settings_left_click(Editor* restrict editor, float mouse_x, float mouse_y);
void settings_backspace(Editor* restrict editor);
void settings_input(Editor* restrict editor, wchar_t ch);
void settings_key_left(Editor* restrict editor);
void settings_key_right(Editor* restrict editor);
void settings_enter(Editor* restrict editor);

uint16_t wstr_to_int(wchar_t* str);

// editor.c 

void editor_loop(Editor* restrict editor);

void text_bind(Editor* restrict editor);
void ico_objects_bind(Editor* restrict editor);

void render_text_call(Shader* restrict shader, float* transforms, int32_t* letter_map, uint32_t length);
void render_text(Editor* restrict editor, Buffer* restrict buffer, float text_x, float text_y);

void ico_render(Editor* restrict editor, float* model);

void adjust_ortographic(Editor* restrict editor);
void editor_window_size(Editor* restrict editor, float width, float height);

Editor editor_create(float width, float height, int32_t dpi, _Bool gl_version_fallback);
void editor_delete(Editor* restrict editor);

void update_font_size(Editor* restrict editor);
void update_font(Editor* restrict editor);
void editor_dpi(Editor* restrict editor, int32_t dpi);
_Bool editor_alloc(Editor* restrict editor);

void build_font(Editor* restrict editor, uint16_t font_size);

void editor_backspace(Editor* restrict editor);
void editor_tab(Editor* restrict editor);
void editor_enter(Editor* restrict editor);
void editor_input(Editor* restrict editor, wchar_t ch);
void editor_paste(Editor* restrict editor, wchar_t* restrict text);
void editor_save(Editor* restrict editor);

void editor_key_left(Editor* restrict editor);
void editor_key_right(Editor* restrict editor);
void editor_key_up(Editor* restrict editor);
void editor_key_down(Editor* restrict editor);
void editor_key_home(Editor* restrict editor);
void editor_key_end(Editor* restrict editor);

void editor_mouse_wheel(Editor* restrict editor, int32_t delta);
void editor_left_click(Editor* restrict editor, float mouse_x, float mouse_y);