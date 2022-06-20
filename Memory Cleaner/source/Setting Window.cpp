#include "..\TKernel\TKernel.h"
#include "resource.h"
#include "Global Variable.h"

#include "Setting Window.h"
#include "Main Window.h"
#include "About Window.h"
#include "Memory Cleaner.h"
#include "Net Speed.h"
#include "Tools.h"

// #include <Python.h>
// #pragma comment(lib, "python37.lib")

//声明
namespace SettingWindow
{
	const TCHAR szClassName[] = TEXT("Wnd::Setting");
	const TCHAR szCaption[] = TEXT("Memory Cleaner");
	HANDLE hThread;
	HWND hwndSetting;
}

//杂项
VOID SettingWindow::Create()
{
	if (!hThread)
		tk::CreateThreadEz(Run, NULL, NULL, NULL, &hThread);
}
BOOL SettingWindow::RegisterClasses()
{
	WNDCLASSEX wndclassex = { sizeof(WNDCLASSEX) }; //窗口类
	wndclassex.style = CS_DBLCLKS; //类风格
	wndclassex.lpfnWndProc = VirtualProc; //窗口过程
	wndclassex.cbClsExtra = 0;
	wndclassex.cbWndExtra = DLGWINDOWEXTRA;
	wndclassex.hInstance = HINST; //实例句柄
	wndclassex.hIcon = GetIconLarge(); //图标句柄
	wndclassex.hIconSm = GetIconSmall();
	wndclassex.hCursor = LoadCursor(NULL, IDC_ARROW); //鼠标指针句柄
	wndclassex.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE)); //客户区颜色画刷
	wndclassex.lpszMenuName = NULL; //菜单句柄
	wndclassex.lpszClassName = szClassName; //窗口类名
	return !!RegisterClassEx(&wndclassex);
}
INT_PTR SettingWindow::InitInstance()
{
	return INT_PTR(CreateDialog(HINST, MAKEINTRESOURCE(IDD_SETTING), NULL, NULL));
}
DWORD WINAPI SettingWindow::MsgLoop(LPVOID lpParam)
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
DWORD WINAPI SettingWindow::Run(LPVOID lpParam)
{
	InitInstance();
	MsgLoop(NULL);
	CloseHandle(hThread);
	hThread = NULL;
	return 0;
}

//窗口大小变化动画效果
namespace SettingWindow
{
	DWORD CALLBACK AnimationThread(LPVOID lpParam)
	{
		WndStruct& ws = *(WndStruct*)lpParam;

		constexpr double scale = 1.0 / 3.0;
		constexpr DWORD delay = 10;
		HWND hwnd = ws.hWnd;
		HANDLE hWait[] = { ws.hSignal, ws.hEvent };

		DWORD dwRet;
		while (true)
		{
			dwRet = WaitForMultipleObjects(sizeof(hWait) / sizeof(HANDLE), hWait, FALSE, INFINITE);
			if (dwRet == WAIT_OBJECT_0)
				break;

			int target = ws.bFoldStatus ? ws.cxFold : ws.cxUnfold;
			if (mc.mcc.bAnimation)
			{
				int delta = std::abs(target - ws.cxCnt);
				delta = static_cast<int>(std::ceil(delta * scale));
				ws.cxCnt += (target > ws.cxCnt ? delta : -delta);
			}
			else
				ws.cxCnt = target;

			if (ws.cxCnt == target)
				ResetEvent(ws.hEvent);

			SetWindowPos(hwnd, NULL, NULL, NULL, ws.cxCnt, ws.cyWnd, SWP_NOZORDER | SWP_NOMOVE);

			dwRet = WaitForSingleObject(ws.hSignal, delay);
			if (dwRet == WAIT_OBJECT_0)
				break;
		}
		return 0;
	}
}

