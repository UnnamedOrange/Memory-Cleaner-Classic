#pragma once
#include "..\TKernel\TKernel.h"
#include "resource.h"

struct MemoryCleanerConfig
{
	BOOL bShow;
	BOOL bAnimation;
	DWORD dwTransparent;

	BOOL bAutoLaunch;
	BOOL bQuicker;
	BOOL bAutoClean;
	BOOL bAutoControl;

	int iLeft, iTop;
};

DWORD WINAPI MCThread(LPVOID lpClass);
class MemoryCleaner :public tk::App
{
public:
	MemoryCleanerConfig mcc;

	BOOL isWin7;

public:
	MemoryCleaner(LPCTSTR lpcGUID, LPCTSTR lpcAppName, INT nProcessPriority = NORMAL_PRIORITY_CLASS, INT nThreadPriority = THREAD_PRIORITY_NORMAL);
	~MemoryCleaner();

	VOID ReadConfig();
	VOID WriteConfig();
	VOID Clean();

	friend DWORD WINAPI MCThread(LPVOID lpClass);
	DWORD OnThread();

	INT_PTR InitializeResource(HINSTANCE hInst, LPTSTR szCmdLine, int iCmdShow) override;
	INT_PTR ReleaseResource(HINSTANCE hInst, LPTSTR szCmdLine, int iCmdShow) override;
	INT_PTR RegisterClasses(HINSTANCE hInst, LPTSTR szCmdLine, int iCmdShow) override;
	INT_PTR InitializeInstance(HINSTANCE hInst, LPTSTR szCmdLine, int iCmdShow) override;
	INT_PTR Execute(HINSTANCE hInst, LPTSTR szCmdLine, int iCmdShow) override;
};