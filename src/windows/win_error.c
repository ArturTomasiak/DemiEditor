#include "../includes.h"
#ifdef demiwindows

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern inline void error(wchar_t* err, wchar_t* title) {
    MessageBox(NULL, err, title, MB_OK);
}

void fatal_error(enum err_type err) {
    wchar_t* error;
    switch (err) {
        case err_glew_initialization:
            error = L"could not initialize glew";
        break;
        case err_freetype_initialization:
            error = L"could not initialize freetype2";
        break;
        case err_shader_compilation:
            error = L"shader compilation failed";
        break;
        case err_memory_allocation:
            error = L"memory allocation failed";
        break;
        case err_opengl_context:
            error = L"could not create opengl context";
        break;
        case err_create_window:
            error = L"could not create window";
        break;
        case err_texture:
            error = L"texture creation failed";
        break;
        case err_file:
            error = L"missing files in resources";
        break;
        case err_icon:
            error = L"missing icon in resources";
        break;
        case err_font:
            error = L"missing font in resources";
        break;
        case err_png: 
            error = L"invalid png in resources";
        break;
        default:
            error = L"invalid error message";
        break;
    }
    MessageBox(NULL, error, L"Error", MB_ICONERROR | MB_OK);
    PostQuitMessage(-1);
}

#ifdef demidebug
static const DWORD red    = FOREGROUND_RED | FOREGROUND_INTENSITY;
static const DWORD green  = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
static const DWORD orange = FOREGROUND_RED | FOREGROUND_GREEN;
static const DWORD pink   = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
static const DWORD reset  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

void fatal(uint32_t line, char* file, char* message) {
    HANDLE console_output = GetStdHandle(STD_OUTPUT_HANDLE);
    printf("%s %d %s", "line:", line, file);
    SetConsoleTextAttribute(console_output, red);
    printf(" fatal ");
    SetConsoleTextAttribute(console_output, reset);
    printf("%s\n", message);
}

void warning(uint32_t line, char* file, char* message) {
    HANDLE console_output = GetStdHandle(STD_OUTPUT_HANDLE);
    printf("%s %d %s", "line:", line, file);
    SetConsoleTextAttribute(console_output, pink);
    printf(" warning ");
    SetConsoleTextAttribute(console_output, reset);
    printf("%s\n", message);
}

void info(uint32_t line, char* file, char* message) {
    HANDLE console_output = GetStdHandle(STD_OUTPUT_HANDLE);
    printf("%s %d %s", "line:", line, file);
    SetConsoleTextAttribute(console_output, green);
    printf(" info ");
    SetConsoleTextAttribute(console_output, reset);
    printf("%s\n", message);
}

static char* translate_gl_error(GLenum error) {
    switch (error) {
        case GL_INVALID_ENUM:
            return "invalid enumeration parameter";
        case GL_INVALID_VALUE:
            return "invalid value parameter";
        case GL_INVALID_OPERATION:
            return "invalid operation";
        case GL_STACK_OVERFLOW:
            return "stack overflow";
        case GL_STACK_UNDERFLOW:
            return "stack underflow";
        case GL_OUT_OF_MEMORY:
            return "memory allocation failed";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "writing to incomplete framebuffer";
        default:
            return "unknown code";
    }
}

void check_gl_errors() {
    GLenum error;
    HANDLE console_output = GetStdHandle(STD_OUTPUT_HANDLE);
    while ((error = glGetError())) {
        SetConsoleTextAttribute(console_output, orange);
        printf("[glGetError] ");
        SetConsoleTextAttribute(console_output, reset);
        printf("%s", translate_gl_error(error));
        printf("%s%d%s\n", " (", error, ")");
    }
}

#endif

#endif