#include "..\TKernel\TKernel.h"
#include "resource.h"
#include "Global Variable.h"

#include "Memory Cleaner.h"
#include "Tools.h"
#include "About Window.h"

//声明
namespace AboutWindow
{
	const TCHAR szCaption[] = TEXT("关于 Memory Cleaner");

	const TCHAR* szItem[] =
	{
		TEXT("关于 Memory Cleaner"),
		TEXT("性能"),
		TEXT("日志")
	};
	const size_t size = sizeof(szItem) / sizeof(TCHAR*);

	const TCHAR* szText[] =
	{
		TEXT
		(
			"Orange Software\r\n"
			"版本 %d.%d.%d.%d"
		)
#ifdef _M_IX86
		L"\r\n" // always Unicode
		L"32 位"
#elif _M_X64
		L"\r\n" // always Unicode
		L"64 位"
#endif
#ifndef _RELEASE
		TEXT
		(
			"\r\n\r\n"
			"Internal Version"
		)
#endif
		L"\r\n\r\n" // always Unicode
		L"致用户（好像只有我）：\r\n"
		L"        感谢您使用 Memory Cleaner。随着时间的推移，我不得不承认我年少时犯的错误："
		L"小时候写的代码太难维护啦！我不得不做出一个艰难的决定：这是 Memory Cleaner 3 的最后一次更新。"
		L"本次更新增加了 64 位的支持。更新的初衷本是想看看是什么 bug 导致了全屏模式下没有隐藏悬浮窗，"
		L"没想到居然是因为没有编译！！！不管怎么说，我自己都已经使用 Memory Cleaner 至少 4 年了，"
		L"感谢大家的支持与陪伴（雾）！新版本，将会在不久的将来推出吼……"
		,
		TEXT
		(
			"已使用的 CPU 时间\r\n"
			"%s\r\n\r\n"
			"使用的物理内存\r\n"
			"%s"
		),
		TEXT
		(
			""
		)
	};
}

//杂项
VOID AboutWindow::Create()
{
	DialogBox(HINST, MAKEINTRESOURCE(IDD_ABOUT), SettingWindow::hwndSetting, DlgProc);
}

//消息
namespace AboutWindow
{
	BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
	{
		SetWindowText(hwnd, szCaption);
		SendMessage(hwnd, WM_SETICON, NULL, LPARAM(GetIconLarge()));

		HWND hwndComb = GetDlgItem(hwnd, IDC_COMBO);
		for (size_t i = 0; i < size; i++)
		{
			ComboBox_InsertItemData(hwndComb, i, szItem[i]);
		}

		ComboBox_SetCurSel(hwndComb, 0);
		FORWARD_WM_COMMAND(hwnd, IDC_COMBO, hwndComb, CBN_SELCHANGE, DlgProc);
		return TRUE;
	}
	VOID OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
	{
		switch (id)
		{
		case IDOK:
		case IDCANCEL:
		{
			EndDialog(hwnd, 0);
			break;
		}

		case IDC_COMBO:
		{
			if (codeNotify == CBN_SELCHANGE)
			{
				HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT);
				HWND hwndButtonDel = GetDlgItem(hwnd, IDC_BUTTON_DELETE);
				HWND hwndButtonRefresh = GetDlgItem(hwnd, IDC_BUTTON_REFRESH);
				INT sel = ComboBox_GetCurSel(hwndCtl);

				if (sel == 0)
					ShowWindow(hwndButtonRefresh, SW_HIDE);
				else
					ShowWindow(hwndButtonRefresh, SW_SHOW);

				if (sel == 2)
					ShowWindow(hwndButtonDel, SW_SHOW);
				else
					ShowWindow(hwndButtonDel, SW_HIDE);

				switch (sel)
				{
				case 0:
				{
					tk::SetWindowTextFormat(hwndEdit, szText[sel],
						mc.MajorVerH(), mc.MajorVerL(), mc.MinorVerH(), mc.MinorVerL());
					break;
				}
				case 1:
				{
					FILETIME Temp, KernelTime, UserTime;
					GetProcessTimes(GetCurrentProcess(), &Temp, &Temp, &KernelTime, &UserTime);
					int64_t CpuTime = (((int64_t(UserTime.dwHighDateTime) << 32ll) + UserTime.dwLowDateTime) +
						((int64_t(KernelTime.dwHighDateTime) << 32ll) + KernelTime.dwLowDateTime)) / 10000000;
					TCHAR szTime[tk::STRING_LENGTH] = { NULL };
					Tool::FormatTimeUnit(szTime, tk::STRING_LENGTH, CpuTime);

					PROCESS_MEMORY_COUNTERS pmc;
					GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
					TCHAR szMemory[tk::STRING_LENGTH] = { NULL };
					Tool::FormatDataUnit(szMemory, tk::STRING_LENGTH, pmc.PagefileUsage);

					tk::SetWindowTextFormat(hwndEdit, szText[sel], szTime, szMemory);
					break;
				}
				case 2:
				{
					if (tk::IsFileValid(szLogPath))
					{
						HANDLE hFile = CreateFile(szLogPath, GENERIC_READ, FILE_SHARE_READ,
							NULL, OPEN_EXISTING, NULL, NULL);
						DWORD dwSize = GetFileSize(hFile, nullptr);
						PCHAR pStr = PCHAR(calloc(dwSize + 1, sizeof(CHAR)));
						DWORD dwRead = NULL;
						ReadFile(hFile, pStr, dwSize, &dwRead, nullptr);
						CloseHandle(hFile);
						SetWindowTextA(hwndEdit, pStr);
						free(pStr);
					}
					else
					{
						tk::SetWindowTextFormat(hwndEdit, TEXT("没有找到日志文件"));
					}
					break;
				}
				default:
					break;
				}
			}
			break;
		}

		case IDC_BUTTON_COPY:
		{
			HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT);
			INT iLength = GetWindowTextLengthW(hwndEdit);
			PWCHAR pGlobal = PWCHAR(GlobalAllocPtr(GHND | GMEM_SHARE, sizeof(WCHAR) * (iLength + 1)));
			GetWindowTextW(hwndEdit, pGlobal, iLength + 1);
			GlobalUnlockPtr(pGlobal);
			OpenClipboard(hwnd);
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, GlobalPtrHandle(pGlobal));
			CloseClipboard();
			break;
		}
		case IDC_BUTTON_REFRESH:
		{
			FORWARD_WM_COMMAND(hwnd, IDC_COMBO, GetDlgItem(hwnd, IDC_COMBO), CBN_SELCHANGE, DlgProc);
			break;
		}
		case IDC_BUTTON_DELETE:
		{
			DeleteFile(szLogPath);
			FORWARD_WM_COMMAND(hwnd, IDC_COMBO, GetDlgItem(hwnd, IDC_COMBO), CBN_SELCHANGE, DlgProc);
			break;
		}
		default:
			break;
		}
	}
}
INT_PTR CALLBACK AboutWindow::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_DLGMSG(hwnd, WM_INITDIALOG, OnInitDialog);
		HANDLE_DLGMSG(hwnd, WM_COMMAND, OnCommand);
	default:
		break;
	}
	return FALSE;
}