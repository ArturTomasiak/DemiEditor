#include "includes.h"
#include "character/character.h"
#include "character/buffer.h"
#include "shader/shader.h"
#include "cursor/cursor.h"
#include "objects/vao.h"
#include "objects/vbo.h"
#include "math/math.h"

static void end_gracefully();
static void render_text_call(uint32_t length);
static void render_text();
static void build_font();
static void build_cursor();
static void adjust_window_size();
static _Bool alloc_variables();
static void enable_vsync();
static void create_objects();
static _Bool create_window();
static HGLRC create_context();
static HGLRC create_temp_context();

static FT_Library ft_lib;
static FT_Face ft_face;
static HINSTANCE hinstance;
static WNDCLASSEX wc = {0};
static HGLRC hglrc;
static HWND hwnd;
static HDC hdc;
static const char* application_name = "DemiEditor";
static const char* wc_class_name = "DemiEditor";

static uint16_t font_pixels = 20;
static float line_spacing = 1.5f;
static float width = 960.0f;
static float height = 540.0f;
static float aspect_ratio = 16.0f / 9.0f; // target aspect ratio for consistency

static _Bool resized = 0;
CharacterMap character_map;
static Shader shader;
static vertex_array_object vao;
static vertex_buffer_object vbo;
static int32_t arr_limit;
static uint32_t texture_array;
static int32_t* letter_map; 
static float* transforms;
static float translate[16] = {0}, scale_matrix[16] = {0}, projection[16] = {0};

static float text_x;
static float text_y;

static Buffer buffer;
static Cursor cursor;

