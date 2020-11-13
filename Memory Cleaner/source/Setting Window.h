#pragma once
#include "..\TKernel\TKernel.h"
#include "resource.h"

namespace SettingWindow
{
	VOID Create();

	BOOL RegisterClasses();
	INT_PTR InitInstance();
	DWORD WINAPI MsgLoop(LPVOID lpParam);
	DWORD WINAPI Run(LPVOID lpParam);
	DWORD CALLBACK AnimationThread(LPVOID lpParam);

	struct WndStruct
	{
		HWND hWnd;
		HANDLE hThread;
		HANDLE hEvent;
		HANDLE hSignal;

		BOOL bFoldStatus;
		HWND hwndUnfoldButton;
		INT cyWnd;
		INT cxUnfold;
		INT cxFold;
		INT cxCnt;

		WndStruct(HWND hwnd) :hWnd(hwnd), hThread(NULL), hEvent(NULL), hSignal(NULL),
			bFoldStatus(TRUE), hwndUnfoldButton(NULL),
			cyWnd(NULL), cxUnfold(NULL), cxFold(NULL), cxCnt(NULL)
		{
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
		VOID OnInit(HWND hwnd);
		VOID OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
		VOID OnUpdateInfo(HWND hwnd);
		VOID OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
		VOID OnClose(HWND hwnd);
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