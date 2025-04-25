#include "includes.h"

#include "character/character.h"
#include "character/buffer.h"
#include "shader/shader.h"
#include "cursor/cursor.h"
#include "objects/vao.h"
#include "objects/vbo.h"
#include "math/math.h"

typedef struct {
    uint16_t font_pixels_setting;
    float line_spacing_setting;
    uint16_t font_pixels; 
    float line_spacing;
    float dpi_scale;
    
    uint8_t processed_chars;
    int32_t arr_limit;
    
    int32_t* letter_map; 
    float* transforms;
    float* projection;
    float width;
    float height;
    float text_x;
    float text_y;
    
    Buffer buffer;
    Cursor cursor;
    
    CharacterMap character_map;
    Shader shader;
    
    uint32_t texture_array;
    vertex_array_object vao;
    vertex_buffer_object vbo;
    
    FT_Library ft_lib;
    FT_Face ft_face;
    } Editor;

// void render_text_call(uint32_t length);
// void render_text();

void editor_loop(Editor* editor);
void editor_window_size(Editor* editor, float width, float height);
Editor editor_create(float width, float height, int32_t dpi);
void editor_delete(Editor* editor);
void editor_dpi(Editor* editor, int32_t dpi);
_Bool alloc_variables(Editor* editor);

// static void build_font();
// static void build_cursor();
// static void create_objects();

void editor_backspace(Editor* editor);
void editor_tab(Editor* editor);
void editor_enter(Editor* editor);
void editor_input(Editor* editor, char ch);

void editor_key_left(Editor* editor);
void editor_key_right(Editor* editor);
void editor_key_up(Editor* editor);
void editor_key_down(Editor* editor);
void editor_key_home(Editor* editor);
void editor_key_end(Editor* editor);

void editor_left_click(Editor* editor, float mouse_x, float mouse_y);