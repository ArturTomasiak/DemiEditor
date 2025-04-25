#include "editor.h"

static const float aspect_ratio = 16.0f / 9.0f; // target aspect ratio for consistency

static void build_font(Editor* editor);
static void build_cursor(Editor* editor);
static void create_objects(Editor* editor);

static void render_text_call(Editor* editor, uint32_t length) {
    shader_set_uniform3f(&editor->shader, "text_color", 1.0f, 1.0f, 1.0f);
    shader_set_uniformmat4f(&editor->shader, "transforms", editor->transforms, length);
    shader_set_uniform1iv(&editor->shader, "letter_map", editor->letter_map, length);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, length);
}

static void render_text(Editor* editor) {
    float translate[16] = {0}, scale_matrix[16] = {0};
    float x = editor->text_x;
    float y = editor->text_y;
    float Xcopy = x;
    uint32_t working_index = 0;
    for (uint64_t i = 0; i < editor->buffer.length; i++) {
        Character character = editor->character_map.character[(uint8_t)editor->buffer.content[i]];
        if (editor->buffer.content[i] == '\n') {
            y -= (character.size.y) * editor->line_spacing;
            x = Xcopy;
        }
        else if (editor->buffer.content[i] == ' ')
            x += character.advance;
        else {
            memset(translate, 0.0f, 16 * sizeof(float));
            memset(scale_matrix, 0.0f, 16 * sizeof(float));
            math_identity_f4x4(scale_matrix, 1.0f);

            float xpos = x + character.bearing.x;
            float ypos = y - (editor->font_pixels - character.bearing.y);

            editor->letter_map[working_index] = character.ch;
            math_translate_f4x4(translate, xpos, ypos, 0);
            math_scale_f4x4(scale_matrix, editor->font_pixels, editor->font_pixels, 0);
            math_multiply_f4x4(editor->transforms + (working_index * 16), translate, scale_matrix);
            x += character.advance;
            working_index++;
            if (working_index == editor->arr_limit) {
                render_text_call(editor, working_index);
                working_index = 0;
            }
        }
    }
    if (working_index != 0)
        render_text_call(editor, working_index);
}

void editor_loop(Editor* editor) {
    cursor_update_blink(&editor->cursor);
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y);
    cursor_render(&editor->cursor, &editor->shader);
    render_text(editor);
}

void editor_window_size(Editor* editor, float width, float height) {
    editor->width = width;
    editor->height = height;
    math_orthographic_f4x4(editor->projection, 0.0f, width, 0.0f, height, -1.0f, 1.0f);
    glViewport(0, 0, width, height);
    shader_set_uniformmat4f(&editor->shader, "projection", editor->projection, 1);
    editor->text_x = 20.0f * aspect_ratio;
    editor->text_y = height - (20.0f * aspect_ratio) - editor->character_map.character[1].bearing.y;
}

Editor editor_create(float width, float height, int32_t dpi) {
    Editor editor = {0};
    editor.processed_chars      = 128;
    editor.font_pixels_setting  = 20;
    editor.line_spacing_setting = 1.5f;
    editor.font_pixels  = (uint16_t)(editor.font_pixels_setting * editor.dpi_scale);
    editor.line_spacing = editor.line_spacing_setting * editor.dpi_scale;
    
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &editor.arr_limit);
    editor.arr_limit = editor.arr_limit >> 5; // return here if anything breaks
    if (editor.arr_limit < 2)
        editor.arr_limit = 2;

    if (!alloc_variables(&editor))
        return editor;

    editor_dpi(&editor, dpi);
    editor.font_pixels  = editor.font_pixels_setting * editor.dpi_scale;
    editor.line_spacing = editor.line_spacing_setting * editor.dpi_scale;

    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    editor.shader = shader_create("..\\resources\\shaders\\vertex.glsl", "..\\resources\\shaders\\fragment.glsl", editor.arr_limit);

    shader_bind(&editor.shader);
    editor_window_size(&editor, width, height);
    shader_set_uniform1i(&editor.shader, "text", 0);

    if (FT_Init_FreeType(&editor.ft_lib)) {
        error(err_freetype_initialization);
        return editor;
    }

    if (FT_New_Face(editor.ft_lib, "..\\resources\\fonts\\arial.ttf", 0, &editor.ft_face)) {
        error(err_freetype_initialization);
        return editor;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &editor.texture_array);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, editor.texture_array);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    build_font(&editor);
    editor.buffer = buffer_create();
    editor.cursor = cursor_create();
    build_cursor(&editor);

    for (uint32_t i = 0; i < editor.arr_limit; i++)
        math_identity_f4x4(editor.transforms + (i * 16), 1.0f);

    create_objects(&editor);

    return editor;
}

void editor_delete(Editor* editor) {
    FT_Done_Face(editor->ft_face);
    FT_Done_FreeType(editor->ft_lib);
    free(editor->projection);
    free(editor->letter_map);
    free(editor->transforms);
    free(editor->character_map.character);
    shader_delete(&editor->shader);
    buffer_delete(&editor->buffer);
    glDeleteTextures(1, &editor->texture_array);
    vao_delete(&editor->vao);
    vbo_delete(&editor->vbo);
}

void editor_dpi(Editor* editor, int32_t dpi) {
    editor->dpi_scale = (float)dpi / 96.0f;
}

