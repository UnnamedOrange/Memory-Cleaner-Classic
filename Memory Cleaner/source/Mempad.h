#pragma once
#include "..\TKernel\TKernel.h"
#include "resource.h"

#include "Tools.h"

namespace Mempad
{
	BOOL RegisterClasses();
	VOID CALLBACK AnimationTimer(HWND hwnd, UINT message, UINT_PTR iTimerId, DWORD dwTime);

	struct WndStruct
	{
		HWND hWnd;

		tk::Timer timer;
		BOOL bOdd;
		INT iCount;

		INT bMouseHover;
		INT bMouseDown;

		INT cxMouse;
		INT cyMouse;
		SIZE size; //窗口大小
		INT bSide; //鼠标在哪边
		INT iShowWhat; // 0 - 内存, 1 - 网速

		static const INT maxn = 60;
		static const INT cxWidth = 8;
		static const INT nxInterval = 6;
		Tool::MonotonousQueue<ULONGLONG, maxn> memLoad;
		Tool::MonotonousQueue<INT, maxn> downSpeed;
		Tool::MonotonousQueue<INT, maxn> upSpeed;

		INT R;

		WndStruct(HWND hwnd) :hWnd(hwnd), timer(AnimationTimer), bOdd(FALSE), iCount(NULL),
			cxMouse(NULL), cyMouse(NULL), bSide(NULL), iShowWhat(FALSE),
			bMouseHover(FALSE), bMouseDown(FALSE),
			R(NULL)
		{
			memLoad.push(0);
			downSpeed.push(0);
			upSpeed.push(0);

			RECT rect;
			GetWindowRect(hwnd, &rect);
			size = { rect.right - rect.left, rect.bottom - rect.top };
		}
		~WndStruct()
		{
		}

		VOID Insert(ULONGLONG iMemLoad)
		{
			memLoad.push(iMemLoad);

			INT iDownloadSpeed = ns.GetDownloadSpeed();
			INT iUploadSpeed = ns.GetUploadSpeed();
			downSpeed.push(iDownloadSpeed);
			upSpeed.push(iUploadSpeed);

			memLoad.pop_until();
			downSpeed.pop_until();
			upSpeed.pop_until();

			bOdd = (bOdd + 1) % nxInterval;
			Update();
		}
		VOID Update()
		{
			iCount = 0;
			timer.Set(hWnd, 0, 0);
		}

		BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
		VOID OnPaint(HDC hdc, HWND hwnd, BOOL fTransparent);
		VOID OnPaint(HWND hwnd);
		VOID OnUpdateInfo(HWND hwnd);
		VOID OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
		VOID OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
		VOID OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
		VOID OnDestroy(HWND hwnd);

		LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	};
	inline LRESULT CALLBACK VirtualProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		WndStruct* p = nullptr;
		if (!(p = (WndStruct*)(GetWindowLongPtr(hwnd, GWLP_USERDATA))))
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (INT_PTR)(p = new WndStruct(hwnd)));
		}
		return p->WndProc(hwnd, message, wParam, lParam);
	}
}