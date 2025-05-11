#include "../includes.h"
#ifdef demiwindows

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../editor.h"

static void end_gracefully(Editor* restrict editor, HGLRC hglrc, HWND hwnd, HDC hdc);
static _Bool create_window(WNDCLASSEX* restrict wc, HINSTANCE hinstance, HWND* restrict hwnd);
static HGLRC create_temp_context(HDC hdc, HWND hwnd);
static HGLRC create_context(HDC hdc, HWND hwnd);

static const wchar_t* s_application_name = L"DemiEditor";
static _Bool s_resized  = 0;
// because WndProc can't directly recieve arguments
static Editor* get_editor(Editor* restrict editor) {
    static Editor* func_editor;
    if (editor) {
        func_editor = editor;
    }
    return func_editor;   
}

#ifdef demidebug
int32_t main() {
    HINSTANCE hinstance = GetModuleHandle(NULL);
#else
int32_t CALLBACK WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int32_t nCmdShow 
) {

    HINSTANCE hinstance = hInstance;
#endif    
    WNDCLASSEX wc = {0};
    HGLRC hglrc;
    HWND hwnd;
    HDC hdc;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (!create_window(&wc, hinstance, &hwnd))
        return -1;
    hdc = GetDC(hwnd);
    HGLRC temp_context = create_temp_context(hdc, hwnd);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        wglDeleteContext(temp_context);
        fatal_error(err_glew_initialization);
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return -1;
    }

    hglrc = create_context(hdc, hwnd);
    wglDeleteContext(temp_context);

    if (WGLEW_EXT_swap_control)
        wglSwapIntervalEXT(1);
    #ifdef demidebug
    else 
        warning(__LINE__, __FILE__, "vsync not supported");
    #endif
    
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int32_t width = clientRect.right - clientRect.left;
    int32_t height = clientRect.bottom - clientRect.top;
    Editor editor = editor_create((float)width, (float)height, GetDpiForWindow(hwnd));
    get_editor(&editor);

    MSG msg;
    _Bool running = 1;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    #ifdef demidebug
    check_gl_errors();
    info(__LINE__, __FILE__, "program enters main loop");
    #endif

    while (running) {
        if (s_resized) {
            GetClientRect(hwnd, &clientRect);
            width = clientRect.right - clientRect.left;
            height = clientRect.bottom - clientRect.top;
            editor_window_size(&editor, (float)width, (float)height);
            s_resized = 0;
        }

        glClearColor(0.12156862745f, 0.12156862745f, 0.11372549019f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        editor_loop(&editor);

        SwapBuffers(hdc);

        #ifdef demidebug
        check_gl_errors();
        #endif

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                running = 0;
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    
    end_gracefully(&editor, hglrc, hwnd, hdc);
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
    Editor* restrict editor = get_editor(NULL);
    switch(msg) {
        case WM_CLOSE: 
            PostQuitMessage(0);
        break;
        case WM_SIZE:
            s_resized = 1;
        break;
        case WM_DPICHANGED:
            editor_dpi(editor, LOWORD(w_param));
        break;
        case WM_MOUSEWHEEL:
            editor_mouse_wheel(editor, (short)HIWORD(w_param));
        break;
        case WM_LBUTTONUP:
            editor_left_click(editor, (float)(short)LOWORD(l_param), (float)(short)HIWORD(l_param)); 
        break;
        case WM_CHAR:
            switch(w_param) {
                case 8:
                    editor_backspace(editor);
                break;
                case 9:
                    editor_tab(editor);
                break;
                case 13:
                    editor_enter(editor);
                break;
                default:
                    if (iswprint(w_param) && !iswcntrl(w_param))
                        editor_input(editor, w_param);
                break;
            }
        break;
        case WM_KEYDOWN:
            switch(w_param) {
                case 'V':
                    if (IsClipboardFormatAvailable(CF_UNICODETEXT))
                        if (GetKeyState(VK_CONTROL) & 0x8000)
                            if (OpenClipboard(hwnd)) {
                                HANDLE clipboard = GetClipboardData(CF_UNICODETEXT);
                                if (clipboard) {
                                    wchar_t* text = GlobalLock(clipboard);
                                    if (text) {
                                        editor_paste(editor, text);
                                        GlobalUnlock(clipboard);
                                    }
                                }
                                CloseClipboard();
                            }
                break;
                case 'S':
                    if (GetKeyState(VK_CONTROL) & 0x8000)
                        editor_save(editor);
                break;
                case VK_LEFT:
                    editor_key_left(editor);
                break;
                case VK_RIGHT:
                    editor_key_right(editor);
                break;
                case VK_UP:
                    editor_key_up(editor);
                break;
                case VK_DOWN:
                    editor_key_down(editor);
                break;
                case VK_HOME:
                    editor_key_home(editor);
                break;
                case VK_END:
                    editor_key_end(editor);
                break;
            }
        break;
    }
    return DefWindowProc(hwnd, msg, w_param, l_param);
}

static void end_gracefully(Editor* restrict editor, HGLRC hglrc, HWND hwnd, HDC hdc) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    editor_delete(editor);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
}

static _Bool create_window(WNDCLASSEX* restrict wc, HINSTANCE hinstance, HWND* restrict hwnd) {
    int32_t width  = 960;
    int32_t height = 540;
    const wchar_t* application_icon = L"..\\resources\\icons\\icon.ico";
    DWORD attributes = GetFileAttributes(application_icon);
    if (attributes == INVALID_FILE_ATTRIBUTES || attributes & FILE_ATTRIBUTE_DIRECTORY) {
        fatal_error(err_icon);
        return 0;
    }
    wc->cbSize = sizeof(*wc);
    wc->style = CS_OWNDC;
    wc->lpfnWndProc = WndProc;
    wc->hInstance = hinstance;
    wc->lpszClassName = s_application_name;
    wc->hIcon = (HICON)LoadImage(hinstance, application_icon, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    wc->hIconSm = (HICON)LoadImage(hinstance, application_icon, IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    wc->hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassEx(wc);
    *hwnd = CreateWindowEx(
        0, s_application_name, s_application_name, 
        WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_SYSMENU, 
        200, 200, width, height, 0, 0, hinstance, 0 
    );
    if (!*hwnd) {
        fatal_error(err_create_window);
        return 0;
    }
    return 1;
}

static HGLRC create_temp_context(HDC hdc, HWND hwnd) {
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

static HGLRC create_context(HDC hdc, HWND hwnd) {
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
    fatal_error(err_opengl_context);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    ExitProcess(0);
}

#endif