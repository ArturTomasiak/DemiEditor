#include "editor.h"

void settings_ico_render(Editor* restrict editor) {
    shader_set_uniformmat4f(&editor->ico_objects.shader, "projection", editor->projection, 1);
    shader_set_uniformmat4f(&editor->ico_objects.shader, "model", editor->settings_ico.model, 1);
    shader_set_uniform1i(&editor->ico_objects.shader, "icon_texture", 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
    
    math_translate_f4x4(editor->settings_ico.translate, xpos, ypos, 0);
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
    
    math_translate_f4x4(editor->settings.close_ico.translate, xpos, ypos, 0);
    math_scale_f4x4(editor->settings.close_ico.scale_matrix, editor->settings.close_ico.size, editor->settings.close_ico.size, 1.0f);
    math_multiply_f4x4(editor->settings.close_ico.model, editor->settings.close_ico.translate, editor->settings.close_ico.scale_matrix);
}

void settings_create(Editor* restrict editor) {
    const float quad_vertices[16] = {
        0.0f, 1.0f,   0.0f, 1.0f,
        0.0f, 0.0f,   0.0f, 0.0f,
        1.0f, 1.0f,   1.0f, 1.0f,
        1.0f, 0.0f,   1.0f, 0.0f
    };

    editor->settings.display  = 0;
    editor->settings.close_ico.size = 30.0f * editor->aspect_ratio;
    editor->settings.close_ico.texture = texture_create("..\\resources\\icons\\close_ico.png");
    settings_close_ico_calculate(editor);

    editor->settings_ico.size = editor->settings.close_ico.size;
    editor->ico_objects.shader = shader_create("..\\resources\\shaders\\ico_vertex.glsl", "..\\resources\\shaders\\ico_fragment.glsl", 1);
    
    shader_bind(&editor->ico_objects.shader);

    editor->ico_objects.vao = vao_create();
    editor->ico_objects.vbo = vbo_create(quad_vertices, 4 * 4 * sizeof(quad_vertices));
    
    vertex_buffer_layout layout = vao_create_layout();
    vao_add_element(&layout, 2, GL_FLOAT, sizeof(float), GL_FALSE);
    vao_add_element(&layout, 2, GL_FLOAT, sizeof(float), GL_FALSE);
    vao_add_buffer(&editor->ico_objects.vbo, &layout, &editor->ico_objects.vao);
    vao_delete_layout(&layout);
    
    editor->settings_ico.texture = texture_create("..\\resources\\icons\\settings_ico.png");
    settings_ico_calculate(editor);

    wchar_t* font_size = L"Font Size";
    wchar_t* line_spacing = L"Line Spacing";
    wchar_t* scroll_speed = L"Scroll Speed";
    wchar_t* font_size_default = L"20";
    wchar_t* line_spacing_default = L"1.5";
    wchar_t* scroll_speed_default = L"30";
    editor->settings.font_pixels_setting  = buffer_create(font_size, wcslen(font_size), wcslen(font_size) + 1);
    editor->settings.font_pixels_input    = buffer_create(font_size_default, wcslen(font_size_default), 4);
    editor->settings.line_spacing_setting = buffer_create(line_spacing, wcslen(line_spacing), wcslen(line_spacing) + 1);
    editor->settings.line_spacing_input   = buffer_create(line_spacing_default, wcslen(line_spacing_default), 5);
    editor->settings.scroll_speed_setting = buffer_create(scroll_speed, wcslen(scroll_speed), wcslen(scroll_speed) + 1);
    editor->settings.scroll_speed_setting = buffer_create(scroll_speed_default, wcslen(scroll_speed_default), 4);

    editor->settings.settings_font_size = 40;
    editor->settings.xpos = 40;
    editor->settings.ypos = editor->height - 40;
}

void settings_delete(Settings* restrict settings) {
    texture_delete(&settings->close_ico.texture);

    buffer_delete(&settings->font_pixels_input);
    buffer_delete(&settings->font_pixels_setting);
    buffer_delete(&settings->line_spacing_input);
    buffer_delete(&settings->line_spacing_setting);
    buffer_delete(&settings->scroll_speed_input);
    buffer_delete(&settings->scroll_speed_setting);
}

void settings_render(Editor* restrict editor) {
    ico_objects_bind(editor);
    texture_bind(0, &editor->settings.close_ico.texture);

    shader_set_uniformmat4f(&editor->ico_objects.shader, "projection", editor->projection, 1);
    shader_set_uniformmat4f(&editor->ico_objects.shader, "model", editor->settings.close_ico.model, 1);
    shader_set_uniform1i(&editor->ico_objects.shader, "icon_texture", 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    text_bind(editor);
    render_text(editor, &editor->settings.font_pixels_setting, 20, 20);
}

void settings_left_click(Editor* restrict editor, float mouse_x, float mouse_y) {
    if (mouse_x >= editor->settings.close_ico.xpos + editor->camera_x
        && mouse_x <= editor->settings.close_ico.xpos + editor->settings.close_ico.size + editor->camera_x 
        && mouse_y <= editor->settings.close_ico.ypos + editor->camera_y 
        && mouse_y >= editor->settings.close_ico.ypos - editor->settings.close_ico.size + editor->camera_y)
        editor->settings.display = !editor->settings.display;
}