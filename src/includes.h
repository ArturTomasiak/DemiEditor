#pragma once

#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#if defined(min)
#undef min
#endif
#if defined(max)
#undef max
#endif

#include <stdint.h>

enum err_type {
    err_glew_initialization,
    err_freetype_initialization,
    err_shader_compilation,
    err_memory_allocation,
    err_opengl_context,
    err_file,
    err_icon,
    err_font
};
void win32_err(enum err_type err);

#define demidebug

#ifdef demidebug
void fatal(uint32_t line, char* file, char* message);
void warning(uint32_t line, char* file, char* message);
void info(uint32_t line, char* file, char* message);
// static char* translate_gl_error(GLenum error);
void check_gl_errors();
#endif