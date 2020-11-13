#include "..\TKernel\TKernel.h"
#include "resource.h"
#include "Global Variable.h"

#include "Memory Cleaner.h"
#include "Net Speed.h"
#include "Tools.h"
#include "Main Window.h"
#include "Setting Window.h"
#include "Mempad.h"
#include "Text.h"

//Information
#ifdef _DEBUG
const TCHAR szGUID[] = TEXT("5CB3ADF3-C482-4D95-A494-DFCADC4ECB55");
#else
const TCHAR szGUID[] = TEXT("B5708F25-B087-407A-B7E4-399FC8C595F5");
#endif
const TCHAR szAppName[] = TEXT("Memory Cleaner");

//App Class
MemoryCleaner mc(szGUID, szAppName);
NetSpeed ns;
tk::GDIP LoadGDIP;
MEMORYSTATUSEX msex = { sizeof(MEMORYSTATUSEX) };

//Extend classes
tk::PrivateFontPlus PrivateFont;
tk::DPIHelper dpi;

//Others
TCHAR szLogPath[MAX_PATH];

VOID CALLBACK Update(HWND hwnd, UINT message, UINT_PTR iTimerId, DWORD dwTime);
tk::Timer timerUpdate(Update);
VOID CALLBACK Update(HWND hwnd, UINT message, UINT_PTR iTimerId, DWORD dwTime)
{
	timerUpdate.Set(mc.mcc.bQuicker ? 500 : 1000);
	GlobalMemoryStatusEx(&msex);
	if (mc.mcc.bShow)
	{
		PostMessage(MainWindow::hwndMain, WM_UPDATEINFO, 0, 0);
	}
	if (SettingWindow::hwndSetting)
	{
		PostMessage(SettingWindow::hwndSetting, WM_UPDATEINFO, 0, 0);
	}
}

VOID CALLBACK CleanTimer(HWND hwnd, UINT message, UINT_PTR iTimerId, DWORD dwTime);
tk::Timer timerClean(CleanTimer);
VOID CALLBACK CleanTimer(HWND hwnd, UINT message, UINT_PTR iTimerId, DWORD dwTime)
{
	if (msex.dwMemoryLoad >= 80) timerClean.Set(10000);
	else if (msex.dwMemoryLoad >= 50) timerClean.Set(30000);
	else timerClean.Set(60000);
	mc.Clean();
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int iCmdShow)
{
	if (mc.SingleInstance())
	{
		SendMessage(FindWindow(MainWindow::szClassName, MainWindow::szCaption), WM_FINDINSTANCE, 0, 0);
		return 0;
	}
	return mc.Run(hInstance, szCmdLine, iCmdShow);
}

