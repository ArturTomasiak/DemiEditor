#include "editor.h"
#include "character/character.h"
#include "character/buffer.h"
#include "shader/shader.h"
#include "cursor/cursor.h"
#include "objects/vao.h"
#include "objects/vbo.h"
#include "math/math.h"

static float translate[16] = {0}, scale_matrix[16] = {0}, projection[16] = {0};

static uint8_t processed_chars = 128;
static uint16_t font_pixels_setting = 20;
static float line_spacing_setting = 1.5f;
static float aspect_ratio = 16.0f / 9.0f; // target aspect ratio for consistency

static uint16_t font_pixels; 
static float line_spacing;
static float dpi_scale;

static int32_t arr_limit;
static int32_t* letter_map; 
static float* transforms;
static float text_x;
static float text_y;

static Buffer buffer;
static Cursor cursor;

static CharacterMap character_map;
static Shader shader;

static uint32_t texture_array;
static vertex_array_object vao;
static vertex_buffer_object vbo;

static FT_Library ft_lib;
static FT_Face ft_face;

static void build_font();
static void build_cursor();
static void create_objects();

static void render_text_call(uint32_t length) {
    shader_set_uniform3f(&shader, "text_color", 1.0f, 1.0f, 1.0f);
    shader_set_uniformmat4f(&shader, "transforms", transforms, length);
    shader_set_uniform1iv(&shader, "letter_map", letter_map, length);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, length);
}

static void render_text() {
    float x = text_x;
    float y = text_y;
    float Xcopy = x;
    uint32_t working_index = 0;
    for (uint64_t i = 0; i < buffer.length; i++) {
        Character character = character_map.character[(uint8_t)buffer.content[i]];
        if (buffer.content[i] == '\n') {
            y -= (character.size.y) * line_spacing;
            x = Xcopy;
        }
        else if (buffer.content[i] == ' ')
            x += character.advance;
        else {
            memset(translate, 0.0f, 16 * sizeof(float));
            memset(scale_matrix, 0.0f, 16 * sizeof(float));
            math_identity_f4x4(scale_matrix, 1.0f);

            float xpos = x + character.bearing.x;
            float ypos = y - (font_pixels - character.bearing.y);

            letter_map[working_index] = character.ch;
            math_translate_f4x4(translate, xpos, ypos, 0);
            math_scale_f4x4(scale_matrix, font_pixels, font_pixels, 0);
            math_multiply_f4x4(transforms + (working_index * 16), translate, scale_matrix);
            x += character.advance;
            working_index++;
            if (working_index == arr_limit) {
                render_text_call(working_index);
                working_index = 0;
            }
        }
    }
    if (working_index != 0)
        render_text_call(working_index);
}

void editor_loop() {
    cursor_update_blink(&cursor);
    cursor_update_position(&cursor, &buffer, &character_map, text_x, text_y);
    cursor_render(&cursor, &shader);
    render_text();
}

void editor_window_size(int32_t width, int32_t height) {
    math_orthographic_f4x4(projection, 0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f);
    glViewport(0, 0, (float)width, (float)height);
    shader_set_uniformmat4f(&shader, "projection", projection, 1);
    text_x = 20.0f * aspect_ratio;
    text_y = height - (20.0f * aspect_ratio) - character_map.character[1].bearing.y;
}

_Bool editor_init(int32_t width, int32_t height) {
    font_pixels  = (uint16_t)(font_pixels_setting * dpi_scale);
    line_spacing = line_spacing_setting * dpi_scale;
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &arr_limit);
    arr_limit = arr_limit >> 5; // return here if anything breaks
    if (arr_limit < 2)
        arr_limit = 2;

    shader = shader_create("..\\resources\\shaders\\vertex.glsl", "..\\resources\\shaders\\fragment.glsl", arr_limit);
    
    shader_bind(&shader);
    editor_window_size(width, height);
    shader_set_uniform1i(&shader, "text", 0);

    buffer = buffer_create();
    cursor = cursor_create();

    if (FT_Init_FreeType(&ft_lib))
        return 0;

    if (FT_New_Face(ft_lib, "..\\resources\\fonts\\arial.ttf", 0, &ft_face))
        return 0;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &texture_array);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    build_font();
    build_cursor();

    for (uint32_t i = 0; i < arr_limit; i++)
        math_identity_f4x4(transforms + (i * 16), 1.0f);

    create_objects();

    return 1;
}

