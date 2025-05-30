#include "editor.h"

extern inline void editor_loop(Editor* restrict editor) {
    if (editor->settings.display) {
        settings_render(editor);
        return;
    }
    ico_objects_bind(editor);
    texture_bind(0, &editor->settings_ico.texture);
    ico_render(editor, editor->settings_ico.model);

    texture_bind(0, &editor->file_ico.texture);
    ico_render(editor, editor->file_ico.model);

    text_bind(editor);
    cursor_update_blink(&editor->cursor);
    cursor_render(&editor->cursor, &editor->shader);
    render_text(editor, &editor->buffer, editor->text_x, editor->text_y);

    cursor_update_blink(&editor->cursor);
    cursor_render(&editor->cursor, &editor->shader);
}

static void create_objects(Editor* restrict editor);
static void file_ico_calculate(Editor* restrict editor);

extern inline void text_bind(Editor* restrict editor) {
    shader_bind(&editor->shader);
    vao_bind(&editor->vao);
    vbo_bind(&editor->vbo);
}

extern inline void ico_objects_bind(Editor* restrict editor) {
    shader_bind(&editor->ico_objects.shader);
    vao_bind(&editor->ico_objects.vao);
    vbo_bind(&editor->ico_objects.vbo);
}

extern inline void render_text_call(Shader* restrict shader, float* transforms, int32_t* letter_map, uint32_t length) {
    shader_set_uniform3f(shader, "text_color", 1.0f, 1.0f, 1.0f);
    shader_set_uniformmat4f(shader, "transforms", transforms, length);
    shader_set_uniform1iv(shader, "letter_map", letter_map, length);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, length);
}

void render_text(Editor* restrict editor, Buffer* restrict buffer, float text_x, float text_y) {
    float translate[16] = {0}, scale_matrix[16] = {0};
    float x = text_x;
    float y = text_y;
    float Xcopy = x;
    uint32_t working_index = 0;
    for (uint64_t i = 0; i < buffer->length; i++) {
        Character character = editor->character_map.character[(int32_t)buffer->content[i]];
        if (!character.processed)
            continue;
        if (buffer->content[i] == L'\n') {
            y -= editor->nl_height;
            x = Xcopy;
        }
        else if (buffer->content[i] == L' ')
            x += character.advance;
        else {
            float xpos = x + character.bearing.x;
            float ypos = y - (editor->font_pixels - character.bearing.y);
            if (ypos + editor->nl_height < editor->camera_y ||
                ypos > editor->height + editor->camera_y)
                continue;
            memset(translate, 0.0f, 16 * sizeof(float));
            memset(scale_matrix, 0.0f, 16 * sizeof(float));
            math_identity_f4x4(scale_matrix, 1.0f);

            editor->letter_map[working_index] = character.ch;
            math_translate_f4x4(translate, xpos, ypos, 0.9f);
            math_scale_f4x4(scale_matrix, editor->font_pixels, editor->font_pixels, 0);
            math_multiply_f4x4(editor->transforms + (working_index * 16), translate, scale_matrix);
            x += character.advance;
            working_index++;
            if (working_index == editor->arr_limit) {
                render_text_call(&editor->shader, editor->transforms, editor->letter_map, working_index);
                working_index = 0;
            }
        }  
    }
    if (working_index != 0)
        render_text_call(&editor->shader, editor->transforms, editor->letter_map, working_index);
}

