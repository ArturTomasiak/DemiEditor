#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define demiwindows
#define UNICODE
#define _UNICODE
#endif

// will add more macros for compatibility with time
#if defined(_MSC_VER) && !defined(__clang__)
  #define restrict __restrict
  #define extern
#endif

#ifdef NDEBUG
#define assert(ignore) ((void)0)
#endif

#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>

enum err_type {
    err_glew_initialization,
    err_freetype_initialization,
    err_shader_compilation,
    err_memory_allocation,
    err_opengl_context,
    err_create_window,
    err_texture,
    err_file,
    err_icon,
    err_font,
    err_png
};

// implemented in error
void error(wchar_t* err, wchar_t* title);
void fatal_error(enum err_type err);

#ifdef demidebug
void fatal(uint32_t line, char* file, char* message);
void warning(uint32_t line, char* file, char* message);
void info(uint32_t line, char* file, char* message);
// static char* translate_gl_error(GLenum error);
void check_gl_errors();
#endif