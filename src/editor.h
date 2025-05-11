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
    _Bool display;
    IconData close_ico;
    Buffer font_pixels_setting;
    Buffer font_pixels_input;
    Buffer line_spacing_setting;
    Buffer line_spacing_input;
    Buffer scroll_speed_setting;
    Buffer scroll_speed_input;

    float editor_font_size;

    int32_t xpos;
    int32_t ypos;
} Settings;

typedef struct {
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
    float scroll_speed;

    float text_x;
    float text_y;
    float nl_height;
    
    Buffer buffer;
    BufferStack undo;
    BufferStack redo;
    Cursor cursor;
    
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

void settings_ico_render(Editor* restrict editor);
void settings_close_ico_calculate(Editor* restrict editor);
void settings_ico_calculate(Editor* restrict editor);

void settings_create(Editor* restrict editor);
void settings_delete(Settings* restrict settings) ;
void settings_render(Editor* restrict editor);
void settings_left_click(Editor* restrict editor, float mouse_x, float mouse_y);

// editor.c 

void text_bind(Editor* restrict editor);
void ico_objects_bind(Editor* restrict editor);

void render_text_call(Shader* restrict shader, float* transforms, int32_t* letter_map, uint32_t length);
void render_text(Editor* restrict editor, Buffer* restrict buffer, float text_x, float text_y);

// static void adjust_ortographic(Editor* restrict editor);
// static void adjust_camera_to_cursor(Editor* restrict editor);
// static void settings_ico_render(Editor* restrict editor);

void editor_loop(Editor* restrict editor);
void editor_window_size(Editor* restrict editor, float width, float height);
Editor editor_create(float width, float height, int32_t dpi);
void editor_delete(Editor* restrict editor);
void editor_dpi(Editor* restrict editor, int32_t dpi);
_Bool alloc_variables(Editor* restrict editor);

// static inline void build_cursor(Editor* restrict editor);
void build_font(Editor* restrict editor, uint16_t font_size);
// static void create_objects(Editor* restrict editor);
// static void file_ico_calculate(Editor* restrict editor);

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