void ico_render(Editor* restrict editor, float* model) {
    shader_set_uniformmat4f(&editor->ico_objects.shader, "model", model, 1);
    shader_set_uniform1i(&editor->ico_objects.shader, "texture_input", 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void adjust_ortographic(Editor* restrict editor) {
    math_orthographic_f4x4(editor->projection, editor->camera_x, editor->width + editor->camera_x, editor->camera_y, editor->height + editor->camera_y, -1.0f, 1.0f);
    shader_bind(&editor->shader);
    shader_set_uniformmat4f(&editor->shader, "projection", editor->projection, 1);
    shader_bind(&editor->ico_objects.shader);
    shader_set_uniformmat4f(&editor->ico_objects.shader, "projection", editor->projection, 1);
    shader_bind(&editor->settings.uibox.shader);
    shader_set_uniformmat4f(&editor->settings.uibox.shader, "projection", editor->projection, 1);
    settings_close_ico_calculate(editor);
    settings_ico_calculate(editor);
    file_ico_calculate(editor);
}

static void adjust_camera_to_cursor(Editor* restrict editor) {
    while (editor->cursor.y < editor->camera_y)
        editor->camera_y -= editor->nl_height;
    while (editor->cursor.y + editor->nl_height > editor->height + editor->camera_y)
        editor->camera_y += editor->nl_height;
    while (editor->cursor.x < editor->camera_x)
        editor->camera_x -= editor->nl_height;
    while (editor->cursor.x > editor->width + editor->camera_x)
        editor->camera_x += editor->nl_height;
    if (editor->cursor.y == editor->text_y)
        editor->camera_y = 0;
    if (editor->cursor.x == editor->text_x )
        editor->camera_x = 0;
    adjust_ortographic(editor);
}

void editor_window_size(Editor* restrict editor, float width, float height) {
    editor->width = width;
    editor->height = height;
    glViewport(0, 0, width, height);
    adjust_ortographic(editor);
    editor->text_x = 40.0f * editor->aspect_ratio;
    editor->text_y = height - 10.0f * editor->aspect_ratio - editor->character_map.character[1].bearing.y;
    uibox_calculate(editor);
    if (!editor->settings.display)
        cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
    else
        editor->settings.cursor_display = 0;
}

static inline void build_cursor(Editor* restrict editor) {
    editor->cursor.blink_rate = 200;
    editor->cursor.character = editor->character_map.character[L'|'];
    editor->cursor.x = editor->text_x;
    editor->cursor.y = editor->text_y;
    editor->cursor.font_pixels = editor->font_pixels;
}

Editor editor_create(float width, float height, int32_t dpi, _Bool gl_version_fallback) {
    Editor editor                    = {0};
    editor.scroll_speed              = 0.30f;
    editor.aspect_ratio              = 16.0f / 9.0f; // target aspect ratio for consistency
    editor.line_spacing_setting      = 1.5f;
    editor.settings.editor_font_size = 20;
    editor.gl_version_fallback       = gl_version_fallback;

    if (FT_Init_FreeType(&editor.ft_lib)) {
        fatal_error(err_freetype_initialization);
        return editor;
    }

    if (FT_New_Face(editor.ft_lib, "..\\resources\\fonts\\arial.ttf", 0, &editor.ft_face)) {
        fatal_error(err_freetype_initialization);
        return editor;
    }

    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &editor.processed_chars); // placeholder solution
    
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &editor.arr_limit);
    editor.arr_limit = editor.arr_limit >> 5; // return here if anything breaks
    if (editor.arr_limit < 10) // arbitraty impossible scenario
        editor.arr_limit = 10;

    if (!editor_alloc(&editor))
        return editor;

    editor_dpi(&editor, dpi);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    editor.shader                = shader_create("..\\resources\\shaders\\text_vertex.glsl", "..\\resources\\shaders\\text_fragment.glsl", editor.arr_limit, editor.gl_version_fallback);
    editor.ico_objects.shader    = shader_create("..\\resources\\shaders\\ico_vertex.glsl", "..\\resources\\shaders\\ico_fragment.glsl", 1, editor.gl_version_fallback);
    editor.settings.uibox.shader = shader_create("..\\resources\\shaders\\uibox_vertex.glsl", "..\\resources\\shaders\\uibox_fragment.glsl", 1, editor.gl_version_fallback);

    shader_bind(&editor.shader);
    shader_set_uniform1i(&editor.shader, "text", 0);
    editor_window_size(&editor, width, height);

    settings_create_buffers(&editor.settings);

    build_font(&editor, editor.settings.editor_font_size);

    editor.buffer = buffer_create(NULL, 0, 250);
    editor.cursor = cursor_create();
    editor.undo = buffer_stack_create();
    editor.redo = buffer_stack_create();
    build_cursor(&editor);
    cursor_update_position(&editor.cursor, &editor.buffer, &editor.character_map, editor.cursor_pos, editor.text_x, editor.text_y, editor.nl_height);

    for (uint32_t i = 0; i < editor.arr_limit; i++)
        math_identity_f4x4(editor.transforms + (i * 16), 1.0f);

    create_objects(&editor);

    const float vertices[16] = {
        0.0f, 1.0f,   0.0f, 1.0f,
        0.0f, 0.0f,   0.0f, 0.0f,
        1.0f, 1.0f,   1.0f, 1.0f,
        1.0f, 0.0f,   1.0f, 0.0f
    };
    
    shader_bind(&editor.ico_objects.shader);

    editor.ico_objects.vao = vao_create();
    editor.ico_objects.vbo = vbo_create(vertices, 4 * 4 * sizeof(vertices));
    
    vertex_buffer_layout layout = vao_create_layout();
    vao_add_element(&layout, 2, GL_FLOAT, sizeof(float), GL_FALSE);
    vao_add_element(&layout, 2, GL_FLOAT, sizeof(float), GL_FALSE);
    vao_add_buffer(&editor.ico_objects.vbo, &layout, &editor.ico_objects.vao);
    vao_delete_layout(&layout);

    settings_create(&editor);

    editor.file_ico.size = 30.0f * editor.aspect_ratio;
    editor.file_ico.texture = texture_create("..\\resources\\icons\\file_ico.png");
    file_ico_calculate(&editor);

    text_bind(&editor);
    editor.file.loaded = 0;

    return editor;
}

