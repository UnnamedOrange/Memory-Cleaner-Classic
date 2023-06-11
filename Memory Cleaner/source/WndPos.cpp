#include "WndPos.h"

#include <windowsx.h>

// VOID OnMoving(HWND hwnd, RECT* pRect)
#define HANDLE_WM_MOVING(hwnd, wParam, lParam, fn) ((fn)((hwnd), (RECT*)(lParam)), 0L)
#define FORWARD_WM_MOVING(hwnd, pRect, fn) (VOID)(fn)((hwnd), WM_MOVING, 0L, (LPARAM)(pRect))

WndPos::WndPos(HWND hwnd) {
    set_hwnd(hwnd);
}

void WndPos::set_hwnd(HWND hwnd) {
    m_hwnd = hwnd;
}
void WndPos::enable_confine_to_screen(bool enable) {
    enable_confine = enable;
}

LRESULT WndPos::VirtualProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (hwnd != m_hwnd)
        return LRESULT{};

    switch (message) {
        HANDLE_MSG(hwnd, WM_MOVING, OnMoving);
        HANDLE_MSG(hwnd, WM_NCLBUTTONDOWN, OnNCLButtonDown);
    }
    return 0;
}
void WndPos::OnNCLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest) {
    GetWindowRect(hwnd, &window_rect);
    GetCursorPos(&initial_cursor);
}
void WndPos::OnMoving(HWND hwnd, RECT* rect) {
    POINT cursor;
    GetCursorPos(&cursor);
    if (enable_confine) {
        auto& r = *rect;
        auto width = r.right - r.left;
        auto height = r.bottom - r.top;
        RECT confine_rect;
        {
            MONITORINFO mi{sizeof(mi)};
            HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            GetMonitorInfoW(hMonitor, &mi);
            confine_rect = mi.rcMonitor;
        }
        // 将窗口可能出现的位置更改为窗口左上角的点可能出现的位置。
        confine_rect.right -= width - 1;
        confine_rect.bottom -= height - 1;
        // 根据鼠标位置初步得出窗口左上角位置。
        r.left = window_rect.left + (cursor.x - initial_cursor.x);
        r.top = window_rect.top + (cursor.y - initial_cursor.y);
        // 窗口左上角的点。
        POINT pCorner{r.left, r.top};
        // 调整窗口左上角位置。
        if (pCorner.x < confine_rect.left) {
            r.left = confine_rect.left;
        }
        if (pCorner.y < confine_rect.top) {
            r.top = confine_rect.top;
        }
        if (pCorner.x > confine_rect.right) {
            r.left = confine_rect.right;
        }
        if (pCorner.y > confine_rect.bottom) {
            r.top = confine_rect.bottom;
        }
        // 根据左上角位置和大小调整右下角。
        r.right = r.left + width;
        r.bottom = r.top + height;
    }
}
