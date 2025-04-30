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
    
    uint32_t texture_array;
    VertexArrayObject vao;
    VertexBufferObject vbo;
    
    FT_Library ft_lib;
    FT_Face ft_face;
} Editor;

// static void render_text_call(Editor* restrict editor, uint32_t length);
// static void render_text(Editor* restrict editor);
// static void adjust_ortographic(Editor* restrict editor);
// static void adjust_camera_to_cursor(Editor* restrict editor);

void editor_loop(Editor* restrict editor);
void editor_window_size(Editor* restrict editor, float width, float height);
Editor editor_create(float width, float height, int32_t dpi, uint8_t supported_ch);
void editor_delete(Editor* restrict editor);
void editor_dpi(Editor* restrict editor, int32_t dpi);
_Bool alloc_variables(Editor* restrict editor);

// static void build_font(Editor* restrict editor);
// static void build_cursor(Editor* restrict editor);
// static void create_objects(Editor* restrict editor);

void editor_backspace(Editor* restrict editor);
void editor_tab(Editor* restrict editor);
void editor_enter(Editor* restrict editor);
void editor_input(Editor* restrict editor, char ch);

void editor_key_left(Editor* restrict editor);
void editor_key_right(Editor* restrict editor);
void editor_key_up(Editor* restrict editor);
void editor_key_down(Editor* restrict editor);
void editor_key_home(Editor* restrict editor);
void editor_key_end(Editor* restrict editor);

void editor_mouse_wheel(Editor* restrict editor, int32_t delta);
void editor_left_click(Editor* restrict editor, float mouse_x, float mouse_y);

void editor_paste(Editor* restrict editor, const char* restrict text);