MemoryCleaner::MemoryCleaner(LPCTSTR lpcGUID, LPCTSTR lpcAppName, INT nProcessPriority, INT nThreadPriority) : tk::App(szGUID, szAppName/*, nProcessPriority, nThreadPriority*/)
{
	ReadConfig();
}
MemoryCleaner::~MemoryCleaner()
{
	WriteConfig();
}
VOID MemoryCleaner::ReadConfig()
{
	if (!RegReadAppCfg((LPVOID)&mcc, sizeof(MemoryCleanerConfig)))
	{
		mcc.bShow = TRUE;
		mcc.bAnimation = TRUE;
		mcc.dwTransparent = 60;
		mcc.bQuicker = FALSE;
		mcc.bAutoClean = FALSE;
		mcc.bAutoControl = TRUE;
		mcc.iLeft = 0xffff;
		mcc.iTop = 0xffff;
	}
	mcc.bAutoLaunch = RegGetAutoBoot();

	TCHAR szPath[2][MAX_PATH];
	GetModuleFileName(hInstance, szPath[0], MAX_PATH);
	_stprintf_s(szPath[1], TEXT("\"%s\" Launch"), szPath[0]);
	if (RegGetAutoBoot(szPath[1]))
	{
		mcc.bAutoLaunch = TRUE;
	}
	else
	{
		mcc.bAutoLaunch = FALSE;
		RegSetAutoBoot(FALSE);
	}
}
VOID MemoryCleaner::WriteConfig()
{
	RegWriteAppCfg((LPVOID)&mcc, sizeof(MemoryCleanerConfig));

	TCHAR szPath[2][MAX_PATH];
	GetModuleFileName(hInstance, szPath[0], MAX_PATH);
	_stprintf_s(szPath[1], TEXT("\"%s\" Launch"), szPath[0]);
	if (mcc.bAutoLaunch)
	{
		RegSetAutoBoot(TRUE, szPath[1]);
	}
	else
	{
		RegSetAutoBoot(FALSE);
	}
}
VOID MemoryCleaner::Clean()
{
	tk::CreateThreadEz(MCThread, (LPVOID)this);
}
DWORD MemoryCleaner::OnThread()
{
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		BOOL bMore = Process32First(hSnapshot, &pe32);
		while (bMore)
		{
			for (size_t i = 0; i < 1; i++)
			{
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
				//Your Codes for the Processes here.
				if (mcc.bAutoControl)
				{
					PROCESS_MEMORY_COUNTERS pmc;
					GetProcessMemoryInfo(hProcess, &pmc, sizeof(PROCESS_MEMORY_COUNTERS));
					FILETIME Temp, UserTime, KernelTime;
					GetProcessTimes(hProcess, &Temp, &Temp, &KernelTime, &UserTime);
					int64_t CpuTime = (((int64_t(UserTime.dwHighDateTime) << 32ll) + UserTime.dwLowDateTime) +
						((int64_t(KernelTime.dwHighDateTime) << 32ll) + KernelTime.dwLowDateTime)) / 10000000;

					if (double(pmc.WorkingSetSize) / 1024 / 1024 >= 32 /*MB*/)
					{

					}
					else if (CpuTime >= 60 * 1000 /*ms*/)
					{

					}
					else if (CpuTime >= 10 * 1000 /*ms*/)
					{
						SetProcessWorkingSetSize(hProcess, -1, -1);
					}
					else if (double(pmc.WorkingSetSize) / 1024 / 1024 >= 1 /*MB*/)
					{
						SetProcessWorkingSetSize(hProcess, -1, -1);
					}
				}
				else SetProcessWorkingSetSize(hProcess, -1, -1);

				//[End]
				CloseHandle(hProcess);
			}
			bMore = Process32Next(hSnapshot, &pe32);
		}
		CloseHandle(hSnapshot);
	}
	return 0;
}
DWORD WINAPI MCThread(LPVOID lpClass)
{
	return ((MemoryCleaner*)lpClass)->OnThread();
}
INT_PTR MemoryCleaner::InitializeResource(HINSTANCE hInst, LPTSTR szCmdLine, int iCmdShow)
{
	GetTempPath(MAX_PATH, szLogPath);
	_tcscat_s(szLogPath, TEXT("Memory Cleaner.log"));
	PrivateFont.Init(hInstance, MAKEINTRESOURCE(IDR_TTF), TEXT("TTF"));
	ReadConfig();
	tk::WriteLogFile(TEXT("Launch"), TEXT("Launched Memory Cleaner."), szLogPath);

	MainWindow::bLaunch = !_tcsicmp(szCmdLine, TEXT("Launch"));
	Text::colorBk = GetSysColor(COLOR_BTNFACE);

	OSVERSIONINFO osver;
	tk::GetVersion(&osver);
	isWin7 = osver.dwMajorVersion == 6 && osver.dwMinorVersion == 1;

	ns.SetSleepTime(mc.mcc.bQuicker ? 500 : 1000);
	timerUpdate.Set(0);
	timerClean.Set(0);
	return 0;
}
INT_PTR MemoryCleaner::ReleaseResource(HINSTANCE hInst, LPTSTR szCmdLine, int iCmdShow)
{
	WriteConfig();
	tk::WriteLogFile(TEXT("Exit"), TEXT("Memory Cleaner exits normally."), szLogPath);
	return 0;
}
INT_PTR MemoryCleaner::RegisterClasses(HINSTANCE hInst, LPTSTR szCmdLine, int iCmdShow)
{
	BOOL bFail = 0;
	bFail |= !MainWindow::RegisterClasses();
	bFail |= !SettingWindow::RegisterClasses();
	bFail |= !Mempad::RegisterClasses();
	bFail |= !Text::RegisterClasses();
	return bFail;
}
INT_PTR MemoryCleaner::InitializeInstance(HINSTANCE hInst, LPTSTR szCmdLine, int iCmdShow)
{
	return MainWindow::InitInstance();
}
INT_PTR MemoryCleaner::Execute(HINSTANCE hInst, LPTSTR szCmdLine, int iCmdShow)
{
	return MainWindow::MsgLoop(NULL);
}