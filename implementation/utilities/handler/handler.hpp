#pragma once
//includes to use windows functions
#include <windows.h>
#include <dwmapi.h>
#include <iostream>

//includes for spoofing function to be ud++
#include <implementation/utilities/handler/spoofing/xor.h>
#include <implementation/utilities/handler/spoofing/callstack.h>

enum TYPES
{
    move = 1,
    left_up = 4,
    left_down = 2,
    right_up = 8,
    right_down = 16
};

struct MOUSE_INFO
{
    int move_direction_x;
    int move_direction_y;
    unsigned int mouse_data;
    TYPES mouse_options;
    unsigned int time_offset_in_miliseconds;
    void* extra_info;
};

inline bool(*_NtUserInjectMouseInput)(MOUSE_INFO*, int) = nullptr;

class inject_mouse_handler
{
public:
    void load_win32u() {
        hide;
        HMODULE win32u = hide_call(LoadLibraryA)(enc("win32u.dll"));
        if (!win32u) {
            return;
        }

        void* NtUserInjectMouseInputProc = (void*)GetProcAddress(win32u, enc("NtUserInjectMouseInput"));
        if (!NtUserInjectMouseInputProc) {
            hide_call(FreeLibrary)(win32u);
            return;
        }

        *(void**)&_NtUserInjectMouseInput = NtUserInjectMouseInputProc;
        hide_call(FreeLibrary)(win32u);
    }

    bool move(int X, int Y)
    {
        hide;
        MOUSE_INFO info = {};
        info.mouse_data = 0;
        info.mouse_options = TYPES::move;
        info.move_direction_x = X;
        info.move_direction_y = Y;
        info.time_offset_in_miliseconds = 0;
        return _NtUserInjectMouseInput(&info, 1);
    }
};
inline std::unique_ptr<inject_mouse_handler> mouse_handler = std::make_unique<inject_mouse_handler>();


class window_handler_ {
public:

    void load_user32() {
        hide;
        user32 = hide_call(LoadLibraryA)(enc("user32.dll"));
        if (!user32) {
            return;
        }

        void* procAddr = (void*)GetProcAddress(user32, enc("FindWindowA"));
        if (procAddr) {
            *(void**)&_FindWindow = procAddr;
        }

        procAddr = (void*)GetProcAddress(user32, enc("SetWindowPos"));
        if (procAddr) {
            *(void**)&_SetWindowPos = procAddr;
        }

        procAddr = (void*)GetProcAddress(user32, enc("SetLayeredWindowAttributes"));
        if (procAddr) {
            *(void**)&_SetLayeredWindowAttributes = procAddr;
        }

        procAddr = (void*)GetProcAddress(user32, enc("UpdateWindow"));
        if (procAddr) {
            *(void**)&_UpdateWindow = procAddr;
        }
    }

    void load_dwmapi() {
        hide;
        dwmapi = hide_call(LoadLibraryA)(enc("dwmapi.dll"));
        if (!dwmapi) {
            return;
        }

        void* procAddr = (void*)GetProcAddress(dwmapi, enc("DwmExtendFrameIntoClientArea"));
        if (procAddr) {
            *(void**)&_DwmExtendFrameIntoClientArea = procAddr;
        }
    }

    HWND find(const std::string& className, const std::string& windowName) {
        hide;
        if (_FindWindow != nullptr) {
            return _FindWindow(className.c_str(), windowName.c_str());
        }
        return nullptr;
    }

    bool set(HWND hwnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
        hide;
        if (_SetWindowPos != nullptr) {
            return _SetWindowPos(hwnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
        }
        return false;
    }

    bool set_layered(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags) {
        hide;
        if (_SetLayeredWindowAttributes != nullptr) {
            return _SetLayeredWindowAttributes(hwnd, crKey, bAlpha, dwFlags);
        }
        return false;
    }

    HRESULT extend_frame(HWND hwnd, const MARGINS* pMargins) {
        hide;
        if (_DwmExtendFrameIntoClientArea != nullptr) {
            return _DwmExtendFrameIntoClientArea(hwnd, pMargins);
        }
        return E_FAIL;
    }

    bool update(HWND hwnd) {
        hide;
        if (_UpdateWindow != nullptr) {
            return _UpdateWindow(hwnd);
        }
        return false;
    }

private:
    HMODULE user32 = nullptr;
    HMODULE dwmapi = nullptr;
    HWND(WINAPI* _FindWindow)(LPCSTR, LPCSTR) = nullptr;
    BOOL(WINAPI* _SetWindowPos)(HWND, HWND, int, int, int, int, UINT) = nullptr;
    BOOL(WINAPI* _SetLayeredWindowAttributes)(HWND, COLORREF, BYTE, DWORD) = nullptr;
    HRESULT(WINAPI* _DwmExtendFrameIntoClientArea)(HWND, const MARGINS*) = nullptr;
    BOOL(WINAPI* _UpdateWindow)(HWND) = nullptr;
};

inline std::unique_ptr<window_handler_> window_handler = std::make_unique<window_handler_>();