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

void settings_render(Editor* restrict editor) {

}

void settings_left_click(Editor* restrict editor, float mouse_x, float mouse_y) {

}