//消息
namespace SettingWindow
{
	BOOL WndStruct::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
	{
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)GetIconLarge());

		PostMessage(hwnd, WM_INIT, 0, 0);
		return TRUE;
	}
	VOID WndStruct::OnInit(HWND hwnd)
	{
		//确保句柄赋值
		hwndSetting = hwnd;

		//初始化控件信息
		hwndUnfoldButton = GetDlgItem(hwnd, IDC_BUTTON_UNFOLD);

		//初始化窗口大小数据
		RECT rectSetting, rectChild;
		GetWindowRect(hwnd, &rectSetting);
		HWND hwndChild = GetDlgItem(hwnd, IDC_CHECK_SHOW);
		GetWindowRect(hwndChild, &rectChild);
		cyWnd = rectSetting.bottom - rectSetting.top;
		cxUnfold = rectSetting.right - rectSetting.left;
		cxFold = cxUnfold - (rectSetting.right - rectChild.left);
		cxCnt = cxFold;

		//初始化窗口位置
		SetWindowPos(hwnd, NULL, NULL, NULL, cxCnt, cyWnd, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		tk::CenterWindow(hwndSetting);

		//控件初始化
		CheckDlgButton(hwnd, IDC_CHECK_SHOW, mc.mcc.bShow);
		CheckDlgButton(hwnd, IDC_CHECK_AMINATION, mc.mcc.bAnimation);
		CheckDlgButton(hwnd, IDC_CHECK_AUTOLAUNCH, mc.mcc.bAutoLaunch);
		CheckDlgButton(hwnd, IDC_CHECK_AUTOCLEAN, mc.mcc.bAutoClean);
		CheckDlgButton(hwnd, IDC_CHECK_QUICKER, mc.mcc.bQuicker);
		CheckDlgButton(hwnd, IDC_CHECK_AUTOCONTROL, mc.mcc.bAutoControl);
		tk::SetWindowTextFormat(GetDlgItem(hwnd, IDC_STATIC_TRANSPARENT), TEXT("悬浮窗透明度 - %d%%"), mc.mcc.dwTransparent);
		SetScrollRange(GetDlgItem(hwnd, IDC_SCROLLBAR), SB_CTL, 20, 100, FALSE);
		SetScrollPos(GetDlgItem(hwnd, IDC_SCROLLBAR), SB_CTL, mc.mcc.dwTransparent, FALSE);

		//刷新显示
		ShowWindow(hwnd, SW_SHOW);
		SendMessage(hwnd, WM_UPDATEINFO, 0, 0);
		Update();
	}
	VOID WndStruct::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
	{
		switch (id)
		{
		case IDM_EXIT:
		{
			SendMessage(MainWindow::hwndMain, WM_CLOSE, 0, 0);
			break;
		}
		case IDM_ABOUT:
		{
			AboutWindow::Create();
			break;
		}
		//
		case IDM_CLIP:
		{
			OpenClipboard(NULL);
			EmptyClipboard();
			CloseClipboard();
			break;
		}
		case IDM_EXPLORER:
		{
			auto ASK = [&](HWND hwndFound)
			{
				if (IsWindowVisible(hwndFound))
				{
					int bRet = tk::MsgBox(hwnd,
						TEXT("检测到您可能正在进行文件操作，重启资源管理器将会终止这些操作。")
						TEXT("确定要重启资源管理器吗？"),
						TEXT("提示"), MB_ICONINFORMATION | MB_OKCANCEL);
					return bRet == IDOK;
				}
				return true;
			};

			if (GetShellWindow())
			{
				if (HWND hwndFound = FindWindow(TEXT("OperationStatusWindow"), NULL))
				{
					if (!ASK(hwndFound))
						break;
				}
				else if (hwndFound = FindWindow(TEXT("#32770"), TEXT("正在复制...")))
				{
					if (!ASK(hwndFound))
						break;
				}
				else if (hwndFound = FindWindow(TEXT("#32770"), TEXT("正在移动...")))
				{
					if (!ASK(hwndFound))
						break;
				}
				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, tk::GetPIdFromWindow(GetShellWindow()));
				TerminateProcess(hProcess, 0);
				WaitForSingleObject(hProcess, 50);
				CloseHandle(hProcess);
			}
			else
			{
				tk::RunBat(TEXT("start explorer"), FALSE, FALSE);
			}
			break;
#undef ASK
		}
		case IDM_TASKMGR:
		{
			TCHAR szPath[MAX_PATH] = TEXT("start taskmgr");
			tk::RunBat(szPath, FALSE, FALSE);
			break;
		}
		case IDM_LOGOFF:
		{
			BOOL bRet = tk::MsgBox(hwnd,
				TEXT("确定要注销 Windows 用户吗？未保存的文件将会丢失。"),
				TEXT("提示"), MB_ICONINFORMATION | MB_OKCANCEL);
			if (bRet == IDCANCEL) break;
			PostMessage(MainWindow::hwndMain, WM_CLOSE, 0, 0);
			ExitWindowsEx(EWX_LOGOFF, NULL);
			break;
		}
		//
		case IDC_CHECK_SHOW:
		{
			CheckDlgButton(hwnd, IDC_CHECK_SHOW, mc.mcc.bShow = !mc.mcc.bShow);
			SEND_WM_RESETPOSITION(MainWindow::hwndMain, mc.mcc.bShow, FALSE);
			break;
		}
		case IDC_CHECK_AMINATION:
		{
			CheckDlgButton(hwnd, IDC_CHECK_AMINATION, mc.mcc.bAnimation = !mc.mcc.bAnimation);
			break;
		}

		case IDC_CHECK_AUTOLAUNCH:
		{
			CheckDlgButton(hwnd, IDC_CHECK_AUTOLAUNCH, mc.mcc.bAutoLaunch = !mc.mcc.bAutoLaunch);
			break;
		}
		case IDC_CHECK_QUICKER:
		{
			CheckDlgButton(hwnd, IDC_CHECK_QUICKER, mc.mcc.bQuicker = !mc.mcc.bQuicker);
			ns.SetSleepTime(mc.mcc.bQuicker ? 500 : 1000);
			break;
		}
		case IDC_CHECK_AUTOCLEAN:
		{
			CheckDlgButton(hwnd, IDC_CHECK_AUTOCLEAN, mc.mcc.bAutoClean = !mc.mcc.bAutoClean);
			if (mc.mcc.bAutoClean)
			{
				timerClean.Set(5000);
			}
			else
			{
				timerClean.Kill();
			}
			break;
		}
		case IDC_CHECK_AUTOCONTROL:
		{
			CheckDlgButton(hwnd, IDC_CHECK_AUTOCONTROL, mc.mcc.bAutoControl = !mc.mcc.bAutoControl);
			break;
		}

		case IDC_BUTTON_UNFOLD:
		{
			bFoldStatus = FALSE;
			ShowWindow(hwndUnfoldButton, SW_HIDE);
			Update();
			break;
		}
		case IDC_BUTTON_FOLD:
		{
			bFoldStatus = TRUE;
			ShowWindow(hwndUnfoldButton, SW_SHOW);
			Update();
			break;
		}
		default:
			break;
		}
	}
	VOID WndStruct::OnUpdateInfo(HWND hwnd)
	{
		PostMessage(GetDlgItem(hwnd, IDC_MEMPAD), WM_UPDATEINFO, 0, 0);

		TCHAR szBuffer[tk::STRING_LENGTH];
		tk::SetWindowTextFormat(GetDlgItem(hwndSetting, IDC_TEXT1), TEXT("%.1f / %.1f GiB"), ((double)msex.ullTotalPhys - msex.ullAvailPhys) / 1024 / 1024 / 1024, (double)msex.ullTotalPhys / 1024 / 1024 / 1024);
		tk::SetWindowTextFormat(GetDlgItem(hwndSetting, IDC_TEXT2), TEXT("%d / %d MiB"), (DWORD)((msex.ullTotalPageFile - msex.ullAvailPageFile) / 1024 / 1024), (DWORD)(msex.ullTotalPageFile / 1024 / 1024));
		Tool::FormatDataUnit(szBuffer, tk::STRING_LENGTH, (DWORD)ns.GetUploadTotal());
		tk::SetWindowTextFormat(GetDlgItem(hwndSetting, IDC_TEXT3), TEXT("%s"), szBuffer);
		Tool::FormatDataUnit(szBuffer, tk::STRING_LENGTH, (DWORD)ns.GetDownloadTotal());
		tk::SetWindowTextFormat(GetDlgItem(hwndSetting, IDC_TEXT4), TEXT("%s"), szBuffer);

		auto temps = nvt.get_nvgpu_temperatures();
		if (temps.empty())
			SetWindowTextW(GetDlgItem(hwndSetting, IDC_TEXT5), L"没有 NVIDIA® GPU");
		else
		{
			std::wstring text;
			text = std::to_wstring(temps[0]);
			for (size_t i = 1; i < temps.size(); i++)
				text += L" / " + std::to_wstring(temps[i]);
			text += L" °C";
			SetWindowTextW(GetDlgItem(hwndSetting, IDC_TEXT5), text.c_str());
		}
	}
	VOID WndStruct::OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
	{
		MainWindow::WndStruct& ws = *(MainWindow::WndStruct*)(GetWindowLongPtr(MainWindow::hwndMain, GWLP_USERDATA));

		DWORD& dwT = mc.mcc.dwTransparent;
		switch (code)
		{
		case SB_PAGERIGHT:
			dwT += 19;
		case SB_LINERIGHT:
			dwT = min(100, dwT + 1);
			break;
		case SB_PAGELEFT:
			dwT -= 19;
		case SB_LINELEFT:
			dwT = max(20, dwT - 1);
			break;
		case SB_LEFT:
			dwT = 20;
			break;
		case SB_RIGHT:
			dwT = 100;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			dwT = pos;
			break;
		default:
			return;
		}
		tk::SetWindowTextFormat(GetDlgItem(hwnd, IDC_STATIC_TRANSPARENT), TEXT("悬浮窗透明度 - %d%%"), dwT);
		ws.dwTransparent = dwT;
		SetScrollPos(hwndCtl, SB_CTL, mc.mcc.dwTransparent, TRUE);

		PostMessage(MainWindow::hwndMain, WM_UPDATEINFO, 0, 0);
	}
	VOID WndStruct::OnClose(HWND hwnd)
	{
		DestroyWindow(hwnd);
	}
	VOID WndStruct::OnDestroy(HWND hwnd)
	{
		hwndSetting = NULL;

		mc.WriteConfig();
		PostQuitMessage(0);
		delete this;
	}
}
LRESULT CALLBACK SettingWindow::WndStruct::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_INIT, OnInit);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hwnd, WM_UPDATEINFO, OnUpdateInfo);
		HANDLE_MSG(hwnd, WM_HSCROLL, OnHScroll);
		HANDLE_MSG(hwnd, WM_CLOSE, OnClose);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}