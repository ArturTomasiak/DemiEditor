#include "editor.h"

static inline void uibox_render(Shader* shader, float* model) {
    shader_set_uniformmat4f(shader, "model", model, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void settings_render(Editor* restrict editor) {
    ico_objects_bind(editor);
    texture_bind(0, &editor->settings.close_ico.texture);

    shader_set_uniformmat4f(&editor->ico_objects.shader, "model", editor->settings.close_ico.model, 1);
    shader_set_uniform1i(&editor->ico_objects.shader, "texture_input", 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    shader_bind(&editor->settings.uibox.shader);
    vao_bind(&editor->settings.uibox.vao);
    vbo_bind(&editor->settings.uibox.vbo);

    uibox_render(&editor->settings.uibox.shader, editor->settings.uibox.font_model);
    uibox_render(&editor->settings.uibox.shader, editor->settings.uibox.line_model);
    uibox_render(&editor->settings.uibox.shader, editor->settings.uibox.scroll_model);

    text_bind(editor);
    render_text(editor, &editor->settings.font_setting, editor->settings.xpos, editor->settings.font_ypos);
    render_text(editor, &editor->settings.font_input, editor->settings.font_input_xpos, editor->settings.font_ypos);
    render_text(editor, &editor->settings.line_setting, editor->settings.xpos, editor->settings.line_ypos);
    render_text(editor, &editor->settings.line_input, editor->settings.line_input_xpos, editor->settings.line_ypos);
    render_text(editor, &editor->settings.scroll_setting, editor->settings.xpos, editor->settings.scroll_ypos);
    render_text(editor, &editor->settings.scroll_input, editor->settings.scroll_input_xpos, editor->settings.scroll_ypos);

    cursor_update_blink(&editor->cursor);
    if (editor->settings.cursor_display)
        cursor_render(&editor->cursor, &editor->shader);
}

static inline void uibox_create(Editor* restrict editor);

void settings_create(Editor* restrict editor) {
    editor->settings.display  = 0;
    editor->settings.close_ico.size = 30.0f * editor->aspect_ratio;
    editor->settings.close_ico.texture = texture_create("..\\resources\\icons\\close_ico.png");
    settings_close_ico_calculate(editor);

    editor->settings_ico.size = editor->settings.close_ico.size;

    editor->settings_ico.texture = texture_create("..\\resources\\icons\\settings_ico.png");
    settings_ico_calculate(editor);

    uibox_create(editor);
}

void settings_delete(Settings* restrict settings) {
    texture_delete(&settings->close_ico.texture);

    buffer_delete(&settings->font_input);
    buffer_delete(&settings->font_setting);
    buffer_delete(&settings->line_input);
    buffer_delete(&settings->line_setting);
    buffer_delete(&settings->scroll_input);
    buffer_delete(&settings->scroll_setting);
}

void settings_create_buffers (Settings* restrict settings) {
    wchar_t* font_size = L"Font Size";
    wchar_t* line_spacing = L"Line Spacing";
    wchar_t* scroll_speed = L"Scroll Speed";
    wchar_t* font_size_default = L"20";
    wchar_t* line_spacing_default = L"15";  // will be divided by 10
    wchar_t* scroll_speed_default = L"30";  // will be divided by 100
 
    settings->font_setting   = buffer_create(font_size, wcslen(font_size), wcslen(font_size) + 1);
    settings->font_input     = buffer_create(font_size_default, wcslen(font_size_default), 4);
    settings->line_setting   = buffer_create(line_spacing, wcslen(line_spacing), wcslen(line_spacing) + 1);
    settings->line_input     = buffer_create(line_spacing_default, wcslen(line_spacing_default), 4);
    settings->scroll_setting = buffer_create(scroll_speed, wcslen(scroll_speed), wcslen(scroll_speed) + 1);
    settings->scroll_input   = buffer_create(scroll_speed_default, wcslen(scroll_speed_default), 4);
} 

static float text_width(Buffer* restrict buffer, CharacterMap* restrict character_map) {
    float width = 0;
    for (uint16_t i = 0; i < buffer->length; i++)
        width += character_map->character[(int32_t)buffer->content[i]].advance;
    return width;
}

void settings_buffer_width(Settings* restrict settings, CharacterMap* restrict character_map) {
    settings->font_setting_width   = text_width(&settings->font_setting, character_map);
    settings->line_setting_width   = text_width(&settings->line_setting, character_map);
    settings->scroll_setting_width = text_width(&settings->scroll_setting, character_map);

    settings->font_input_width   = text_width(&settings->font_input, character_map);
    settings->line_input_width   = text_width(&settings->line_input, character_map);
    settings->scroll_input_width = text_width(&settings->scroll_input, character_map);
}

void settings_ico_calculate(Editor* restrict editor) {
    editor->settings_ico.xpos = 5.0f * editor->aspect_ratio;
    editor->settings_ico.ypos = editor->height - 5.0f  * editor->aspect_ratio;
    float xpos = editor->settings_ico.xpos;
    float ypos = editor->settings_ico.ypos - editor->settings_ico.size + editor->camera_y;

    memset(editor->settings_ico.scale_matrix, 0.0f, 16 * sizeof(float));
    memset(editor->settings_ico.translate, 0.0f, 16 * sizeof(float));

    math_identity_f4x4(editor->settings_ico.scale_matrix, 1.0f);
    math_identity_f4x4(editor->settings_ico.translate, 1.0f);
    
    math_translate_f4x4(editor->settings_ico.translate, xpos, ypos, 0.0f);
    math_scale_f4x4(editor->settings_ico.scale_matrix, editor->settings_ico.size, editor->settings_ico.size, 1.0f);
    math_multiply_f4x4(editor->settings_ico.model, editor->settings_ico.translate, editor->settings_ico.scale_matrix);
}

void settings_close_ico_calculate(Editor* restrict editor) {
    editor->settings.close_ico.xpos = 5.0f * editor->aspect_ratio;
    editor->settings.close_ico.ypos = editor->height - 5.0f  * editor->aspect_ratio;
    float xpos = editor->settings.close_ico.xpos;
    float ypos = editor->settings.close_ico.ypos - editor->settings.close_ico.size + editor->camera_y;

    memset(editor->settings.close_ico.scale_matrix, 0.0f, 16 * sizeof(float));
    memset(editor->settings.close_ico.translate, 0.0f, 16 * sizeof(float));

    math_identity_f4x4(editor->settings.close_ico.scale_matrix, 1.0f);
    math_identity_f4x4(editor->settings.close_ico.translate, 1.0f);
    
    math_translate_f4x4(editor->settings.close_ico.translate, xpos, ypos, 0.0f);
    math_scale_f4x4(editor->settings.close_ico.scale_matrix, editor->settings.close_ico.size, editor->settings.close_ico.size, 1.0f);
    math_multiply_f4x4(editor->settings.close_ico.model, editor->settings.close_ico.translate, editor->settings.close_ico.scale_matrix);
}

static inline void uibox_create(Editor* restrict editor) {
    shader_bind(&editor->settings.uibox.shader);

    float vertices[12] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f,
    };

    editor->settings.uibox.vao = vao_create();
    vao_bind(&editor->settings.uibox.vao);

    editor->settings.uibox.vbo = vbo_create(vertices, sizeof(vertices));
    vbo_bind(&editor->settings.uibox.vbo);

    vertex_buffer_layout layout = vao_create_layout();
    vao_add_element(&layout, 2, GL_FLOAT, sizeof(float), GL_FALSE);
    vao_add_buffer(&editor->settings.uibox.vbo, &layout, &editor->settings.uibox.vao);
    vao_delete_layout(&layout);

    shader_set_uniform3f(&editor->settings.uibox.shader, "box_color", 0.08f, 0.08f, 0.08f);
    shader_set_uniform1f(&editor->settings.uibox.shader, "corner_brightness", 3.0f);
    shader_set_uniform1f(&editor->settings.uibox.shader, "corner_size", 0.1f);
}

static void uibox_model(UIBox* restrict uibox, float* model, float width, float height, float x, float y) {
    float translate[16] = {0}, scale[16] = {0};
    math_identity_f4x4(translate, 1.0f);
    math_identity_f4x4(scale, 1.0f);

    math_translate_f4x4(translate, x, y, 0.0f);
    math_scale_f4x4(scale, width, height, 1.0f);
    math_multiply_f4x4(model, translate, scale);
}

void uibox_calculate(Editor* restrict editor) {
    editor->settings.input_spacing     = editor->character_map.character[(int32_t)L' '].advance << 3;
    editor->settings.y_spacing         = editor->nl_height * 2;

    editor->settings.xpos = editor->text_x;
    editor->settings.font_input_xpos = editor->settings.xpos + editor->settings.font_setting_width + editor->settings.input_spacing;
    editor->settings.line_input_xpos = editor->settings.xpos + editor->settings.line_setting_width + editor->settings.input_spacing;
    editor->settings.scroll_input_xpos = editor->settings.xpos + editor->settings.scroll_setting_width + editor->settings.input_spacing;
    editor->settings.font_ypos = editor->text_y - editor->font_pixels/2;
    editor->settings.line_ypos = editor->settings.font_ypos - editor->settings.y_spacing;
    editor->settings.scroll_ypos = editor->settings.line_ypos - editor->settings.y_spacing;

    float width = editor->settings.font_input_width * 1.5;
    float height = editor->character_map.character[L'1'].size.y;

    float font_x = editor->settings.xpos + editor->settings.font_setting_width + editor->settings.input_spacing + editor->settings.font_input_width / 2.0f;
    float font_y = editor->settings.font_ypos + height/2;
    uibox_model(&editor->settings.uibox, editor->settings.uibox.font_model, width, height, font_x, font_y);
    float line_x = editor->settings.xpos + editor->settings.line_setting_width + editor->settings.input_spacing + editor->settings.line_input_width / 2.0f;
    float line_y = editor->settings.line_ypos + height/2;
    uibox_model(&editor->settings.uibox, editor->settings.uibox.line_model, width, height, line_x, line_y);
    float scroll_x = editor->settings.xpos + editor->settings.scroll_setting_width + editor->settings.input_spacing + editor->settings.scroll_input_width / 2.0f;
    float scroll_y = editor->settings.scroll_ypos + height/2;
    uibox_model(&editor->settings.uibox, editor->settings.uibox.scroll_model, width, height, scroll_x, scroll_y);

    // above calculations cannot be re-used in settings_left_click

    editor->settings.uibox_width  = width * 2;
    editor->settings.uibox_height = height * 2;
    editor->settings.uibox_pos.font_x = editor->settings.xpos + editor->settings.font_setting_width + editor->settings.input_spacing - editor->settings.font_input_width;
    editor->settings.uibox_pos.font_y = font_y + height;
    editor->settings.uibox_pos.line_x = editor->settings.xpos + editor->settings.line_setting_width + editor->settings.input_spacing - editor->settings.line_input_width;
    editor->settings.uibox_pos.line_y = line_y + height;
    editor->settings.uibox_pos.scroll_x = editor->settings.xpos + editor->settings.scroll_setting_width + editor->settings.input_spacing - editor->settings.scroll_input_width;
    editor->settings.uibox_pos.scroll_y = scroll_y + height;
}

extern inline void settings_left_click(Editor* restrict editor, float mouse_x, float mouse_y) {
    if (mouse_x >= editor->settings.close_ico.xpos + editor->camera_x
        && mouse_x <= editor->settings.close_ico.xpos + editor->settings.close_ico.size + editor->camera_x 
        && mouse_y <= editor->settings.close_ico.ypos + editor->camera_y 
        && mouse_y >= editor->settings.close_ico.ypos - editor->settings.close_ico.size + editor->camera_y) {
            cursor_update_position(&editor->cursor, &editor->buffer, &editor->character_map, editor->cursor_pos, editor->text_x, editor->text_y, editor->nl_height);
            editor->settings.display = 0;
            editor->camera_x = editor->camera_x_save;
            editor->camera_y = editor->camera_y_save;
            adjust_ortographic(editor);
    }
    else if (mouse_x >= editor->settings.uibox_pos.font_x
        && mouse_x <= editor->settings.uibox_pos.font_x + editor->settings.uibox_width
        && mouse_y <= editor->settings.uibox_pos.font_y
        && mouse_y >= editor->settings.uibox_pos.font_y - editor->settings.uibox_height) {
            editor->settings.cursor_display = 1;
            editor->settings.cursor_pos     = editor->settings.font_input.length;
            editor->settings.selected       = font;
            cursor_update_position(&editor->cursor, &editor->settings.font_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.font_input_xpos, editor->settings.font_ypos, editor->nl_height);
    }
    else if (mouse_x >= editor->settings.uibox_pos.line_x
        && mouse_x <= editor->settings.uibox_pos.line_x + editor->settings.uibox_width
        && mouse_y <= editor->settings.uibox_pos.line_y
        && mouse_y >= editor->settings.uibox_pos.line_y - editor->settings.uibox_height) {
            editor->settings.cursor_display = 1;
            editor->settings.cursor_pos     = editor->settings.line_input.length;
            editor->settings.selected       = line;
            cursor_update_position(&editor->cursor, &editor->settings.line_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.line_input_xpos, editor->settings.line_ypos, editor->nl_height);
    }
    else if (mouse_x >= editor->settings.uibox_pos.scroll_x
        && mouse_x <= editor->settings.uibox_pos.scroll_x + editor->settings.uibox_width
        && mouse_y <= editor->settings.uibox_pos.scroll_y
        && mouse_y >= editor->settings.uibox_pos.scroll_y - editor->settings.uibox_height) {
            editor->settings.cursor_display = 1;
            editor->settings.cursor_pos     = editor->settings.scroll_input.length;
            editor->settings.selected       = scroll;
            cursor_update_position(&editor->cursor, &editor->settings.scroll_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.scroll_input_xpos, editor->settings.scroll_ypos, editor->nl_height);
    }
}

extern inline void settings_backspace(Editor* restrict editor) {
    if (!editor->settings.cursor_display || editor->settings.cursor_pos == 0)
        return;
    switch(editor->settings.selected) {
        case font:
            if (editor->settings.font_input.length != 0) {
                buffer_delete_char(&editor->settings.font_input, editor->settings.cursor_pos);
                editor->settings.cursor_pos--;
                cursor_update_position(&editor->cursor, &editor->settings.font_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.font_input_xpos, editor->settings.font_ypos, editor->nl_height);
            }
        break;
        case line:
            if (editor->settings.line_input.length != 0) {
                buffer_delete_char(&editor->settings.line_input, editor->settings.cursor_pos);
                editor->settings.cursor_pos--;
                cursor_update_position(&editor->cursor, &editor->settings.line_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.line_input_xpos, editor->settings.line_ypos, editor->nl_height);
            }
        break;
        case scroll:
            if (editor->settings.scroll_input.length != 0) {
                buffer_delete_char(&editor->settings.scroll_input, editor->settings.cursor_pos);
                editor->settings.cursor_pos--;
                cursor_update_position(&editor->cursor, &editor->settings.scroll_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.scroll_input_xpos, editor->settings.scroll_ypos, editor->nl_height);
            }
        break;
        default:
        break;
    }
}

extern inline void settings_input(Editor* restrict editor, wchar_t ch) {
    if (!(ch >= L'0' && ch <= L'9'))
        return;
    switch(editor->settings.selected) {
        case font:
            if (editor->settings.font_input.length == 2)
                return;
            buffer_insert_char(&editor->settings.font_input, ch, editor->settings.cursor_pos);
            if (editor->settings.cursor_pos > editor->settings.font_input.length)
                editor->settings.cursor_pos = editor->settings.font_input.length;
            else
                editor->settings.cursor_pos++;
            cursor_update_position(&editor->cursor, &editor->settings.font_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.font_input_xpos, editor->settings.font_ypos, editor->nl_height);
        break;
        case line:
            if (editor->settings.line_input.length == 2)
                    return;
            buffer_insert_char(&editor->settings.line_input, ch, editor->settings.cursor_pos);
            if (editor->settings.cursor_pos > editor->settings.line_input.length)
                editor->settings.cursor_pos = editor->settings.line_input.length;
            else
                editor->settings.cursor_pos++;
            cursor_update_position(&editor->cursor, &editor->settings.line_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.line_input_xpos, editor->settings.line_ypos, editor->nl_height);
        break;
        case scroll:
            if (editor->settings.scroll_input.length == 3)
                    return;
            buffer_insert_char(&editor->settings.scroll_input, ch, editor->settings.cursor_pos);
            if (editor->settings.cursor_pos > editor->settings.scroll_input.length)
                editor->settings.cursor_pos = editor->settings.scroll_input.length;
            else
                editor->settings.cursor_pos++;
            cursor_update_position(&editor->cursor, &editor->settings.scroll_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.scroll_input_xpos, editor->settings.scroll_ypos, editor->nl_height);
        break;
        default:
        break;
    }
}

extern inline void settings_key_left(Editor* restrict editor) {
    if (editor->settings.cursor_pos == 0)
        return;
    editor->settings.cursor_pos--;
    switch(editor->settings.selected) {
        case font:
            cursor_update_position(&editor->cursor, &editor->settings.font_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.font_input_xpos, editor->settings.font_ypos, editor->nl_height);
        break;
        case line:
            cursor_update_position(&editor->cursor, &editor->settings.line_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.line_input_xpos, editor->settings.line_ypos, editor->nl_height);
        break;
        case scroll:
            cursor_update_position(&editor->cursor, &editor->settings.scroll_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.scroll_input_xpos, editor->settings.scroll_ypos, editor->nl_height);
        break;
        default:
        break;
    }
}

extern inline void settings_key_right(Editor* restrict editor) {
    switch(editor->settings.selected) {
        case font:
            if (editor->settings.cursor_pos >= editor->settings.font_input.length)
                return;
            editor->settings.cursor_pos++;
            cursor_update_position(&editor->cursor, &editor->settings.font_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.font_input_xpos, editor->settings.font_ypos, editor->nl_height);
        break;
        case line:
            if (editor->settings.cursor_pos >= editor->settings.line_input.length)
                return;
            editor->settings.cursor_pos++;
            cursor_update_position(&editor->cursor, &editor->settings.line_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.line_input_xpos, editor->settings.line_ypos, editor->nl_height);
        break;
        case scroll:
            if (editor->settings.cursor_pos >= editor->settings.scroll_input.length)
                return;
            editor->settings.cursor_pos++;
            cursor_update_position(&editor->cursor, &editor->settings.scroll_input, &editor->character_map, editor->settings.cursor_pos, editor->settings.scroll_input_xpos, editor->settings.scroll_ypos, editor->nl_height);
        break;
        default:
        break;
    }
}

uint16_t wstr_to_int(wchar_t* str) {
    uint8_t i = 0;
    uint16_t res = 0;
    while (str[i]) {
        if (i != 0)
            res *= 10;
        res += str[i] - L'0';
        i++;
    }
    return res;
}

extern inline void settings_enter(Editor* restrict editor) {
    switch(editor->settings.selected) {
        case font: 
        {
            uint16_t input = wstr_to_int(editor->settings.font_input.content);
            if (input < 10) {
                input = 10;
                buffer_replace_content(&editor->settings.font_input, L"10", 2);
            }
            else if (input > 50) {
                input = 50;
                buffer_replace_content(&editor->settings.font_input, L"50", 2);
            }
            editor->settings.editor_font_size = input;
            build_font(editor, input);
        }
        break;
        case line:
        {
            float input = wstr_to_int(editor->settings.line_input.content);
            input /= 10;
            if (input < 1.2f) {
                input = 1.2f;
                buffer_replace_content(&editor->settings.line_input, L"12", 2);
            }
            editor->line_spacing_setting = input;
            update_font(editor);
        }
        break;
        case scroll:
        {
            float input = wstr_to_int(editor->settings.scroll_input.content);
            input /= 100;
            if (input < 0.1f) {
                input = 0.1f;
                buffer_replace_content(&editor->settings.scroll_input, L"10", 2);
            }
            else if (input > 9.0f) {
                input = 9.0f;
                buffer_replace_content(&editor->settings.scroll_input, L"900", 3);
            }
            editor->scroll_speed = input;
        }
        break;
        default:
        break;
    }
    editor->settings.cursor_display = 0;
}