#include "includes.h"
#ifdef demiwindows

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "editor.h"

static void end_gracefully();
static void enable_vsync();
static _Bool create_window();
static HGLRC create_temp_context();
static HGLRC create_context();

static HINSTANCE hinstance;
static WNDCLASSEX wc = {0};
static HGLRC hglrc;
static HWND hwnd;
static HDC hdc;
static const char* application_name = "DemiEditor";
static const char* wc_class_name = "DemiEditor";
static float width = 960.0f;
static float height = 540.0f;
static _Bool resized = 0;

#ifdef demidebug
int32_t main() {
    hinstance = GetModuleHandle(NULL);
#else
int32_t CALLBACK WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int32_t nCmdShow 
) {

    hinstance = hInstance;
#endif    
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (!create_window())
        return -1;
    hdc = GetDC(hwnd);
    HGLRC temp_context = create_temp_context();

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        wglDeleteContext(temp_context);
        error(err_glew_initialization);
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return -1;
    }

    hglrc = create_context();
    wglDeleteContext(temp_context);

    enable_vsync();
    
    if (!alloc_variables()) {
        wglDeleteContext(hglrc);
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return -1;
    }
    editor_dpi(GetDpiForWindow(hwnd));
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    uint32_t width = clientRect.right - clientRect.left;
    uint32_t height = clientRect.bottom - clientRect.top;
    if (!editor_init(width, height)) {
        end_gracefully();
    }

    MSG msg;
    _Bool running = 1;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    #ifdef demidebug
    check_gl_errors();
    info(__LINE__, __FILE__, "program enters main loop");
    #endif

    while (running) {
        if (resized) {
            GetClientRect(hwnd, &clientRect);
            width = clientRect.right - clientRect.left;
            height = clientRect.bottom - clientRect.top;
            editor_window_size(width, height);
            resized = 0;
        }

        glClearColor(0.12156862745f, 0.12156862745f, 0.11372549019f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        editor_loop();

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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
    switch(msg) {
        case WM_CLOSE: 
            PostQuitMessage(0);
        break;
        case WM_SIZE:
            resized = 1;
        break;
        case WM_DPICHANGED:
            editor_dpi(LOWORD(w_param));
        break;
        case WM_CHAR:
            switch(w_param) {
                case 8:
                    editor_backspace();
                break;
                case 9:
                    editor_tab();
                break;
                case 13:
                    editor_enter();
                break;
                default:
                    editor_input(w_param);
                break;
            }
        break;
        case WM_KEYDOWN:
            switch(w_param) {
                case VK_LEFT:
                    editor_key_left();
                break;
                case VK_RIGHT:
                    editor_key_right();
                break;
                case VK_UP:
                    editor_key_up();
                break;
                case VK_DOWN:
                    editor_key_down();
                break;
                case VK_HOME:
                    editor_key_home();
                break;
                case VK_END:
                    editor_key_end();
                break;
            }
        break;
    }
    return DefWindowProc(hwnd, msg, w_param, l_param);
}

static void end_gracefully() {
    editor_end_gracefully();
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
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

static _Bool create_window() {
    const char* application_icon = "..\\resources\\icons\\placeholder.ico";
    DWORD attributes = GetFileAttributesA(application_icon);
    if (attributes == INVALID_FILE_ATTRIBUTES || attributes & FILE_ATTRIBUTE_DIRECTORY) {
        error(err_icon);
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
    error(err_opengl_context);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    ExitProcess(0);
}

#endif