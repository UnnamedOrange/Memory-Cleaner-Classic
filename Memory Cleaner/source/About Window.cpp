#include "..\TKernel\TKernel.h"
#include "resource.h"
#include "Global Variable.h"

#include "Memory Cleaner.h"
#include "Tools.h"
#include "About Window.h"

//声明
namespace AboutWindow
{
	constexpr TCHAR szCaption[] = TEXT("关于 Memory Cleaner");

	constexpr const TCHAR* szItem[] =
	{
		TEXT("关于 Memory Cleaner"),
		TEXT("性能"),
		TEXT("日志")
	};
	constexpr size_t size = sizeof(szItem) / sizeof(TCHAR*);

	constexpr const wchar_t* szText[] =
	{
		L"Author: UnnamedOrange\r\n"
		"Version: "
#ifdef _M_IX86
		"32 位"
#elif _M_X64
		"64 位"
#endif
		" %d.%d.%d.%d"
		"\r\n"
		"编译时间: %s, %s"
#ifdef _DEBUG
		"\r\n\r\n"
		"Debug Build"
#endif
		,

		L"已使用的 CPU 时间\r\n"
		"%s\r\n\r\n"
		"使用的物理内存\r\n"
		"%s",

		L""
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
						mc.MajorVerH(), mc.MajorVerL(), mc.MinorVerH(), mc.MinorVerL(),
						TEXT(__DATE__), TEXT(__TIME__));
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