_Bool alloc_variables(Editor* editor) {
    editor->projection = calloc(16, sizeof(float));
    if (!editor->projection) goto projection_failed;

    editor->letter_map = calloc(editor->arr_limit, sizeof(int32_t)); 
    if (!editor->letter_map) goto letter_map_failed;

    editor->transforms = calloc(editor->arr_limit * 16, sizeof(float));
    if (!editor->transforms) goto transforms_failed;

    editor->character_map.character = malloc(editor->processed_chars * sizeof(Character));
    editor->character_map.length = editor->processed_chars;
    if (!editor->character_map.character) goto character_map_failed;

    return 1;

character_map_failed:
    free(editor->transforms);
transforms_failed:
    free(editor->letter_map);
letter_map_failed:
    free(editor->projection);
projection_failed:
    error(err_memory_allocation);
    return 0;
}

static void build_font(Editor* editor) {
    FT_Set_Pixel_Sizes(editor->ft_face, editor->font_pixels, editor->font_pixels);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, editor->font_pixels, editor->font_pixels, editor->processed_chars, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    for (uint8_t c = 0; c < editor->processed_chars; c++) {
        if (FT_Load_Char(editor->ft_face, c, FT_LOAD_RENDER)) {
            #ifdef demidebug
            warning(__LINE__, __FILE__, "failed to load glyph");
            #endif
            continue;
        }
        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY,
            0, 0, 0, (int32_t)c,
            editor->ft_face->glyph->bitmap.width,
            editor->ft_face->glyph->bitmap.rows, 1,
            GL_RED,
            GL_UNSIGNED_BYTE,
            editor->ft_face->glyph->bitmap.buffer
        );
        Character character = {
            (char)c,
            {editor->ft_face->glyph->bitmap.width, editor->ft_face->glyph->bitmap.rows},
            {editor->ft_face->glyph->bitmap_left, editor->ft_face->glyph->bitmap_top},
            editor->ft_face->glyph->advance.x >> 6 // divide by 64
        };
        editor->character_map.character[c] = character;
    }
}

static void build_cursor(Editor* editor) {
    editor->cursor.blink_rate = 500;
    editor->cursor.line_spacing = editor->line_spacing;
    editor->cursor.character = editor->character_map.character['|'];
    editor->cursor.x = editor->text_x;
    editor->cursor.y = editor->text_y;
    editor->cursor.font_pixels = editor->font_pixels;
}

static void create_objects(Editor* editor) {
    float vertex_data[8] = {
        0.0f,1.0f,
        0.0f,0.0f,
        1.0f,1.0f,
        1.0f,0.0f,
    };

    editor->vao = vao_create();
    editor->vbo = vbo_create(vertex_data, sizeof(vertex_data));
    
    vertex_buffer_layout layout = vao_create_layout();
    vao_add_element(&layout, 2, GL_FLOAT, sizeof(float), GL_FALSE);
    vao_add_buffer(&editor->vbo, &layout, &editor->vao);
    vao_delete_layout(&layout);
}

void editor_backspace(Editor* editor) {
    if (editor->cursor.position > 0 && editor->buffer.length != 0) {
        buffer_delete_char(&editor->buffer, editor->cursor.position);
        editor->cursor.position--;
    }
}

void editor_tab(Editor* editor) {
    buffer_insert_string(&editor->buffer, "    ", editor->cursor.position);
    editor->cursor.position += 4;
}

void editor_enter(Editor* editor) {
    buffer_insert_char(&editor->buffer, '\n', editor->cursor.position);
    editor_key_down(editor);
}

void editor_input(Editor* editor, char ch) {
    buffer_insert_char(&editor->buffer, ch, editor->cursor.position);
    editor->cursor.position++;
}

void editor_key_left(Editor* editor) {
    if (editor->cursor.position > 0)
        editor->cursor.position--;
}

void editor_key_right(Editor* editor) {
    if (editor->cursor.position < editor->buffer.length) 
        editor->cursor.position++;
}

void editor_key_up(Editor* editor) {
    _Bool repeat = 1;
    reach_nl:
    while (editor->cursor.position > 0 && editor->buffer.content[editor->cursor.position - 1] != '\n')
        editor->cursor.position--;
    if (repeat == 1 && editor->cursor.position > 0) {
        editor->cursor.position--;
        repeat = 0;
        goto reach_nl;
    }
}

void editor_key_down(Editor* editor) {
    if (editor->cursor.position == 0)
        editor->cursor.position++;
    if (editor->cursor.position + 1 < editor->buffer.length)
        if (editor->buffer.content[editor->cursor.position - 1] == '\n' && editor->buffer.content[editor->cursor.position + 1] == '\n') {
            editor->cursor.position++;
            return;
        }
    if (editor->buffer.content[editor->cursor.position - 1] == '\n')
        editor->cursor.position++;
    while (editor->cursor.position < editor->buffer.length && editor->buffer.content[editor->cursor.position - 1] != '\n')
        editor->cursor.position++;
}

void editor_key_home(Editor* editor) {
    editor->cursor.position = 0;
}

void editor_key_end(Editor* editor) {
    while (editor->cursor.position < editor->buffer.length && editor->buffer.content[editor->cursor.position] != '\n')
    editor->cursor.position++;
}

void editor_left_click(Editor* editor, float mouse_x, float mouse_y) {
    #ifdef demidebug
    printf("%f %f\n", mouse_x, mouse_y);
    #endif
}