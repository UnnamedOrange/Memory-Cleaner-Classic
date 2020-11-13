#pragma once

#ifndef _TKERNEL_INC
#define _TKERNEL_INC
#pragma warning(push, 3)

// 链接库
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "dwmapi.lib")

// 忽视不必要的警告
#pragma warning(disable: 4001)
#pragma warning(disable: 4100)
#pragma warning(disable: 4699)
#pragma warning(disable: 4710)
#pragma warning(disable: 4514)
#pragma warning(disable: 4512)
#pragma warning(disable: 4245)
#pragma warning(disable: 4312)
#pragma warning(disable: 4244)
#pragma warning(disable: 4995)
#pragma warning(disable: 4146)
#pragma warning(disable: 4267)
#pragma warning(disable: 4201)
#pragma warning(disable: 4091)

// CRT
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cctype>
#include <cassert>
#include <climits>
#include <ctime>

// Windows Base
#include <Windows.h>
#include <windowsx.h>
#include <tchar.h>

// STL
#include <algorithm>
#include <vector>
#include <string>
#include <stack>
#include <queue>
#include <deque>
#include <map>
#include <set>
#include <bitset>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <functional>

// Extra
#include <gdiplus.h>
#include <tlhelp32.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h> 
#include <shlobj.h>
#include <shldisp.h>
#include <Psapi.h>
#include <dwmapi.h>

// 控件
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#pragma warning(pop)
#endif // !_TKERNEL_INC

#ifndef _TKERNEL
#define _TKERNEL
#pragma warning(push, 4)

#pragma region
#define chSTRIndirect(x) #x
#define chSTR(x)  chSTRIndirect(x)
#define TODO(desc) message(__FILE__ "(" chSTR(__LINE__) "):" #desc) // #pragma
#define DebugBreak() _asm { int 3 }
#pragma endregion TODO HELPER

#pragma region