void editor_end_gracefully() {
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_lib);
    free(letter_map);
    free(transforms);
    free(character_map.character);
    shader_delete(&shader);
    buffer_delete(&buffer);
    glDeleteTextures(1, &texture_array);
    vao_delete(&vao);
    vbo_delete(&vbo);
}

void editor_dpi(int32_t dpi) {
    dpi_scale = (float)dpi / 96.0f;
}

_Bool alloc_variables() {
    letter_map = calloc(arr_limit, sizeof(int32_t)); 
    if (!letter_map) goto letter_map_failed;

    transforms = malloc(arr_limit * 16 * sizeof(float));
    if (!transforms) goto transforms_failed;

    character_map.character = malloc(processed_chars * sizeof(Character));
    character_map.length = processed_chars;
    if (!character_map.character) goto character_map_failed;

    return 1;

character_map_failed:
    free(transforms);
transforms_failed:
    free(letter_map);
letter_map_failed:
    error(err_memory_allocation);
    return 0;
}

static void build_font() {
    FT_Set_Pixel_Sizes(ft_face, font_pixels, font_pixels);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, font_pixels, font_pixels, processed_chars, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    for (uint8_t c = 0; c < processed_chars; c++) {
        if (FT_Load_Char(ft_face, c, FT_LOAD_RENDER)) {
            #ifdef demidebug
            warning(__LINE__, __FILE__, "failed to load glyph");
            #endif
            continue;
        }
        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY,
            0, 0, 0, (int32_t)c,
            ft_face->glyph->bitmap.width,
            ft_face->glyph->bitmap.rows, 1,
            GL_RED,
            GL_UNSIGNED_BYTE,
            ft_face->glyph->bitmap.buffer
        );
        Character character = {
            (char)c,
            {ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows},
            {ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top},
            ft_face->glyph->advance.x >> 6 // divide by 64
        };
        character_map.character[c] = character;
    }
}

static void build_cursor() {
    cursor.blink_rate = 500;
    cursor.line_spacing = line_spacing;
    cursor.character = character_map.character['|'];
    cursor.x = text_x;
    cursor.y = text_y;
    cursor.font_pixels = font_pixels;
}

static void create_objects() {
    float vertex_data[8] = {
        0.0f,1.0f,
        0.0f,0.0f,
        1.0f,1.0f,
        1.0f,0.0f,
    };

    vao = vao_create();
    vbo = vbo_create(vertex_data, sizeof(vertex_data));
    
    vertex_buffer_layout layout = vao_create_layout();
    vao_add_element(&layout, 2, GL_FLOAT, sizeof(float), GL_FALSE);
    vao_add_buffer(&vbo, &layout, &vao);
    vao_delete_layout(&layout);
}

void editor_backspace() {
    if (cursor.position > 0 && buffer.length != 0) {
        buffer_delete_char(&buffer, cursor.position);
        cursor.position--;
    }
}

void editor_tab() {
    buffer_insert_string(&buffer, "    ", cursor.position);
    cursor.position += 4;
}

void editor_enter() {
    buffer_insert_char(&buffer, '\n', cursor.position);
    editor_key_down();
}

void editor_input(char ch) {
    buffer_insert_char(&buffer, ch, cursor.position);
    cursor.position++;
}

void editor_key_left() {
    if (cursor.position > 0)
        cursor.position--;
}

void editor_key_right() {
    if (cursor.position < buffer.length) 
        cursor.position++;
}

void editor_key_up() {
    _Bool repeat = 1;
    reach_nl:
    while (cursor.position > 0 && buffer.content[cursor.position - 1] != '\n')
        cursor.position--;
    if (repeat == 1 && cursor.position > 0) {
        cursor.position--;
        repeat = 0;
        goto reach_nl;
    }
}

void editor_key_down() {
    if (cursor.position == 0)
        cursor.position++;
    if (cursor.position + 1 < buffer.length)
        if (buffer.content[cursor.position - 1] == '\n' && buffer.content[cursor.position + 1] == '\n') {
            cursor.position++;
            return;
        }
    if (buffer.content[cursor.position - 1] == '\n')
        cursor.position++;
    while (cursor.position < buffer.length && buffer.content[cursor.position - 1] != '\n')
        cursor.position++;
}

void editor_key_home() {
    cursor.position = 0;
}

void editor_key_end() {
    while (cursor.position < buffer.length && buffer.content[cursor.position] != '\n')
        cursor.position++;
}