int32_t CALLBACK WinMain(
    HINSTANCE hinstance,
    HINSTANCE h_prev_instance,
    LPSTR lp_cmd_line,
    int32_t n_cmd_show 
) {
    SetProcessDPIAware();
    if (!create_window())
        return -1;
    hdc = GetDC(hwnd);
    HGLRC temp_context = create_temp_context();

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        wglDeleteContext(temp_context);
        win32_err(err_glew_initialization);
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return -1;
    }

    hglrc = create_context();
    wglDeleteContext(temp_context);

    enable_vsync();
    
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &arr_limit);
    arr_limit = arr_limit >> 5;  
    if (arr_limit < 2)
        arr_limit = 2;
    // I would divide by 16 as all I need is one mat4 array,
    // however the result cannot be trusted and division by 32 is my duct tape fix;
    // if the program breaks for only certain gpu's or drivers it's 99% the arr_limit
    
    if (!alloc_variables())
        return -1;

    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader = shader_create("..\\resources\\shaders\\vertex.glsl", "..\\resources\\shaders\\fragment.glsl", arr_limit);
    
    shader_bind(&shader);
    adjust_window_size(); // sets projection matrix
    shader_set_uniform1i(&shader, "text", 0);

    buffer = buffer_create();
    cursor = cursor_create();

    if (FT_Init_FreeType(&ft_lib))
        end_gracefully();

    if (FT_New_Face(ft_lib, "..\\resources\\fonts\\arial.ttf", 0, &ft_face))
        end_gracefully();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &texture_array);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_array);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    build_font();
    build_cursor();

    for (uint32_t i = 0; i < arr_limit; i++)
        math_identity_f4x4(transforms + (i * 16), 1.0f);

    create_objects();

    MSG msg;
    _Bool running = 1;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    text_x = 20.0f * aspect_ratio;
    text_y = height - (20.0f * aspect_ratio) - character_map.character[1].bearing.y;

    #ifdef demidebug
    check_gl_errors();
    info(__LINE__, __FILE__, "program enters main loop");
    #endif

    while (running) {
        if (resized) {
            adjust_window_size();
            resized = 0;
        }

        glClearColor(0.12156862745f, 0.12156862745f, 0.11372549019f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        cursor_update_blink(&cursor);
        cursor_update_position(&cursor, &buffer, &character_map, text_x, text_y);
        cursor_render(&cursor, &shader);
        render_text();

        SwapBuffers(hdc);

        #ifdef demidebug
        check_gl_errors();
        #endif
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                running = 0;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    end_gracefully();
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CLOSE: 
            PostQuitMessage(0);
        break;
        case WM_SIZE:
            width = (float)LOWORD(lParam);
            height = (float)HIWORD(lParam);
            if (width == 0) width = 1;
            if (height == 0) height = 1;
            resized = 1;
        break;
        case WM_CHAR:
            switch(wParam) {
                case 8: // backspace
                    buffer_delete_char(&buffer, cursor.position);
                    cursor.position--;
                break;
                case 9: // tab
                    buffer_insert_string(&buffer, "    ", cursor.position);
                    cursor.position += 4;
                break;
                case 13: // enter
                    buffer_insert_char(&buffer, '\n', cursor.position);
                    cursor_to_nl(&cursor, &buffer);
                break;
                default:
                    buffer_insert_char(&buffer, wParam, cursor.position);
                    cursor.position++;
                break;
            }
        break;
        case WM_KEYDOWN:
            switch(wParam) {
                case VK_LEFT:
                    if (cursor.position > 0)
                        cursor.position--;
                break;
                case VK_RIGHT:
                    if (cursor.position < buffer.length) 
                        cursor.position++;
                break;
                case VK_UP: {
                    _Bool repeat = 1;
                    reach_nl:
                    while (cursor.position > 0 && buffer.content[cursor.position - 1] != '\n')
                        cursor.position--;
                    if (repeat == 1 && cursor.position > 0) {
                        cursor.position--;
                        repeat = 0;
                        goto reach_nl;
                    }
                }
                break;
                case VK_DOWN: {
                    cursor_to_nl(&cursor, &buffer);
                }
                break;
                case VK_HOME: // text start
                    cursor.position = 0;
                break;
                case VK_END:
                    while (cursor.position < buffer.length && buffer.content[cursor.position] != '\n')
                        cursor.position++;
                break;
            }
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void end_gracefully() {
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_lib);
    free(letter_map);
    free(transforms);
    free(character_map.character);
    shader_delete(&shader);
    buffer_delete(&buffer);
    glDeleteTextures(1, &texture_array);
    vao_delete(&vao);
    vbo_delete(&vbo);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
}

static void render_text_call(uint32_t length) {
    shader_set_uniform3f(&shader, "text_color", 1.0f, 1.0f, 1.0f);
    shader_set_uniformmat4f(&shader, "transforms", transforms, length);
    shader_set_uniform1iv(&shader, "letter_map", letter_map, length);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, length);
}

static void render_text() {
    float x = text_x;
    float y = text_y;
    float Xcopy = x;
    uint32_t working_index = 0;
    for (uint64_t i = 0; i < buffer.length; i++) {
        Character character = character_map.character[(uint8_t)buffer.content[i]];
        if (buffer.content[i] == '\n') {
            y -= (character.size.y) * line_spacing;
            x = Xcopy;
        }
        else if (buffer.content[i] == ' ')
            x += character.advance;
        else {
            memset(translate, 0.0f, 16 * sizeof(float));
            memset(scale_matrix, 0.0f, 16 * sizeof(float));
            math_identity_f4x4(scale_matrix, 1.0f);
            float xpos = x + character.bearing.x;
            float ypos = y - (font_pixels - character.bearing.y);
            letter_map[working_index] = character.ch;
            math_translate_f4x4(translate, xpos, ypos, 0);
            math_scale_f4x4(scale_matrix, font_pixels, font_pixels, 0);
            math_multiply_f4x4(transforms + (working_index * 16), translate, scale_matrix);
            x += character.advance;
            working_index++;
            if (working_index == arr_limit) {
                render_text_call(working_index);
                working_index = 0;
            }
        }
    }
    if (working_index != 0)
        render_text_call(working_index);
}