// HANDLE_DLGMSG
#define HANDLE_DLGMSG(hWnd, message, fn)\
   case (message): return (SetDlgMsgResult(hWnd, message,\
      HANDLE_##message((hWnd), (wParam), (lParam), (fn))))

// Custom Message
#define WM_INIT (WM_USER + 0x8001)

/* VOID OnInit(HWND hwnd) */
#define HANDLE_WM_INIT(hwnd, wParam, lParam, fn)\
	((fn)((hwnd)), 0L)
#define FORWARD_WM_INIT(hwnd, fn)\
	(VOID)(fn)((hwnd), WM_INIT, 0L, 0L)

/* VOID OnMoving(HWND hwnd, RECT* pRect) */
#define HANDLE_WM_MOVING(hwnd, wParam, lParam, fn)\
	((fn)((hwnd), (RECT*)(lParam)), 0L)
#define FORWARD_WM_MOVING(hwnd, pRect, fn)\
	(VOID)(fn)((hwnd), WM_MOVING, 0L, (LPARAM)(pRect))

/* VOID OnExitSizeMove(HWND hwnd) */
#define HANDLE_WM_EXITSIZEMOVE(hwnd, wParam, lParam, fn)\
	((fn)((hwnd)), 0L)
#define FORWARD_WM_EXITSIZEMOVE(hwnd, pRect, fn)\
	(VOID)(fn)((hwnd), WM_EXITSIZEMOVE, 0L, 0L)

#pragma endregion HANDLE_MSG

// HINST
extern "C" const IMAGE_DOS_HEADER __ImageBase; // &__ImageBase
#define HINST (HINSTANCE)(&__ImageBase)

namespace std
{
	// 定义 string cin cout 的 TCHAR 形式
#ifdef UNICODE

	typedef wstring _tstring;
#define tcin wcin
#define tcout wcout

#else // !UNICODE

	typedef string _tstring;
#define tcin cin
#define tcout cout

#endif // UNICODE
}

namespace tk
{
	//const
#pragma TODO(抛弃 SKIP)
//#define SKIP (-1)
//#define STRING_LENGTH ((size_t)256)
	const INT SKIP = -1;
	const size_t STRING_LENGTH = 256;

	//项目
	namespace Debug
	{
		VOID DebugString(LPCTSTR lpOutputString, ...);
		BOOL WriteLogFile(LPCTSTR lpcTitle = nullptr, LPCTSTR lpcText = nullptr, LPCTSTR lpcFileName = nullptr, ...);
	};

	//应用
	class App
	{
		static BOOL bUsedApplicationClass;

	protected:
		std::_tstring strAppName;
		std::_tstring strGUID;
		VS_FIXEDFILEINFO ffi;

		HANDLE hMutex;
		BOOL InitApp();

	public:
		const HINSTANCE hInstance;

	public:
		App(LPCTSTR szGUID, LPCTSTR szAppName);
		~App();

		BOOL GetAppName(LPTSTR szBuffer, size_t nSize);
		BOOL GetGUID(LPTSTR szBuffer, size_t nSize);
		DWORD MajorVerH();
		DWORD MajorVerL();
		DWORD MinorVerH();
		DWORD MinorVerL();

		DWORD RegReadAppCfg(LPCTSTR lpcValueName, DWORD dwDefault = 0);
		LSTATUS RegWriteAppCfg(LPCTSTR lpcValueName, DWORD dwValue, BOOL bDelete = FALSE);
		BOOL RegReadAppCfg(LPVOID lpConfig, size_t cbSize, LPCTSTR lpcValueName = nullptr);
		BOOL RegWriteAppCfg(LPCVOID lpcConfig, size_t cbSize, LPCTSTR lpcValueName = nullptr);
		BOOL RegSetAutoBoot(BOOL bSet = TRUE, LPCTSTR lpcPath = NULL);
		BOOL RegGetAutoBoot(LPCTSTR lpcPath = NULL);

		BOOL ResourceToFile(LPCTSTR lpcName, LPCTSTR lpcType, LPCTSTR lpcFileName);
		BOOL About(HWND hwnd, LPCTSTR lpcOtherStuff = TEXT(""), HICON hIcon = NULL, ...);
		BOOL SingleInstance(); // 返回TRUE代表重复实例
		BOOL RevokeSingleInstance();

		virtual INT_PTR InitializeResource(HINSTANCE hInstance, LPTSTR szCmdLine, INT iCmdShow);
		virtual INT_PTR ReleaseResource(HINSTANCE hInstance, LPTSTR szCmdLine, INT iCmdShow);
		virtual INT_PTR RegisterClasses(HINSTANCE hInstance, LPTSTR szCmdLine, INT iCmdShow);
		virtual INT_PTR InitializeInstance(HINSTANCE hInstance, LPTSTR szCmdLine, INT iCmdShow);
		virtual INT_PTR Execute(HINSTANCE hInstance, LPTSTR szCmdLine, INT iCmdShow);
		virtual INT_PTR Run(HINSTANCE hInstance, LPTSTR szCmdLine, INT iCmdShow);
	};
	BOOL MsgBox(HWND hwnd = NULL, LPCTSTR lpcText = TEXT(""), LPCTSTR lpcCaption = TEXT(""), UINT uType = MB_ICONINFORMATION, ...);

	//DLL应用
	class DllApp
	{
	protected:
		std::_tstring strAppName;
		std::_tstring strGUID;
		VS_FIXEDFILEINFO ffi;

		BOOL InitApp();

	public:
		const HINSTANCE hInstance;

	public:
		DllApp(LPCTSTR szGUID, LPCTSTR szAppName);
		~DllApp();

		BOOL GetAppName(LPTSTR szBuffer, size_t nSize);
		BOOL GetGUID(LPTSTR szBuffer, size_t nSize);
		DWORD MajorVerH();
		DWORD MajorVerL();
		DWORD MinorVerH();
		DWORD MinorVerL();

		DWORD RegReadAppCfg(LPCTSTR lpcValueName, DWORD dwDefault = 0);
		LSTATUS RegWriteAppCfg(LPCTSTR lpcValueName, DWORD dwValue, BOOL bDelete = FALSE);
		BOOL RegReadAppCfg(LPVOID lpConfig, size_t cbSize, LPCTSTR lpcValueName = nullptr);
		BOOL RegWriteAppCfg(LPCVOID lpcConfig, size_t cbSize, LPCTSTR lpcValueName = nullptr);

		BOOL ResourceToFile(LPCTSTR lpcName, LPCTSTR lpcType, LPCTSTR lpcFileName);
		BOOL About(HWND hwnd, LPCTSTR lpcOtherStuff = TEXT(""), HICON hIcon = NULL, ...);

		friend BOOL MsgBox(HWND hwnd, LPCTSTR lpcText, LPCTSTR lpcCaption, UINT uType, ...);

		virtual INT_PTR InitializeResource(HINSTANCE hInstance);
		virtual INT_PTR ReleaseResource(HINSTANCE hInstance);
		virtual INT_PTR RegisterClasses(HINSTANCE hInstance);
		virtual INT_PTR InitializeInstance(HINSTANCE hInstance);
		virtual INT_PTR Execute(HINSTANCE hInstance);
		virtual INT_PTR Run(HINSTANCE hInstance);
	};

	//自动加载GDIP
	class GDIP
	{
	private:
		ULONG_PTR gdiplusToken;

	public:
		GDIP();
		~GDIP();
#ifdef UNICODE
		static BOOL DrawStringShadow(Gdiplus::Graphics& graphics, LPCWSTR lpcText, const Gdiplus::Font* font, const Gdiplus::PointF& origin, const Gdiplus::Brush* brush);
		static BOOL DrawStringShadow(Gdiplus::Graphics& graphics, LPCWSTR lpcText, const Gdiplus::Font* font, const Gdiplus::RectF& layoutRect, const Gdiplus::StringFormat* stringFormat, const Gdiplus::Brush* brush);
#endif
	};

	//私有字体
	class PrivateFont
	{
	protected:
		HANDLE fHandle;
		LPVOID lpFont;
		DWORD dwSize;

		virtual VOID Unload();

	public:
		PrivateFont();
		~PrivateFont();

		virtual BOOL Init(HINSTANCE hInst, LPCTSTR lpcName, LPCTSTR lpcType);
		virtual BOOL Init(LPCTSTR lpcFileName);
	};

	//带GDIP支持的私有字体
	class PrivateFontPlus : public PrivateFont
	{
	protected:
		VOID Unload() override;
		Gdiplus::PrivateFontCollection* pfc;

	public:
		~PrivateFontPlus();

		BOOL Init(HINSTANCE hInst, LPCTSTR lpcName, LPCTSTR lpcType) override;
		BOOL Init(LPCTSTR lpcFileName) override;
		INT GetFamilyCount();
		INT GetFamilies(INT numSought, Gdiplus::FontFamily* gpfamilies, INT* numFound);
	};

	//常用字符串
	class ComStr
	{
	private:
		TCHAR* mes[12];		//Month_En_s
		TCHAR* mef[12];		//Month_En_f
		TCHAR* mc[12];		//Month_Cn
		TCHAR* des[7];		//DayOfWeek_En_s
		TCHAR* def[7];		//DayOfWeek_En_f
		TCHAR* dc[7];		//DayOfWeek_Cn
	public:
		ComStr();
		~ComStr();

		LPCTSTR Month_En_s(INT index);
		LPCTSTR Month_En_f(INT index);
		LPCTSTR Month_Cn(INT index);
		LPCTSTR DayOfWeek_En_s(INT index);
		LPCTSTR DayOfWeek_En_f(INT index);
		LPCTSTR DayOfWeek_Cn(INT index);
	};

	//时钟
	class Timer
	{
		TIMERPROC tp;
		HWND hWnd;
		UINT iTimerId;
		BOOL bSet;

	public:
		Timer() = delete;
		Timer(const Timer&) = delete;
		Timer(TIMERPROC pProc = nullptr);
		~Timer();

		VOID SetProc(TIMERPROC pProc);
		UINT Set(UINT uElapse);
		UINT Set(HWND hwnd, UINT_PTR nId_Event, UINT uElapse);
		VOID Kill();
	};

	//窗口位置
	class WndPos
	{
		BOOL bSysDock;
		BOOL isWin10;

		HWND hWnd;

		INT iLeft;
		INT iTop;
		INT iWidth;
		INT iHeight;

		//minSize
		POINT minTrackSize;

		//Dock
		BOOL bDock;
		INT LastNCMsg;
		BOOL bNCMouseDown;
		POINT pMouse;
		RECT rectOrigin; //用于保存一般的信息
		BOOL isDocking; //1左2右
		BOOL bMaximize;

		//InScreen
		BOOL bInScreen;
		BOOL bBar;
		RECT rectWnd;

	public:
		WndPos(HWND hwnd = NULL);
		HWND SetHwnd(HWND hwnd);

		VOID SetMinTrackSize(INT cx = 0, INT cy = 0);
		VOID SetDock(BOOL bSet = TRUE);
		VOID SetInScreen(BOOL bSet = TRUE);

		LRESULT VirtualProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	};

	//DPI助手
	class DPIHelper
	{
		static const INT iRegularDPI = 96;
		INT iDPI;
		DWORD dwCacheTime;

		inline void cache()
		{
			if (GetTickCount() - dwCacheTime <= 1000) return;

			HDC hdc = GetDC(NULL);
			iDPI = GetDeviceCaps(hdc, LOGPIXELSX);
			ReleaseDC(0, hdc);

			dwCacheTime = GetTickCount();
		}

	public:
		DPIHelper() : iDPI(NULL), dwCacheTime()
		{
			cache();
			if (!iDPI) iDPI = iRegularDPI;
		}

#define __CALC(x) MulDiv((x), iDPI, iRegularDPI)
#define __SET(x) x = __CALC(x)
		INT operator() (const INT& x)
		{
			return __CALC(x);
		}
		POINT operator() (const POINT& x)
		{
			POINT ret = x;
			__SET(ret.x);
			__SET(ret.y);
			return ret;
		}
		SIZE operator() (const SIZE& x)
		{
			SIZE ret = x;
			__SET(ret.cx);
			__SET(ret.cy);
			return ret;
		}
		RECT operator() (const RECT& x)
		{
			RECT ret = x;
			__SET(ret.left);
			__SET(ret.top);
			__SET(ret.right);
			__SET(ret.bottom);
			return ret;
		}
#undef __CALC
#undef __SET

		template <class T>
		inline T operator() (T x)
		{
			cache();
			return x * T(iDPI) / T(iRegularDPI);
		}
	};

	//托盘(实验)
	class Tray
	{
#define WM_TRAY (WM_USER + 0x8000)
		/* VOID OnTray(HWND hwnd, UINT message, UINT identifier) */
#define HANDLE_WM_TRAY(hwnd, wParam, lParam, fn)\
	((fn)((hwnd), (UINT)(lParam), (UINT)(wParam)), 0L)
#define FORWARD_WM_TRAY(hwnd, message, identifier, fn)\
	(VOID)(fn)((hwnd, WM_TRAY, (WPARAM)(identifier), (LPARAM)(message)))
		NOTIFYICONDATA nid;
		BOOL is_Inited;

	public:
		UINT WM_TASKBARCREATED;

	public:
		Tray(HWND hwnd = NULL);
		~Tray();

		VOID Init(HWND hwnd);
		VOID Add(); //重建
		VOID Add(HICON hIcon, LPCTSTR lpcInfo);
		VOID Delete();
		BOOL Balloon(LPCTSTR lpcText, LPCTSTR lpcTitle, DWORD dwInfoFlags = NIIF_INFO, HICON hIconBalloon = NULL);
	};

	//通用对话框（实验）
	class ComDlg
	{
	private:
		OPENFILENAME ofn;

	public:
		ComDlg();
		~ComDlg();

		BOOL OpenFileDlg(LPTSTR lpFile, DWORD nMaxFile, HWND hwndOwner = NULL, LPCTSTR lpcTitle = TEXT("打开"), LPCTSTR lpcFilter = TEXT("所有文件\0*.*\0\0"), DWORD dwFilterIndex = 1, LPCTSTR lpcDefPath = nullptr, LPTSTR lpFileTitle = nullptr, DWORD nMaxFileTitle = NULL, DWORD Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST);
		BOOL SaveFileDlg(LPTSTR lpFile, DWORD nMaxFile, HWND hwndOwner = NULL, LPCTSTR lpcTitle = TEXT("保存"), LPCTSTR lpcFilter = TEXT("所有文件\0*.*\0\0"), DWORD dwFilterIndex = 1, LPCTSTR lpcDefExt = nullptr, LPCTSTR lpcDefPath = nullptr, LPTSTR lpFileTitle = nullptr, DWORD nMaxFileTitle = NULL, DWORD Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT);
	};

	//文本文件读取（实验）
	class TextFile
	{
	private:
		LPTSTR lpFileName;

	public:
		TextFile(LPCTSTR lpcFileName = nullptr);
		~TextFile();

		VOID SetFileName(LPCTSTR lpcFileName);
		BOOL ReadFile(HWND hwnd);
		BOOL ReadFile(LPTSTR lpString, size_t nMaxLength);
		BOOL WriteFile(HWND hwnd);
		BOOL WriteFile(LPCTSTR lpcString);
	};

	//系统相关
	namespace System
	{
#define KEYDOWN 0
#define KEYDOWN_EX KEYEVENTF_EXTENDEDKEY
#define KEYUP KEYEVENTF_KEYUP
#define KEYUP_EX KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP
#define KEYUPDOWN 4
#define KEYDOWNUP 5

#define LBDOWN 1
#define LBUP 2
#define LBCLICK 3
#define LBDBCLICK 4
#define RBDOWN 5
#define RBUP 6
#define RBCLICK 7
#define RBDBCLICK 8
#define MBDOWN 9
#define MBUP 10
#define MBCLICK 11
#define MBDBCLICK 12
#define MWHEEL 13

		INT cxS();
		INT cyS();
		BOOL GetDesktopPath(LPTSTR lpPath);
		BOOL SystemShutdown(DWORD uFlags = EWX_SHUTDOWN, BOOL bForce = FALSE);
		BOOL SystemSleep(BOOLEAN Hibernate = TRUE, BOOLEAN DisableWakeEvent = FALSE);
		BOOL GetVersion(LPOSVERSIONINFO lpVersionInformation);
		BOOL RunBat(LPCTSTR lpcBatText, BOOL bNewConsole = TRUE, BOOL bShow = TRUE);
		BOOL DeleteTree(LPCTSTR lpcPath);
		VOID KeybdEvent(BYTE bVk, DWORD dwFlags = KEYDOWNUP);
		VOID MouseEvent(DWORD dwFlags = LBCLICK, DWORD dwData = 20, DWORD dx = SKIP, DWORD dy = SKIP, HWND hwnd = NULL); //dwData在无意义时表示延迟
	}

	//进程相关
	namespace Process
	{
		BOOL EnableDebugPrivilege(BOOL fEnable = TRUE);
		BOOL SetPriorityClass(INT nPriority = HIGH_PRIORITY_CLASS);
		BOOL TerminateProcess(DWORD dwProcessId);
		DWORD GetPIdFromWindow(HWND hwnd);
		DWORD GetPIdFromName(LPCTSTR lpcName);
		BOOL GetProcessNameFromPId(DWORD dwProcessId, LPTSTR lpString, size_t iMaxLength);
		HANDLE CreateProcessEz(LPCTSTR lpcPath, LPTSTR lpCmd, BOOL bShowWindow = TRUE);
		VOID SuspendProcess(DWORD dwProcessId, BOOL fSuspend = TRUE);
		DWORD GetThreadModuleName(HANDLE hProcess, HANDLE hThread, LPTSTR lpName, size_t iMaxLength);
		BOOL InjectDll(LPCSTR lpcDllPath, HANDLE hProcess);
		BOOL InjectDll(LPCSTR lpcDllPath, DWORD dwPId);
		BOOL InjectDll(LPCWSTR lpcDllPath, HANDLE hProcess);
		BOOL InjectDll(LPCWSTR lpcDllPath, DWORD dwPId);
	}

	//线程相关
	namespace Thread
	{
		DWORD CreateThreadEz(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter = NULL, DWORD dwCreationFlags = NULL, INT nPriority = THREAD_PRIORITY_NORMAL, PHANDLE phThread = nullptr);
	}

	//窗口相关
	namespace Windows
	{
#define WS_NOFRAME WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP | WS_SYSMENU //无边框窗口
#define WS_CHANGEABLE WS_OVERLAPPEDWINDOW //可调边框窗口
#define WS_UNCHANGEABLE WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX //不可调边框窗口

		BOOL SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey = SKIP, BYTE bAlpha = SKIP);
		BOOL SetWindowTransparent(HWND hwnd, BOOL iTransparent);
		HWND GetFocusEx();
		BOOL SetFocusEx(HWND hwnd);
		HWND WindowFromCursor();
		BOOL SetWindowTextFormat(HWND hwnd, LPCTSTR lpcText, ...);
		BOOL SetWindowStyle(HWND hwnd, LONG dwNewLong, BOOL bSet = TRUE);
		BOOL SetWindowExStyle(HWND hwnd, LONG dwNewLong, BOOL bSet = TRUE);
		RECT GetWorkArea(); //包含边界
		POINT GetPointInWorkArea(RECT rWorkArea, POINT pScreen);
		BOOL SetClientPos(HWND hWnd, HWND hWndInsertAfter, INT X, INT Y, INT cx, INT cy, UINT uFlags);
		BOOL CenterWindow(HWND hWnd);
	}

	//GDI相关
	namespace GDI
	{
#define CF_ATTR_BOLD          1 //粗体
#define CF_ATTR_ITALIC        2 //斜体
#define CF_ATTR_UNDERLINE     4 //下划线
#define CF_ATTR_STRIKEOUT     8 //删除线

		BYTE R(COLORREF rgb);
		BYTE G(COLORREF rgb);
		BYTE B(COLORREF rgb);

		HFONT CreateFontEz(HDC hdc, LPCTSTR szFaceName, INT iDeciPtHeight, INT iDeciPtWidth = NULL, INT iAttributes = NULL, BOOL fLogRes = TRUE);
		BOOL Blt(HDC hdcDest, INT xDest, INT yDest, INT wDest, INT hDest, HDC hdcSrc, INT xSrc, INT ySrc, INT wSrc = SKIP, INT hSrc = SKIP, INT alpha = SKIP);
		HDC CreateCompatibleDC(HDC hdc, INT cx = SKIP, INT cy = SKIP);
		HBITMAP HDCToHBitmap(HDC hdc, INT nWidth, INT nHeight);
		BOOL HBitmapToFile(HBITMAP hBitmap, LPCTSTR lpcFileName);
	}

	//注册表相关
	namespace Reg
	{
		LSTATUS RegSetKey(HKEY hKey, LPCTSTR lpcSubKey, LPCTSTR lpcValueName, DWORD dwType, CONST BYTE* lpData, DWORD cbData);
		DWORD RegGetKey(HKEY hKey, LPCTSTR lpcSubKey, LPCTSTR lpcValueName, DWORD dwDefault = 0);
		DWORD RegGetKey(HKEY hKey, LPCTSTR lpcSubKey, LPCTSTR lpcValueName, LPTSTR lpData, size_t iMaxLength, LPCTSTR szDefault = TEXT(""));
		BOOL RegGetKey(HKEY hKey, LPCTSTR lpcSubKey, LPCTSTR lpcValueName, LPVOID lpData, size_t cbSize);
		LSTATUS RegDeleteKeyEz(HKEY hKey, LPCTSTR lpcSubKey, LPCTSTR lpcValueName);
	}

	//文件相关
	namespace File
	{
		BOOL IsFileValid(LPCTSTR lpcFileName);
		BOOL GetFileVersion(LPCTSTR lpcFileName, VS_FIXEDFILEINFO* ffi);
		BOOL WriteMultiBytes(LPCTSTR lpcFileName, LPCWSTR lpcText, DWORD dwLength = SKIP);
		BOOL WriteMultiBytes(LPCTSTR lpcFileName, LPCSTR lpcText, DWORD dwLength = SKIP);
	}

	//针对文件名的字符串处理
	namespace Name
	{
		size_t GetLastPath(LPTSTR lpStr); //最后没有\\ //
		size_t GetFileName(LPTSTR lpStr);
		size_t GetNameWithoutExtra(LPTSTR lpStr);
		size_t GetExtraName(LPTSTR lpStr);
		size_t AddQuotes(LPTSTR lpStr, size_t nSize);
	}

	//全局命名空间声明
	using namespace Debug;
	using namespace System;
	using namespace Process;
	using namespace Thread;
	using namespace Windows;
	using namespace GDI;
	using namespace Reg;
	using namespace File;
	using namespace Name;
}

#endif // !_TKERNEL