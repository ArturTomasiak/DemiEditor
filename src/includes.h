#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define demidebug // comment out for release

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define demiwindows
#endif

#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

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

// implemented in error
void error(enum err_type err);

#ifdef demidebug
void fatal(uint32_t line, char* file, char* message);
void warning(uint32_t line, char* file, char* message);
void info(uint32_t line, char* file, char* message);
// static char* translate_gl_error(GLenum error);
void check_gl_errors();
#endif