void editor_delete(Editor* restrict editor) {
    FT_Done_Face(editor->ft_face);
    FT_Done_FreeType(editor->ft_lib);

    free(editor->projection);
    free(editor->letter_map);
    free(editor->transforms);
    free(editor->character_map.character);

    shader_delete(&editor->shader);
    buffer_delete(&editor->buffer);
    buffer_stack_delete(&editor->undo);
    buffer_stack_delete(&editor->redo);
    glDeleteTextures(1, &editor->text_texture);
    vao_delete(&editor->vao);
    vbo_delete(&editor->vbo);

    shader_delete(&editor->ico_objects.shader);
    vao_delete(&editor->ico_objects.vao);
    vbo_delete(&editor->ico_objects.vbo);
    texture_delete(&editor->settings_ico.texture);
    texture_delete(&editor->file_ico.texture);

    platform_delete_file(&editor->file);

    settings_delete(&editor->settings);
}

// call after dpi/font size changes
extern inline void update_font_size(Editor* restrict editor) {
    editor->font_pixels  = editor->font_pixels_setting * editor->dpi_scale;
    editor->cursor.font_pixels = editor->font_pixels;
}

void editor_dpi(Editor* restrict editor, int32_t dpi) {
    if (dpi < 30) // arbitraty impossible scenario
        dpi = 96;
    editor->dpi_scale = (float)dpi / 96.0f;
    update_font_size(editor);
}

_Bool editor_alloc(Editor* restrict editor) {
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
    fatal_error(err_memory_allocation);
    return 0;
}

extern inline void update_font(Editor* restrict editor) {
    editor->nl_height = editor->character_map.character[L'\n'].size.y * editor->line_spacing;
    editor->text_y = editor->height - 10.0f * editor->aspect_ratio - editor->character_map.character[1].bearing.y;
    editor->cursor.character = editor->character_map.character[L'|'];
    settings_buffer_width(&editor->settings, &editor->character_map);
    uibox_calculate(editor);
    editor->line_spacing = editor->line_spacing_setting * editor->dpi_scale;
    editor->nl_height = editor->character_map.character[L'\n'].size.y * editor->line_spacing;
}

static void build_font(Editor* restrict editor, uint16_t font_size) {
    editor->font_pixels_setting = font_size;
    update_font_size(editor);

    if (editor->text_texture) {
        glDeleteTextures(1, &editor->text_texture);
        editor->text_texture = 0;
    }
    glGenTextures(1, &editor->text_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, editor->text_texture);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    FT_Set_Pixel_Sizes(editor->ft_face, editor->font_pixels, editor->font_pixels);

    uint8_t *empty = calloc(editor->font_pixels * editor->font_pixels * editor->processed_chars, sizeof(uint8_t));
    if (!empty) {
        fatal_error(err_memory_allocation);
        return;
    }

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, editor->font_pixels, editor->font_pixels, editor->processed_chars, 0, GL_RED, GL_UNSIGNED_BYTE, empty);
    free(empty);

    for (int32_t c = 0; c < editor->processed_chars; c++) {
        Character* ch = &editor->character_map.character[c];
        if (FT_Load_Char(editor->ft_face, c, FT_LOAD_RENDER)) {
            #ifdef demidebug
            warning(__LINE__, __FILE__, "failed to load glyph");
            #endif
            continue;
        }
        else if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || iswspace(c))
            goto get_processed;
        else if (editor->ft_face->glyph->bitmap.width == 0 
            || editor->ft_face->glyph->bitmap.rows == 0 
            || editor->ft_face->glyph->bitmap.width > editor->font_pixels 
            || editor->ft_face->glyph->bitmap.rows > editor->font_pixels) {
            ch->processed = 0;
            continue;
        }
        get_processed:
        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY,
            0, 0, 0, c,
            editor->ft_face->glyph->bitmap.width,
            editor->ft_face->glyph->bitmap.rows, 1,
            GL_RED,
            GL_UNSIGNED_BYTE,
            editor->ft_face->glyph->bitmap.buffer
        );
        ch->ch = (wchar_t)c;
        ch->size.x = editor->ft_face->glyph->bitmap.width;
        ch->size.y = editor->ft_face->glyph->bitmap.rows;
        ch->bearing.x = editor->ft_face->glyph->bitmap_left;
        ch->bearing.y = editor->ft_face->glyph->bitmap_top;
        ch->advance = editor->ft_face->glyph->advance.x >> 6;
        ch->processed = 1;
    }
    update_font(editor);

    text_bind(editor);
    shader_set_uniform1i(&editor->shader, "text", 0);
}

