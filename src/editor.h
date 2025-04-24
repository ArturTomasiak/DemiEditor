#include "includes.h"

// static void render_text_call(uint32_t length);
// static void render_text();

void editor_loop();
void editor_window_size(int32_t width, int32_t height);
_Bool editor_init(int32_t width, int32_t height);
void editor_end_gracefully();
void editor_dpi(int32_t dpi);
_Bool alloc_variables();

// static void build_font();
// static void build_cursor();
// static void create_objects();

void editor_backspace();
void editor_tab();
void editor_enter();
void editor_input(char ch);

void editor_key_left();
void editor_key_right();
void editor_key_up();
void editor_key_down();
void editor_key_home();
void editor_key_end();