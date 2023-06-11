#pragma once
#include "..\TKernel\TKernel.h"
#include "WndPos.h"
#include "resource.h"

namespace MainWindow
{
	BOOL RegisterClasses();
	INT_PTR InitInstance();
	DWORD WINAPI MsgLoop(LPVOID lpParam);
	DWORD WINAPI AnimationThread(LPVOID lpParam);

	struct WndStruct
	{
		HWND hWnd;
		HANDLE hThread;
		HANDLE hEvent;
		HANDLE hSignal;

		WndPos wp;

		DWORD dwTransparent;
		INT bMouseHover;
		INT bMouseDown;

		WndStruct(HWND hwnd) :hWnd(hwnd), hThread(NULL), hEvent(NULL), hSignal(NULL),
			wp(hwnd),
			dwTransparent(NULL), bMouseHover(FALSE), bMouseDown(FALSE)
		{
			wp.enable_confine_to_screen();
			hSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
			hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			tk::CreateThreadEz(AnimationThread, this, NULL, NULL, &hThread);
		}
		~WndStruct()
		{
			SetEvent(hSignal);
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			CloseHandle(hEvent);
			CloseHandle(hSignal);
		}

		VOID Update()
		{
			SetEvent(hEvent);
		}

		BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
		VOID OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
		VOID OnMoving(HWND hwnd, RECT* pRect);
		VOID OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
		VOID OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
		VOID OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
		VOID OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
		VOID OnNCLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest);
		VOID OnDisplayChange(HWND hwnd, UINT bitsPerPixel, UINT cxScreen, UINT cyScreen);
		BOOL OnQueryEndSession(HWND hwnd);
		VOID OnDestroy(HWND hwnd);
		VOID OnTimer(HWND hwnd, UINT id);

		VOID OnPaint(HWND hwnd);
		VOID OnTray(HWND hwnd, UINT message, UINT identifier);
		VOID OnFindInstance(HWND hwnd);
		VOID OnResetPosition(HWND hwnd, BOOL fShow, BOOL fDisplayChanged);
		VOID OnUpdateInfo(HWND hwnd);

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