static void create_objects(Editor* restrict editor) {
    const float text_vertex_data[8] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    };

    editor->vao = vao_create();
    editor->vbo = vbo_create(text_vertex_data, 4 * sizeof(text_vertex_data));
    
    vertex_buffer_layout layout = vao_create_layout();
    vao_add_element(&layout, 2, GL_FLOAT, sizeof(float), GL_FALSE);
    vao_add_buffer(&editor->vbo, &layout, &editor->vao);
    vao_delete_layout(&layout);
}

static void file_ico_calculate(Editor* restrict editor) {
    editor->file_ico.xpos = editor->settings_ico.xpos;
    editor->file_ico.ypos = editor->height - editor->settings_ico.xpos - editor->settings_ico.size;
    float xpos = editor->file_ico.xpos;
    float ypos = editor->file_ico.ypos - editor->file_ico.size + editor->camera_y;

    memset(editor->file_ico.scale_matrix, 0.0f, 16 * sizeof(float));
    memset(editor->file_ico.translate, 0.0f, 16 * sizeof(float));

    math_identity_f4x4(editor->file_ico.scale_matrix, 1.0f);
    math_identity_f4x4(editor->file_ico.translate, 1.0f);
    
    math_translate_f4x4(editor->file_ico.translate, xpos, ypos, 0.0f);
    math_scale_f4x4(editor->file_ico.scale_matrix, editor->file_ico.size, editor->file_ico.size, 1.0f);
    math_multiply_f4x4(editor->file_ico.model, editor->file_ico.translate, editor->file_ico.scale_matrix);
}

extern inline void editor_backspace(Editor* restrict editor) {
    if (editor->cursor_pos > 0 && editor->buffer.length != 0) {
        buffer_delete_char(&editor->buffer, editor->cursor_pos);
        editor->cursor_pos--;
        cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
        adjust_camera_to_cursor(editor);
        editor->file.saved_progress = 0;
    }
}

extern inline void editor_tab(Editor* restrict editor) {
    buffer_insert_string(&editor->buffer, L"    ", editor->cursor_pos, 4);
    editor->cursor_pos += 4;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
    editor->file.saved_progress = 0;
}

extern inline void editor_enter(Editor* restrict editor) {
    buffer_insert_char(&editor->buffer, L'\n', editor->cursor_pos);
    editor->cursor_pos++;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
    editor->file.saved_progress = 0;
}

extern inline void editor_input(Editor* restrict editor, wchar_t ch) {
    buffer_insert_char(&editor->buffer, ch, editor->cursor_pos);
    if (editor->cursor_pos > editor->buffer.length)
        editor->cursor_pos = editor->buffer.length;
    else
        editor->cursor_pos++;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
    editor->file.saved_progress = 0;
}

extern inline void editor_paste(Editor* restrict editor, wchar_t* restrict text) {
    uint64_t len = platform_validate_string(text, editor->processed_chars);
    buffer_insert_string(&editor->buffer, text, editor->cursor_pos, len);
    editor->cursor_pos += len;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
    editor->file.saved_progress = 0;
}

extern inline void editor_save(Editor* restrict editor) {
    editor->file.saved_progress = 1;
    platform_save_file(&editor->file, &editor->buffer);
}

