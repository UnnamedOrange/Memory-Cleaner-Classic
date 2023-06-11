#pragma once

#include <Windows.h>

class WndPos {
private:
    HWND m_hwnd{};

    bool enable_confine = false;
    RECT window_rect{};
    POINT initial_cursor{};

public:
    WndPos(HWND hwnd = nullptr);

public:
    void set_hwnd(HWND hwnd = nullptr);
    void enable_confine_to_screen(bool enable = true);

public:
    LRESULT VirtualProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    void OnNCLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest);
    void OnMoving(HWND hwnd, RECT* rect);
};
