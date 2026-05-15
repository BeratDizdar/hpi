#include "hpi.h"
#define WIN32_LEAN_AND_MEAN
#define NOCRYPT
#include <Windows.h>
#include <stdlib.h>

static int curr_keys[256] = { FALSE };
static int prev_keys[256] = { FALSE };
static int input_mouse_left = FALSE;
static int input_mouse_right = FALSE;
static int input_mouse_x = 0;
static int input_mouse_y = 0;

static struct Timer {
    LARGE_INTEGER freq;
    LARGE_INTEGER last;
    float delta;
} timer;

static struct Window {
    HWND hwnd;
    DWORD style;
    int active;
    int width, height;

    // framebuffer thingi
    HDC hdc;
    HDC memdc;
    HBITMAP hbmp;
    HBITMAP oldbmp;
} window;

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
    switch (umessage) {
    case WM_CLOSE: DestroyWindow(hwnd); window.active = FALSE; return 0;
    case WM_MOUSEMOVE:
        input_mouse_x = LOWORD(lparam);
        input_mouse_y = HIWORD(lparam);
        return 0;
    case WM_LBUTTONDOWN: input_mouse_left = TRUE; return 0;
    case WM_LBUTTONUP: input_mouse_left = FALSE; return 0;
    case WM_RBUTTONDOWN: input_mouse_right = TRUE; return 0;
    case WM_RBUTTONUP: input_mouse_right = FALSE; return 0;
    default: return DefWindowProcW(hwnd, umessage, wparam, lparam);
    }
}

void Win32CreateWindow(const unsigned short* title, int w, int h, int bfullscreen)
{
    window.width = w;
    window.height = h;
    HMODULE instance = GetModuleHandle(NULL);
    WNDCLASSW wnd = {0};
    wnd.hInstance = instance;
    wnd.lpfnWndProc = wnd_proc;
    wnd.lpszClassName = L"wnd1";
    wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
    wnd.style = CS_OWNDC;
    RegisterClassW(&wnd);

    window.style = WS_VISIBLE | ((bfullscreen) ? WS_POPUPWINDOW : WS_OVERLAPPEDWINDOW);
    int x = CW_USEDEFAULT, y = CW_USEDEFAULT;
    if (bfullscreen)
    {
        // tcc kullandığım için böyle ekliyorum
        HMODULE user32 = LoadLibraryA("user32.dll");
        if (user32 != NULL) {
            FARPROC dpi_func = GetProcAddress(user32, "SetProcessDPIAware");
            if (dpi_func != NULL) ((BOOL(WINAPI*)(void))dpi_func)();
            FreeLibrary(user32);
            w = GetSystemMetrics(SM_CXSCREEN);
            h = GetSystemMetrics(SM_CYSCREEN);
        }
        x = 0;
        y = 0;
    }
    else
    {
        RECT rc = { 0, 0, w, h };
        AdjustWindowRect(&rc, window.style, FALSE);
        w = rc.right - rc.left;
        h = rc.bottom - rc.top;
    }


    window.hwnd = CreateWindowW(wnd.lpszClassName, title, window.style, x, y, w, h, NULL, NULL, instance, NULL);
    if (window.hwnd == NULL)
    {
        MessageBoxW(window.hwnd, L"Pencere Yaratılamadı!", L"WinAPI Error", MB_ICONERROR | MB_OK);
        exit(-1);
    }

    QueryPerformanceFrequency(&timer.freq);
    QueryPerformanceCounter(&timer.last);

    window.active = TRUE;
}

void Win32DestroyWindow()
{
    window.active = FALSE;
    DestroyWindow(window.hwnd);
}

void* Win32GetPointer()
{
    return (void*)window.hwnd;
}

int Win32ProcessMessages()
{
    MSG msg = {0};
    while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // input update
    for (int i = 0; i < 256; i++)
    {
        prev_keys[i] = curr_keys[i];
        curr_keys[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
    }

    // timer update
    LARGE_INTEGER cur;
    QueryPerformanceCounter(&cur);
    timer.delta = (float)(cur.QuadPart - timer.last.QuadPart) / (float)timer.freq.QuadPart;
    timer.last = cur;
    if (timer.delta > 0.1f) timer.delta = 0.1f;

    return window.active;
}

int Win32GetWidth()
{
    return window.width;
}

int Win32GetHeight()
{
    return window.height;
}

void* Win32GetFramebuffer()
{
    BITMAPINFOHEADER bmih = {sizeof(BITMAPINFOHEADER), window.width, -window.height, 1, 32, BI_RGB};
    BITMAPINFO bmi = {.bmiHeader = bmih};

    void* memory = NULL;

    window.hdc = GetDC(window.hwnd);
    window.memdc = CreateCompatibleDC(window.hdc);
    window.hbmp = CreateDIBSection(window.hdc, &bmi, DIB_RGB_COLORS, &memory, NULL, 0);
    window.oldbmp = (HBITMAP)SelectObject(window.memdc, window.hbmp);

    return memory;
}

void Win32BlitFramebuffer()
{
    BitBlt(window.hdc, 0, 0, window.width, window.height, window.memdc, 0, 0, SRCCOPY);
    Sleep(1);
}

void Win32DeleteFramebuffer()
{
    SelectObject(window.memdc, window.oldbmp);
    DeleteObject(window.hbmp);
    DeleteDC(window.memdc);
    ReleaseDC(window.hwnd, window.hdc);
}

int Win32KeyDown(int k)
{
    return curr_keys[k];
}

int Win32KeyPressed(int k)
{
    return curr_keys[k] && !prev_keys[k];
}

int Win32KeyReleased(int k)
{
    return !curr_keys[k] && prev_keys[k];
}

void Win32MouseLock()
{
    //
}

void Win32MouseUnlock()
{
    //
}

void Win32MousePos(int* x, int* y)
{
    *x = input_mouse_x;
    *y = input_mouse_y;
}

int Win32LeftMouseButton()
{
    return input_mouse_left;
}

int Win32RightMouseButton()
{
    return input_mouse_right;
}

float Win32GetDeltaTime()
{
    return timer.delta;
}

static IPlatform p = {
    .Release = Win32DestroyWindow,
    .GetPtr = Win32GetPointer,
    .Update = Win32ProcessMessages,
    .GetWidth = Win32GetWidth,
    .GetHeight = Win32GetHeight,
    .AllocateFramebuffer = Win32GetFramebuffer,
    .BlitFramebuffer = Win32BlitFramebuffer,
    .FreeFramebuffer = Win32DeleteFramebuffer,
    .KeyDown = Win32KeyDown,
    .KeyPressed = Win32KeyPressed,
    .KeyReleased = Win32KeyReleased,
    .MouseLock = Win32MouseLock,
    .MouseUnlock = Win32MouseUnlock,
    .MousePos = Win32MousePos,
    .MouseLeft = Win32LeftMouseButton,
    .MouseRight = Win32RightMouseButton,
};

IPlatform *HPI_Create(const unsigned short *title, int w, int h, int bFullscreen)
{
    Win32CreateWindow(title, w, h, bFullscreen); 
    return &p;
}