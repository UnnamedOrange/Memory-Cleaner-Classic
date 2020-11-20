#include "..\TKernel\TKernel.h"
#include "resource.h"
#include "Global Variable.h"

#include "Tools.h"

BOOL Tool::FormatDataUnit(LPTSTR lpText, size_t size, int64_t qwTotalByBytes)
{
	const TCHAR* szUnit[] =
	{
		TEXT("B"),
		TEXT("KiB"),
		TEXT("MiB"),
		TEXT("GiB")
	};
	double floatRes = qwTotalByBytes;
	const UINT maxSize = sizeof(szUnit) / sizeof(TCHAR*);
	UINT index = 0;
	for (index; qwTotalByBytes / 1024 && index < maxSize; index++)
	{
		qwTotalByBytes /= 1024;
		floatRes /= 1024;
	}
	UINT length = 0;
	if (index <= 1)
		length = _stprintf_s(lpText, size, TEXT("%lld %s"), qwTotalByBytes, szUnit[index]);
	else
		length = _stprintf_s(lpText, size, TEXT("%.2f %s"), floatRes, szUnit[index]);
	return length;
}
BOOL Tool::FormatSpeedUnit(LPTSTR lpText, size_t size, int64_t qwSpeedByBytes)
{
	const TCHAR* szUnit[] =
	{
		TEXT("B/s"),
		TEXT("KiB/s"),
		TEXT("MiB/s")
	};
	double floatRes = qwSpeedByBytes;
	const UINT maxSize = sizeof(szUnit) / sizeof(TCHAR*);
	UINT index = 0;
	for (index; qwSpeedByBytes / 1024 && index < maxSize; index++)
	{
		qwSpeedByBytes /= 1024;
		floatRes /= 1024;
	}
	UINT length = 0;
	if (index <= 1)
		length = _stprintf_s(lpText, size, TEXT("%lld %s"), qwSpeedByBytes, szUnit[index]);
	else
		length = _stprintf_s(lpText, size, TEXT("%.2f %s"), floatRes, szUnit[index]);
	return length;
}
BOOL Tool::FormatTimeUnit(LPTSTR lpText, size_t size, int64_t qwTimeByS)
{
	const TCHAR* szUnit[] =
	{
		TEXT("s"),
		TEXT("min"),
		TEXT("h")
	};
	const UINT maxSize = sizeof(szUnit) / sizeof(TCHAR*);
	int64_t qwTimes[maxSize] = { NULL };
	INT index = 0;
	for (index; index < maxSize - 1; index++)
	{
		qwTimes[index] = qwTimeByS % 60;
		qwTimeByS /= 60;
	}
	if (qwTimeByS) qwTimes[maxSize - 1] = qwTimeByS;

	UINT length = 0;
	for (; index >= 0; index--)
	{
		if (qwTimes[index] || length || !index)
		{
			length = _stprintf_s(lpText, size, TEXT("%s%s%lld %s"), lpText, length ? TEXT(" ") : TEXT(""), qwTimes[index], szUnit[index]);
		}
	}
	return length;
}

BOOL Tool::IsForegroundFullscreen()
{
	BOOL bRet = FALSE;
	HWND hwnd = GetForegroundWindow(); // 获取当前正在与用户交互的前台窗口句柄
	WCHAR buffer[256];
	GetClassNameW(hwnd, buffer, 256); //获取前台窗口的类名
	std::wstring name(buffer);

	if (hwnd != GetDesktopWindow() && hwnd != GetShellWindow() && name != L"WorkerW") // 如果前台窗口不是桌面窗口，也不是控制台窗口
	{
		RECT rcApp, rcDesk;
		GetWindowRect(hwnd, &rcApp); // 获取前台窗口的坐标
		GetWindowRect(GetDesktopWindow(), &rcDesk);	// 根据桌面窗口句柄，获取整个屏幕的坐标
		if (rcApp.left <= rcDesk.left && // 如果前台窗口的坐标完全覆盖住桌面窗口，就表示前台窗口是全屏的
			rcApp.top <= rcDesk.top &&
			rcApp.right >= rcDesk.right &&
			rcApp.bottom >= rcDesk.bottom)
		{
			bRet = true;
		}
	}
	return bRet;
}