extern inline void editor_key_left(Editor* restrict editor) {
    if (editor->cursor_pos > 0)
        editor->cursor_pos--;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

extern inline void editor_key_right(Editor* restrict editor) {
    if (editor->cursor_pos <= editor->buffer.length) 
        editor->cursor_pos++;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

extern inline void editor_key_up(Editor* restrict editor) {
    if (editor->cursor_pos == 0)
        return;
    _Bool repeat = 1;
    uint32_t pos = 0;

    reach_nl:
    while (editor->cursor_pos > 0 && editor->buffer.content[editor->cursor_pos - 1] != L'\n') {
        editor->cursor_pos--;
        if (repeat == 1)
            pos++;
    }
    if (repeat == 1 && editor->cursor_pos > 0) {
        editor->cursor_pos--;
        repeat = 0;
        goto reach_nl;
    }
    
    while (pos != 0 && editor->buffer.content[editor->cursor_pos] != L'\n') {
        editor->cursor_pos++;
        pos--;
    }
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

extern inline void editor_key_down(Editor* restrict editor) {
    uint32_t pos = 0;
    if (editor->cursor_pos == editor->buffer.length)
        return;
    while (editor->cursor_pos > 0 && editor->buffer.content[editor->cursor_pos - 1] != L'\n') {
        editor->cursor_pos--;
        pos++;
    }
    editor->cursor_pos++;
    while (editor->cursor_pos + 1 < editor->buffer.length && editor->buffer.content[editor->cursor_pos - 1] != L'\n')
        editor->cursor_pos++; 
    while (pos != 0 && editor->buffer.content[editor->cursor_pos] != L'\n') {
        editor->cursor_pos++;
        pos--;
    }
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

extern inline void editor_key_home(Editor* restrict editor) {
    editor->cursor_pos = 0;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

extern inline void editor_key_end(Editor* restrict editor) {
    while (editor->cursor_pos < editor->buffer.length && editor->buffer.content[editor->cursor_pos] != L'\n')
        editor->cursor_pos++;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

extern inline void editor_mouse_wheel(Editor* restrict editor, int32_t delta) {
    delta *= editor->scroll_speed;
    editor->camera_y += delta;
    if (editor->camera_y > 0)
        editor->camera_y = 0;
    adjust_ortographic(editor);
}

static void text_left_click(Editor* editor, float mouse_x, float mouse_y) {
    float y_start = editor->text_y + editor->nl_height;
    float y_end   = editor->text_y;
    uint64_t index_y = 0;
    y_start -= editor->nl_height;
    y_end   -= editor->nl_height;
    for (uint64_t i = 0; i < editor->buffer.length; i++) {
        if (editor->buffer.content[i] == L'\n') {
            if (mouse_y <= y_start && mouse_y >= y_end)
                index_y = i + 1;
            y_start -= editor->nl_height;
            y_end   -= editor->nl_height;
        }
    }
    
    float x_start = editor->text_x;
    float x_end   = x_start;
    editor->cursor_pos = index_y;
    if (editor->buffer.content[index_y] == L'\n')
        goto left_click_end;
    if (mouse_x < x_start)
        goto left_click_end;
    for (int i = index_y; index_y < editor->buffer.length && editor->buffer.content[i] != L'\n'; i++) {
        Character character = editor->character_map.character[(int32_t)editor->buffer.content[i]];
        x_start = x_end;
        x_end += character.advance;
        index_y = i;
        editor->cursor_pos = i;
        if (mouse_x >= x_start && mouse_x <= x_end) {
            goto left_click_end;
        }
        editor->cursor_pos++;
    }
    left_click_end:
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
}

extern inline void editor_left_click(Editor* restrict editor, float mouse_x, float mouse_y) {
    mouse_y = editor->height - mouse_y;
    mouse_y += editor->camera_y;
    mouse_x += editor->camera_x;
    if (editor->settings.display) {
        settings_left_click(editor, mouse_x, mouse_y);
    }
    else if (mouse_x >= editor->settings_ico.xpos + editor->camera_x 
        && mouse_x <= editor->settings_ico.xpos + editor->settings_ico.size + editor->camera_x 
        && mouse_y <= editor->settings_ico.ypos + editor->camera_y
        && mouse_y >= editor->settings_ico.ypos - editor->settings_ico.size + editor->camera_y) {
            editor->settings.display        = 1;
            editor->settings.cursor_display = 0;
            editor->camera_x_save = editor->camera_x;
            editor->camera_y_save = editor->camera_y;
            editor->camera_x = 0; editor->camera_y = 0;
            adjust_ortographic(editor);
    }
    else if (mouse_x >= editor->file_ico.xpos + editor->camera_x 
        && mouse_x <= editor->file_ico.xpos + editor->file_ico.size + editor->camera_x 
        && mouse_y <= editor->file_ico.ypos + editor->camera_y 
        && mouse_y >= editor->file_ico.ypos - editor->settings_ico.size + editor->camera_y) {
        DemiFile file = platform_open_file(&editor->buffer, editor->processed_chars);
        if (file.loaded) {
            editor->file.loaded = 1;
            editor->file.path = file.path;
            editor->file.saved_progress = 1;
            editor->file.encoding = file.encoding;

            editor->cursor_pos = 0;
            cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
        }
    }
    else
        text_left_click(editor, mouse_x, mouse_y);
}