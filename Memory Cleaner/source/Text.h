#pragma once
#include "..\TKernel\TKernel.h"
#include "resource.h"

namespace Text
{
	BOOL RegisterClasses();
	VOID CALLBACK AnimationTimer(HWND hwnd, UINT message, UINT_PTR iTimerId, DWORD dwTime);

	struct WndStruct
	{
		HWND hWnd;
		TCHAR szText[tk::STRING_LENGTH];

		tk::Timer timer;
		INT iCount;

		WndStruct(HWND hwnd) :hWnd(hwnd), timer(AnimationTimer), iCount(NULL),
			szText{ NULL }
		{
		}
		~WndStruct()
		{
		}

		VOID Update()
		{
			iCount = 0;
			timer.Set(hWnd, 0, 0);
		}

		BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
		VOID OnPaint(HDC hdc, HWND hwnd, BOOL fTransparent);
		VOID OnPaint(HWND hwnd);
		VOID OnSetText(HWND hwnd, LPCTSTR lpszText);
		INT OnGetText(HWND hwnd, int cchTextMax, LPTSTR lpszText);
		VOID OnDestroy(HWND hwnd);

		LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	};
	inline LRESULT CALLBACK VirtualProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		WndStruct* p = nullptr;
		if (!(p = (WndStruct*)(GetWindowLongPtr(hwnd, GWLP_USERDATA))))
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(p = new WndStruct(hwnd)));
		}
		return p->WndProc(hwnd, message, wParam, lParam);
	}
}