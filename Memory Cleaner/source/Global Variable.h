#pragma once
#include "..\TKernel\TKernel.h"
#include "resource.h"

#include "Memory Cleaner.h"
#include "Net Speed.h"
#include "Tools.h"
#include "Main Window.h"
#include "Setting Window.h"

//Global
#define GetIconLarge() LoadIcon(HINST, MAKEINTRESOURCE(IDI_ICON))
#define GetIconSmall() HICON(LoadImage(HINST, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, NULL))
#define GetRBMenu() GetSubMenu(LoadMenu(HINST, MAKEINTRESOURCE(IDR_MENU_RB)), 0)

#define WM_FINDINSTANCE (WM_USER + 100)
#define WM_RESETPOSITION (WM_USER + 101)
#define WM_UPDATEINFO (WM_USER + 102)
/* VOID OnFindInstance(HWND hwnd) */
#define HANDLE_WM_FINDINSTANCE(hwnd, wParam, lParam, fn)\
	((fn)((hwnd)), 0L)
/* VOID OnResetPosition(HWND hwnd, BOOL fShow, BOOL fDisplayChanged) */
#define HANDLE_WM_RESETPOSITION(hwnd, wParma, lParam, fn)\
	((fn)((hwnd), (BOOL)(wParam), (BOOL)(lParam)), 0L)
#define SEND_WM_RESETPOSITION(hwnd, fShow, fDisplayChanged)\
	SendMessage((hwnd), WM_RESETPOSITION, (WPARAM)(fShow), (LPARAM)(fDisplayChanged))
/* VOID OnUpdateInfo(HWND hwnd) */
#define HANDLE_WM_UPDATEINFO(hwnd, wParam, lParam, fn)\
	((fn)((hwnd)), 0L)

//Memory Cleaner
extern const TCHAR szGUID[];
extern const TCHAR szAppName[];
extern TCHAR szLogPath[MAX_PATH];
extern MemoryCleaner mc;
extern NetSpeed ns;
extern MEMORYSTATUSEX msex;
extern tk::Timer timerClean;

extern tk::PrivateFontPlus PrivateFont;
extern tk::DPIHelper dpi;

//Net Speed

//Tools

//Main Window
namespace MainWindow
{
	extern const TCHAR szClassName[];
	extern const TCHAR szCaption[];
	extern HWND hwndMain;

	extern BOOL bLaunch;
}

//Setting Window
namespace SettingWindow
{
	extern const TCHAR szClassName[];
	extern const TCHAR szCaption[];
	extern HANDLE hThread;
	extern HWND hwndSetting;
}

//Text
namespace Text
{
	extern COLORREF colorBk;
}