#include "..\TKernel\TKernel.h"
#include "resource.h"
#include "Global Variable.h"

#include "Text.h"
#include "Memory Cleaner.h"

//声明
namespace Text
{
	constexpr TCHAR szClassName[] = TEXT("Wnd::Text");
	constexpr TCHAR szCaption[] = TEXT("");

	COLORREF colorBk;
}

//杂项
BOOL Text::RegisterClasses()
{
	WNDCLASSEX wndclassex = { sizeof(WNDCLASSEX) }; //窗口类
	wndclassex.style = CS_DBLCLKS; //类风格
	wndclassex.lpfnWndProc = VirtualProc; //窗口过程
	wndclassex.cbClsExtra = 0;
	wndclassex.cbWndExtra = 0;
	wndclassex.hInstance = HINST; //实例句柄
	wndclassex.hIcon = GetIconLarge(); //图标句柄
	wndclassex.hIconSm = GetIconSmall();
	wndclassex.hCursor = LoadCursor(NULL, IDC_ARROW); //鼠标指针句柄
	wndclassex.hbrBackground = CreateSolidBrush(RGB(255, 255, 255)); //客户区颜色画刷
	wndclassex.lpszMenuName = NULL; //菜单句柄
	wndclassex.lpszClassName = szClassName; //窗口类名
	return !!RegisterClassEx(&wndclassex);
}

//窗口动画
namespace Text
{
	VOID CALLBACK AnimationTimer(HWND hwnd, UINT message, UINT_PTR iTimerId, DWORD dwTime)
	{
		WndStruct& ws = *(WndStruct*)(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		constexpr DWORD delay[] = { 500, 38 };
		constexpr INT times = 4;
		ws.timer.Set(hwnd, 0, delay[mc.mcc.bAnimation]);

		HDC hdc = GetDC(hwnd);
		ws.OnPaint(hdc, hwnd, mc.mcc.bAnimation);
		ReleaseDC(hwnd, hdc);

		if (ws.iCount >= times || !mc.mcc.bAnimation)
		{
			ws.timer.Kill();
			ws.iCount = 0;
		}
	}
}

//消息
namespace Text
{
	BOOL WndStruct::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
	{
		Update();
		return TRUE;
	}
	VOID WndStruct::OnPaint(HDC hdc, HWND hwnd, BOOL fTransparent)
	{
		using namespace Gdiplus;

		RECT rect_; GetWindowRect(hwnd, &rect_);
		SIZE size = { rect_.right - rect_.left, rect_.bottom - rect_.top };
		RectF rect(0, 0, size.cx, size.cy);

		HDC hdcMem = tk::CreateCompatibleDC(hdc, size.cx, size.cy);
		Graphics gMem(hdcMem);
		gMem.SetTextRenderingHint(TextRenderingHint::TextRenderingHintClearTypeGridFit);

		SolidBrush BrushBk(Color(255, tk::R(colorBk), tk::G(colorBk), tk::B(colorBk)));
		SolidBrush BrushFont(Color(255, 0, 0, 0));
		gMem.FillRectangle(&BrushBk, rect);

		FontFamily fontFamily;
		INT found;
		PrivateFont.GetFamilies(1, &fontFamily, &found);
		Font font(&fontFamily, dpi(19), FontStyleRegular, UnitPixel);

		StringFormat sf;
		sf.SetLineAlignment(StringAlignment::StringAlignmentCenter);
		//sf.SetAlignment(StringAlignment::StringAlignmentCenter);
		gMem.DrawString(szText, _tcslen(szText), &font, rect, &sf, &BrushFont);

		if (fTransparent)
			tk::Blt(hdc, 0, 0, size.cx, size.cy, hdcMem, 0, 0, size.cx, size.cy, MulDiv(255, 25, 100));
		else
			tk::Blt(hdc, 0, 0, size.cx, size.cy, hdcMem, 0, 0, size.cx, size.cy, tk::SKIP);
		DeleteDC(hdcMem);
	}
	VOID WndStruct::OnPaint(HWND hwnd)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		OnPaint(hdc, hwnd, FALSE);
		EndPaint(hwnd, &ps);
	}
	VOID WndStruct::OnSetText(HWND hwnd, LPCTSTR lpszText)
	{
		_tcscpy_s(szText, lpszText);
		Update();
	}
	INT WndStruct::OnGetText(HWND hwnd, int cchTextMax, LPTSTR lpszText)
	{
		return _tcscpy_s(lpszText, cchTextMax, szText);
	}
	VOID WndStruct::OnDestroy(HWND hwnd)
	{
		delete this;
	}
}
LRESULT CALLBACK Text::WndStruct::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hwnd, WM_SETTEXT, OnSetText);
		HANDLE_MSG(hwnd, WM_GETTEXT, OnGetText);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}