static void build_font() {
    FT_Set_Pixel_Sizes(ft_face, font_pixels, font_pixels);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, font_pixels, font_pixels, 128, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    for (uint8_t c = 0; c < 128; c++) {
        if (FT_Load_Char(ft_face, c, FT_LOAD_RENDER)) {
            warning(__LINE__, __FILE__, "failed to load glyph");
            continue;
        }
        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY,
            0, 0, 0, (int32_t)c,
            ft_face->glyph->bitmap.width,
            ft_face->glyph->bitmap.rows, 1,
            GL_RED,
            GL_UNSIGNED_BYTE,
            ft_face->glyph->bitmap.buffer
        );
        Character character = {
            (char)c,
            {ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows},
            {ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top},
            ft_face->glyph->advance.x >> 6 // divide by 64
        };
        character_map.character[c] = character;
    }
}

static void build_cursor() {
    cursor.blink_rate = 500;
    cursor.line_spacing = line_spacing;
    cursor.character = character_map.character['|'];
    cursor.x = text_x;
    cursor.y = text_y;
    cursor.font_pixels = font_pixels;
}

static void adjust_window_size() {
    math_orthographic_f4x4(projection, 0.0f, width, 0.0f, height, -1.0f, 1.0f);
    glViewport(0, 0, width, height);
    shader_set_uniformmat4f(&shader, "projection", projection, 1);
}

static _Bool alloc_variables() {
    letter_map = calloc(arr_limit, sizeof(int32_t)); 
    if (!letter_map) goto letter_map_failed;

    transforms = malloc(arr_limit * 16 * sizeof(float));
    if (!transforms) goto transforms_failed;

    character_map.character = malloc(128 * sizeof(Character));
    character_map.length = 128;
    if (!character_map.character) goto character_map_failed;

    return 1;

character_map_failed:
    free(transforms);
transforms_failed:
    free(letter_map);
letter_map_failed:
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    win32_err(err_memory_allocation);
    return 0;
}

static void enable_vsync() {
    if (!__wglewSwapIntervalEXT)
        __wglewSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (__wglewSwapIntervalEXT)
        __wglewSwapIntervalEXT(1);
    #ifdef demidebug
    else 
        warning(__LINE__, __FILE__, "vsync not supported");
    #endif
}

static void create_objects() {
    float vertex_data[8] = {
        0.0f,1.0f,
        0.0f,0.0f,
        1.0f,1.0f,
        1.0f,0.0f,
    };

    vao = vao_create();
    vbo = vbo_create(vertex_data, sizeof(vertex_data));
    
    vertex_buffer_layout layout = vao_create_layout();
    vao_add_element(&layout, 2, GL_FLOAT, sizeof(float), GL_FALSE);
    vao_add_buffer(&vbo, &layout, &vao);
    vao_delete_layout(&layout);
}

static _Bool create_window() {
    const char* application_icon = "..\\resources\\icons\\placeholder.ico";
    DWORD attributes = GetFileAttributesA(application_icon);
    if (attributes == INVALID_FILE_ATTRIBUTES || attributes & FILE_ATTRIBUTE_DIRECTORY) {
        win32_err(err_icon);
        return 0;
    }
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hinstance;
    wc.lpszClassName = wc_class_name;
    wc.hIcon = (HICON)LoadImage(hinstance, application_icon, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    wc.hIconSm = (HICON)LoadImage(hinstance, application_icon, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassEx(&wc);
    hwnd = CreateWindowEx(
        0, wc_class_name, application_name, 
        WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_SYSMENU, 
        200, 200, width, height, 0, 0, hinstance, 0 
    );
    return 1;
}

static HGLRC create_temp_context() {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, 
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,  
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    int32_t pixel_format = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixel_format, &pfd);
    HGLRC temp_context = wglCreateContext(hdc);
    wglMakeCurrent(hdc, temp_context);
    return temp_context;
}

static HGLRC create_context() {
    const int attribList[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0,
    };
    int pixel_format;
    UINT num_format;
    if (!wglChoosePixelFormatARB(hdc, attribList, NULL, 1, &pixel_format, &num_format))
        goto fail;
    const int32_t attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribs);
    if (!hglrc)
        goto fail;
    wglMakeCurrent(hdc, hglrc);
    return hglrc;

fail:
    win32_err(err_opengl_context);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    ExitProcess(0);
}