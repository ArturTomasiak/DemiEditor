#include "editor.h"

static void build_font(Editor* restrict editor);
static void build_cursor(Editor* restrict editor);
static void create_objects(Editor* restrict editor);
static void file_ico_calculate(Editor* restrict editor);

static void text_bind(Editor* restrict editor) {
    shader_bind(&editor->shader);
    vao_bind(&editor->vao);
    vbo_bind(&editor->vbo);
}

static void ico_objects_bind(Editor* restrict editor) {
    shader_bind(&editor->ico_objects.shader);
    vao_bind(&editor->ico_objects.vao);
    vbo_bind(&editor->ico_objects.vbo);
}

static void render_text_call(Editor* restrict editor, uint32_t length) {
    shader_set_uniform3f(&editor->shader, "text_color", 1.0f, 1.0f, 1.0f);
    shader_set_uniformmat4f(&editor->shader, "transforms", editor->transforms, length);
    shader_set_uniform1iv(&editor->shader, "letter_map", editor->letter_map, length);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, length);
}

static void render_text(Editor* restrict editor) {
    float translate[16] = {0}, scale_matrix[16] = {0};
    float x = editor->text_x;
    float y = editor->text_y;
    float Xcopy = x;
    uint32_t working_index = 0;
    for (uint64_t i = 0; i < editor->buffer.length; i++) {
        Character character = editor->character_map.character[(int32_t)editor->buffer.content[i]];
        if (!character.processed)
            continue;
        if (editor->buffer.content[i] == '\n') {
            y -= editor->nl_height;
            x = Xcopy;
        }
        else if (editor->buffer.content[i] == ' ')
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

static void adjust_ortographic(Editor* restrict editor) {
    math_orthographic_f4x4(editor->projection, editor->camera_x, editor->width + editor->camera_x, editor->camera_y, editor->height + editor->camera_y, -1.0f, 1.0f);
    shader_set_uniformmat4f(&editor->shader, "projection", editor->projection, 1);
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
    if (editor->cursor.x < editor->text_x)
        editor->camera_x = 0;
    adjust_ortographic(editor);
    settings_ico_calculate(editor);
    file_ico_calculate(editor);
}

static void file_ico_render(Editor* restrict editor) {
    shader_set_uniformmat4f(&editor->ico_objects.shader, "projection", editor->projection, 1);
    shader_set_uniformmat4f(&editor->ico_objects.shader, "model", editor->file_ico.model, 1);
    shader_set_uniform1i(&editor->ico_objects.shader, "icon_texture", 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void editor_loop(Editor* restrict editor) {
    ico_objects_bind(editor);
    texture_bind(0, &editor->settings_ico.texture);
    settings_ico_render(editor);
    if (editor->settings.display)
        settings_render(editor);

    texture_bind(0, &editor->file_ico.texture);
    file_ico_render(editor);

    text_bind(editor);
    cursor_update_blink(&editor->cursor);
    cursor_render(&editor->cursor, &editor->shader);
    render_text(editor);
}

void editor_window_size(Editor* restrict editor, float width, float height) {
    editor->width = width;
    editor->height = height;
    glViewport(0, 0, width, height);
    adjust_ortographic(editor);
    settings_ico_calculate(editor);
    file_ico_calculate(editor);
    editor->text_x = 40.0f * editor->aspect_ratio;
    editor->text_y = height - 10.0f * editor->aspect_ratio - editor->character_map.character[1].bearing.y;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
}

Editor editor_create(float width, float height, int32_t dpi) {
    Editor editor               = {0};
    editor.scroll_speed         = 0.30f;
    editor.aspect_ratio         = 16.0f / 9.0f; // target aspect ratio for consistency
    editor.font_pixels_setting  = 20;
    editor.line_spacing_setting = 1.5f;

    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &editor.processed_chars); // placeholder solution
    
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &editor.arr_limit);
    editor.arr_limit = editor.arr_limit >> 5; // return here if anything breaks
    if (editor.arr_limit < 10) // arbitraty impossible scenario
        editor.arr_limit = 10;

    if (!alloc_variables(&editor))
        return editor;

    editor_dpi(&editor, dpi);

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
    // editor.dpi hasn't set nl_height because character map didn't exist
    editor.nl_height = editor.character_map.character['\n'].size.y * editor.line_spacing;

    editor.buffer = buffer_create();
    editor.cursor = cursor_create();
    editor.undo = buffer_stack_create();
    editor.redo = buffer_stack_create();
    build_cursor(&editor);

    cursor_update_position(&editor.cursor, &editor.buffer, &editor.character_map, editor.text_x, editor.text_y, editor.nl_height);

    for (uint32_t i = 0; i < editor.arr_limit; i++)
        math_identity_f4x4(editor.transforms + (i * 16), 1.0f);

    create_objects(&editor);

    // settings

    editor.settings.display  = 0;
    editor.settings_ico.size = 30.0f * editor.aspect_ratio;

    const float quad_vertices[16] = {
        0.0f, 1.0f,   0.0f, 1.0f,
        0.0f, 0.0f,   0.0f, 0.0f,
        1.0f, 1.0f,   1.0f, 1.0f,
        1.0f, 0.0f,   1.0f, 0.0f
    };
    
    editor.ico_objects.shader = shader_create("..\\resources\\shaders\\settings_ico_vertex.glsl", "..\\resources\\shaders\\settings_ico_fragment.glsl", 1);
    
    shader_bind(&editor.ico_objects.shader);

    editor.ico_objects.vao = vao_create();
    editor.ico_objects.vbo = vbo_create(quad_vertices, 4 * 4 * sizeof(quad_vertices));
    
    vertex_buffer_layout layout = vao_create_layout();
    vao_add_element(&layout, 2, GL_FLOAT, sizeof(float), GL_FALSE);
    vao_add_element(&layout, 2, GL_FLOAT, sizeof(float), GL_FALSE);
    vao_add_buffer(&editor.ico_objects.vbo, &layout, &editor.ico_objects.vao);
    vao_delete_layout(&layout);
    
    editor.settings_ico.texture = texture_create("..\\resources\\icons\\settings_ico.png");

    settings_ico_calculate(&editor);

    // file

    editor.file_ico.size = 30.0f * editor.aspect_ratio;
    editor.file_ico.texture = texture_create("..\\resources\\icons\\file_ico.png");
    file_ico_calculate(&editor);

    text_bind(&editor);

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
    glDeleteTextures(1, &editor->texture_array);
    vao_delete(&editor->vao);
    vbo_delete(&editor->vbo);
}

void editor_dpi(Editor* restrict editor, int32_t dpi) {
    if (dpi < 30) // arbitraty impossible scenario
        dpi = 96;
    editor->dpi_scale = (float)dpi / 96.0f;
    editor->font_pixels  = editor->font_pixels_setting * editor->dpi_scale;
    editor->line_spacing = editor->line_spacing_setting * editor->dpi_scale;
    if (editor->character_map.character)
        editor->nl_height = editor->character_map.character['\n'].size.y * editor->line_spacing;
}

_Bool alloc_variables(Editor* restrict editor) {
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

static void build_font(Editor* restrict editor) {
    FT_Set_Pixel_Sizes(editor->ft_face, editor->font_pixels, editor->font_pixels);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, editor->font_pixels, editor->font_pixels, editor->processed_chars, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    for (int32_t c = 0; c < editor->processed_chars; c++) {
        if (FT_Load_Char(editor->ft_face, c, FT_LOAD_RENDER)) {
            #ifdef demidebug
            warning(__LINE__, __FILE__, "failed to load glyph");
            #endif
            continue;
        }
        else if (editor->ft_face->glyph->bitmap.width == 0 || editor->ft_face->glyph->bitmap.rows == 0 ||
            editor->ft_face->glyph->bitmap.width > editor->font_pixels || editor->ft_face->glyph->bitmap.rows > editor->font_pixels) {
                Character character = {(wchar_t)c, {0, 0}, {0, 0}, 0, 0};
                editor->character_map.character[c] = character;
                continue;
            }
        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY,
            0, 0, 0, c,
            editor->ft_face->glyph->bitmap.width,
            editor->ft_face->glyph->bitmap.rows, 1,
            GL_RED,
            GL_UNSIGNED_BYTE,
            editor->ft_face->glyph->bitmap.buffer
        );
        Character character = {
            (wchar_t)c,
            {editor->ft_face->glyph->bitmap.width, editor->ft_face->glyph->bitmap.rows},
            {editor->ft_face->glyph->bitmap_left, editor->ft_face->glyph->bitmap_top},
            editor->ft_face->glyph->advance.x >> 6, // divide by 64
            1
        };
        editor->character_map.character[c] = character;
    }
}

static void build_cursor(Editor* restrict editor) {
    editor->cursor.blink_rate = 200;
    editor->cursor.character = editor->character_map.character['|'];
    editor->cursor.x = editor->text_x;
    editor->cursor.y = editor->text_y;
    editor->cursor.font_pixels = editor->font_pixels;
}

static void create_objects(Editor* restrict editor) {
    const float text_vertex_data[8] = {
        0.0f,1.0f,
        0.0f,0.0f,
        1.0f,1.0f,
        1.0f,0.0f,
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
    ico_objects_bind(editor);
    float xpos = editor->file_ico.xpos;
    float ypos = editor->file_ico.ypos - editor->file_ico.size + editor->camera_y;

    memset(editor->file_ico.scale_matrix, 0.0f, 16 * sizeof(float));
    memset(editor->file_ico.translate, 0.0f, 16 * sizeof(float));

    math_identity_f4x4(editor->file_ico.scale_matrix, 1.0f);
    math_identity_f4x4(editor->file_ico.translate, 1.0f);
    
    math_translate_f4x4(editor->file_ico.translate, xpos, ypos, 0);
    math_scale_f4x4(editor->file_ico.scale_matrix, editor->file_ico.size, editor->file_ico.size, 1.0f);
    math_multiply_f4x4(editor->file_ico.model, editor->file_ico.translate, editor->file_ico.scale_matrix);
    text_bind(editor);
}

void editor_backspace(Editor* restrict editor) {
    if (editor->cursor.position > 0 && editor->buffer.length != 0) {
        buffer_delete_char(&editor->buffer, editor->cursor.position);
        editor->cursor.position--;
        cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
        adjust_camera_to_cursor(editor);
    }
}

void editor_tab(Editor* restrict editor) {
    buffer_insert_string(&editor->buffer, L"    ", editor->cursor.position, 4);
    editor->cursor.position += 4;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

void editor_enter(Editor* restrict editor) {
    buffer_insert_char(&editor->buffer, L'\n', editor->cursor.position);
    editor->cursor.position++;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

void editor_input(Editor* restrict editor, wchar_t ch) {
    if (ch < editor->processed_chars)
        buffer_insert_char(&editor->buffer, ch, editor->cursor.position);
    if (editor->cursor.position < editor->buffer.length)
        editor->cursor.position++;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

void editor_paste(Editor* restrict editor, const wchar_t* restrict text) {
    uint32_t len = 0;
    while (text[len] != '\0') {
        if (!(text[len] >= 20 && text[len] < editor->processed_chars))
            if (text[len] != '\n')
                return;
        len++;
    }
    buffer_insert_string(&editor->buffer, text, editor->cursor.position, len);
    editor->cursor.position += len;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
}

void editor_key_left(Editor* restrict editor) {
    if (editor->cursor.position > 0)
        editor->cursor.position--;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

void editor_key_right(Editor* restrict editor) {
    if (editor->cursor.position <= editor->buffer.length) 
        editor->cursor.position++;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

void editor_key_up(Editor* restrict editor) {
    if (editor->cursor.position == 0)
        return;
    _Bool repeat = 1;
    uint32_t pos = 0;

    reach_nl:
    while (editor->cursor.position > 0 && editor->buffer.content[editor->cursor.position - 1] != '\n') {
        editor->cursor.position--;
        if (repeat == 1)
            pos++;
    }
    if (repeat == 1 && editor->cursor.position > 0) {
        editor->cursor.position--;
        repeat = 0;
        goto reach_nl;
    }
    
    while (pos != 0 && editor->buffer.content[editor->cursor.position] != '\n') {
        editor->cursor.position++;
        pos--;
    }
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

void editor_key_down(Editor* restrict editor) {
    uint32_t pos = 0;
    if (editor->cursor.position == editor->buffer.length)
        return;
    while (editor->cursor.position > 0 && editor->buffer.content[editor->cursor.position - 1] != '\n') {
        editor->cursor.position--;
        pos++;
    }
    editor->cursor.position++;
    while (editor->cursor.position + 1 < editor->buffer.length && editor->buffer.content[editor->cursor.position - 1] != '\n')
        editor->cursor.position++; 
    while (pos != 0 && editor->buffer.content[editor->cursor.position] != '\n') {
        editor->cursor.position++;
        pos--;
    }
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

void editor_key_home(Editor* restrict editor) {
    editor->cursor.position = 0;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

void editor_key_end(Editor* restrict editor) {
    while (editor->cursor.position < editor->buffer.length && editor->buffer.content[editor->cursor.position] != '\n')
        editor->cursor.position++;
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
    adjust_camera_to_cursor(editor);
}

void editor_mouse_wheel(Editor* restrict editor, int32_t delta) {
    delta *= editor->scroll_speed;
    editor->camera_y += delta;
    if (editor->camera_y > 0)
        editor->camera_y = 0;
    adjust_ortographic(editor);
    settings_ico_calculate(editor);
    file_ico_calculate(editor);
}

static void text_left_click(Editor* editor, float mouse_x, float mouse_y) {
    float y_start = editor->text_y + editor->nl_height;
    float y_end   = editor->text_y;
    uint64_t index_y = 0;
    y_start -= editor->nl_height;
    y_end   -= editor->nl_height;
    for (uint64_t i = 0; i < editor->buffer.length; i++) {
        if (editor->buffer.content[i] == '\n') {
            if (mouse_y <= y_start && mouse_y >= y_end)
                index_y = i + 1;
            y_start -= editor->nl_height;
            y_end   -= editor->nl_height;
        }
    }
    
    float x_start = editor->text_x;
    float x_end   = x_start;
    editor->cursor.position = index_y;
    if (editor->buffer.content[index_y] == '\n')
        goto left_click_end;
    if (mouse_x < x_start)
        goto left_click_end;
    for (int i = index_y; index_y < editor->buffer.length && editor->buffer.content[i] != '\n'; i++) {
        Character character = editor->character_map.character[(int32_t)editor->buffer.content[i]];
        x_start = x_end;
        x_end += character.advance;
        index_y = i;
        editor->cursor.position = i;
        if (mouse_x >= x_start && mouse_x <= x_end) {
            goto left_click_end;
        }
        editor->cursor.position++;
    }
    left_click_end:
    cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
}

void editor_left_click(Editor* restrict editor, float mouse_x, float mouse_y) {
    mouse_y = editor->height - mouse_y;
    if (mouse_x >= editor->settings_ico.xpos && mouse_x <= editor->settings_ico.xpos + editor->settings_ico.size && mouse_y <= editor->settings_ico.ypos && mouse_y >= editor->settings_ico.ypos - editor->settings_ico.size)
        editor->settings.display = !editor->settings.display;
    else if (mouse_x >= editor->file_ico.xpos && mouse_x <= editor->file_ico.xpos + editor->file_ico.size && mouse_y <= editor->file_ico.ypos && mouse_y >= editor->file_ico.ypos - editor->settings_ico.size) {
        open_file(&editor->buffer);
        editor->cursor.position = 0;
        cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->text_x, editor->text_y, editor->nl_height);
    }
    else if (editor->settings.display)
        settings_left_click(editor, mouse_x, mouse_y);
    else
        text_left_click(editor, mouse_x, mouse_y);
}