#include "TKernel.h"

VOID tk::Debug::DebugString(LPCTSTR lpOutputString, ...)
{
	TCHAR szBuffer[STRING_LENGTH * STRING_LENGTH] = { NULL };

	va_list pArgList;
	va_start(pArgList, lpOutputString);
	_vsntprintf_s(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), lpOutputString, pArgList);
	va_end(pArgList);

	OutputDebugString(TEXT("DebugString: "));
	OutputDebugString(szBuffer);
	OutputDebugString(TEXT("\n"));
}
BOOL tk::Debug::WriteLogFile(LPCTSTR lpcTitle, LPCTSTR lpcText, LPCTSTR lpcFileName, ...)
{
	std::_tstring title;
	std::_tstring text;
	std::_tstring filename;

	if (lpcTitle)
	{
		title = TEXT("[");
		title += lpcTitle;
		title += TEXT("] ");
	}
	else
	{
		title = TEXT("");
	}
	if (lpcText)
	{
		TCHAR szBuffer[STRING_LENGTH * STRING_LENGTH];
		va_list pArgList;
		va_start(pArgList, lpcFileName);
		_vsntprintf_s(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), lpcText, pArgList);
		va_end(pArgList);
		text = szBuffer;
	}
	else
	{
		text = TEXT("");
	}
	if (lpcFileName)
	{
		filename = lpcFileName;
	}
	else
	{
		TCHAR szBuffer[STRING_LENGTH * STRING_LENGTH];
		tk::System::GetDesktopPath(szBuffer);
		_tcscat_s(szBuffer, TEXT("\\LogFile.log"));
		filename = szBuffer;
	}

	tk::ComStr cs;
	SYSTEMTIME st;
	GetLocalTime(&st);
	TCHAR szBuffer[STRING_LENGTH * STRING_LENGTH];
	DWORD dwLength = _stprintf_s(szBuffer, TEXT("%02d:%02d:%02d.%03d %s %d, %d\t%s%s"),
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, cs.Month_En_s(st.wMonth - 1), st.wDay,
		st.wYear, title.c_str(), text.c_str());
	tk::File::WriteMultiBytes(filename.c_str(), szBuffer, dwLength);
	tk::File::WriteMultiBytes(filename.c_str(), "\r\n");
	return 0;
}

BOOL tk::App::bUsedApplicationClass; // 只允许有一个 Application 的实例
tk::App::App(LPCTSTR szGUID, LPCTSTR szAppName) :hInstance(HINSTANCE(&__ImageBase)), hMutex(NULL), ffi({ NULL })
{
	if (bUsedApplicationClass)
		std::exception();
	bUsedApplicationClass = TRUE;

	strGUID = szGUID;
	strAppName = szAppName;

	InitApp();
}
tk::App::~App()
{
	RevokeSingleInstance();
}
BOOL tk::App::InitApp()
{
	tk::Process::EnableDebugPrivilege();

	TCHAR szPath[MAX_PATH];
	GetModuleFileName(hInstance, szPath, MAX_PATH);
	tk::File::GetFileVersion(szPath, &ffi);
	return TRUE;
}
BOOL tk::App::GetAppName(LPTSTR szBuffer, size_t nSize)
{
	if (!szBuffer) return FALSE;
	size_t iLength = strAppName.length();
	if (iLength + 1 > nSize) return FALSE;
	_tcscpy_s(szBuffer, nSize, strAppName.c_str());
	return TRUE;
}
BOOL tk::App::GetGUID(LPTSTR szBuffer, size_t nSize)
{
	if (!szBuffer) return FALSE;
	size_t iLength = strGUID.length();
	if (iLength + 1 > nSize) return FALSE;
	_tcscpy_s(szBuffer, nSize, strGUID.c_str());
	return TRUE;
}
DWORD tk::App::MajorVerH()
{
	return HIWORD(ffi.dwFileVersionMS);
}
DWORD tk::App::MajorVerL()
{
	return LOWORD(ffi.dwFileVersionMS);
}
DWORD tk::App::MinorVerH()
{
	return HIWORD(ffi.dwFileVersionLS);
}
DWORD tk::App::MinorVerL()
{
	return LOWORD(ffi.dwFileVersionLS);
}
DWORD tk::App::RegReadAppCfg(LPCTSTR lpcValueName, DWORD dwDefault)
{
	TCHAR szSubKey[MAX_PATH] = { 0 };
	HKEY hKey = NULL;

	if (!lpcValueName)
		return FALSE;
	_stprintf_s(szSubKey, TEXT("Software\\%s"), strAppName.c_str());

	DWORD dwTemp, dwLength;
	RegOpenKeyEx(HKEY_CURRENT_USER, szSubKey, 0, KEY_READ, &hKey);
	if (RegQueryValueEx(hKey, lpcValueName, 0, NULL, (LPBYTE)&dwTemp, &dwLength))
		if (RegQueryValueEx(hKey, lpcValueName, 0, NULL, (LPBYTE)&dwTemp, &dwLength))
			dwTemp = dwDefault;
	RegCloseKey(hKey);

	return dwTemp;
}
LSTATUS tk::App::RegWriteAppCfg(LPCTSTR lpcValueName, DWORD dwValue, BOOL bDelete)
{
	TCHAR szSubKey[MAX_PATH] = { 0 };
	HKEY hKey = NULL;

	if (!lpcValueName)
		return FALSE;
	_stprintf_s(szSubKey, TEXT("Software\\%s"), strAppName.c_str());

	LSTATUS lsResult = NULL;
	if (bDelete)
	{
		RegOpenKeyEx(HKEY_CURRENT_USER, szSubKey, 0, KEY_WRITE, &hKey);
		lsResult = RegDeleteValue(hKey, lpcValueName);
	}
	else
	{
		RegCreateKeyEx(HKEY_CURRENT_USER, szSubKey, NULL, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
		lsResult = RegSetValueEx(hKey, lpcValueName, 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(DWORD));
	}
	RegCloseKey(hKey);
	return lsResult;
}
BOOL tk::App::RegReadAppCfg(LPVOID lpConfig, size_t cbSize, LPCTSTR lpcValueName)
{
	TCHAR szSubKey[MAX_PATH] = TEXT("");
	TCHAR szValueName[MAX_PATH] = TEXT("config");
	_stprintf_s(szSubKey, TEXT("Software\\%s"), strAppName.c_str());
	if (lpcValueName)
		_tcscpy_s(szValueName, lpcValueName);

	return tk::RegGetKey(HKEY_CURRENT_USER, szSubKey, szValueName, lpConfig, cbSize);
}
BOOL tk::App::RegWriteAppCfg(LPCVOID lpcConfig, size_t cbSize, LPCTSTR lpcValueName)
{
	TCHAR szSubKey[MAX_PATH] = TEXT("");
	TCHAR szValueName[MAX_PATH] = TEXT("config");
	_stprintf_s(szSubKey, TEXT("Software\\%s"), strAppName.c_str());
	if (lpcValueName)
		_tcscpy_s(szValueName, lpcValueName);

	return tk::RegSetKey(HKEY_CURRENT_USER, szSubKey, szValueName, REG_BINARY, (CONST BYTE*)lpcConfig, cbSize);
}
BOOL tk::App::RegSetAutoBoot(BOOL bSet, LPCTSTR lpcPath)
{
	TCHAR szPath[MAX_PATH] = { 0 };
	HKEY hKey = NULL;
	LSTATUS lsResult = NULL;

	if (!lpcPath)
	{
		if (!GetModuleFileName(hInstance, szPath, MAX_PATH))
			return FALSE;
		tk::Name::AddQuotes(szPath, MAX_PATH);
	}
	else
	{
		_tcscpy_s(szPath, MAX_PATH, lpcPath);
	}

	if (bSet)
	{
		RegCreateKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), NULL, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
		lsResult = RegSetValueEx(hKey, strAppName.c_str(), 0, REG_SZ, (const BYTE*)szPath, sizeof(TCHAR) * (_tcslen(szPath) + 1));
		RegCloseKey(hKey);
	}
	else
	{
		RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_WRITE, &hKey);
		lsResult = RegDeleteValue(hKey, strAppName.c_str());
		RegCloseKey(hKey);
	}
	return lsResult;
}
BOOL tk::App::RegGetAutoBoot(LPCTSTR lpcPath)
{
	TCHAR szPath[MAX_PATH] = { 0 };
	HKEY hKey = NULL;
	BOOL bResult = NULL;
	DWORD dwSize = NULL;

	if (!lpcPath)
	{
		if (!GetModuleFileName(hInstance, szPath, MAX_PATH))
			return FALSE;
		tk::Name::AddQuotes(szPath, MAX_PATH);
	}
	else
	{
		_tcscpy_s(szPath, MAX_PATH, lpcPath);
	}

	RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_READ, &hKey);
	if (RegQueryValueEx(hKey, strAppName.c_str(), 0, NULL, NULL, &dwSize))
	{
		return FALSE;
	}
	TCHAR* szTemp = (TCHAR*)malloc(dwSize);
	if (RegQueryValueEx(hKey, strAppName.c_str(), 0, NULL, (LPBYTE)szTemp, &dwSize))
	{
		if (szTemp) free(szTemp);
		return FALSE;
	}
	RegCloseKey(hKey);

	bResult = !_tcsicmp(szPath, szTemp);
	free(szTemp);
	return bResult;
}
BOOL tk::App::ResourceToFile(LPCTSTR lpcName, LPCTSTR lpcType, LPCTSTR lpcFileName)
{
	BOOL result = TRUE;
	HRSRC hResInfo = FindResource(hInstance, lpcName, lpcType);
	HGLOBAL hResource = LoadResource(hInstance, hResInfo);
	DWORD dwSize = SizeofResource(hInstance, hResInfo);
	LPVOID p = LockResource(hResource);

	HANDLE hFile = CreateFile(lpcFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	DWORD dwWritten;
	WriteFile(hFile, p, dwSize, &dwWritten, NULL);
	CloseHandle(hFile);

	FreeResource(hResource);

	if (!hResInfo || !hResource || !dwSize || !dwWritten)
		result = FALSE;
	return result;
}
BOOL tk::App::About(HWND hwnd, LPCTSTR lpcOtherStuff, HICON hIcon, ...)
{
	if (!lpcOtherStuff)
		return 0;

	TCHAR   szBuffer[STRING_LENGTH * STRING_LENGTH];
	va_list pArgList;
	va_start(pArgList, hIcon);
	_vsntprintf_s(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), lpcOtherStuff, pArgList);
	va_end(pArgList);

	return ShellAbout(hwnd, strAppName.c_str(), szBuffer, hIcon);
}
BOOL tk::App::SingleInstance()
{
	if (hMutex) return FALSE;
	hMutex = CreateMutex(NULL, TRUE, strGUID.c_str());
	BOOL bResult = GetLastError() == ERROR_ALREADY_EXISTS;
	if (bResult)
	{
		CloseHandle(hMutex);
		hMutex = NULL;
		return TRUE;
	}
	return FALSE;
}
BOOL tk::App::RevokeSingleInstance()
{
	if (!hMutex) return FALSE;
	CloseHandle(hMutex);
	hMutex = NULL;
	return TRUE;
}
INT_PTR tk::App::InitializeResource(HINSTANCE hInst, LPTSTR szCmdLine, INT iCmdShow)
{
	return 0;
}
INT_PTR tk::App::ReleaseResource(HINSTANCE hInst, LPTSTR szCmdLine, INT iCmdShow)
{
	return 0;
}
INT_PTR tk::App::RegisterClasses(HINSTANCE hInst, LPTSTR szCmdLine, INT iCmdShow)
{
	return 0;
}
INT_PTR tk::App::InitializeInstance(HINSTANCE hInst, LPTSTR szCmdLine, INT iCmdShow)
{
	return 0;
}
INT_PTR tk::App::Execute(HINSTANCE hInst, LPTSTR szCmdLine, INT iCmdShow)
{
	return 0;
}
INT_PTR tk::App::Run(HINSTANCE hInst, LPTSTR szCmdLine, INT iCmdShow)
{
	if (InitializeResource(hInst, szCmdLine, iCmdShow)) return 1;
	if (RegisterClasses(hInst, szCmdLine, iCmdShow)) return 1;
	if (InitializeInstance(hInst, szCmdLine, iCmdShow)) return 1;
	if (Execute(hInst, szCmdLine, iCmdShow)) return 1;
	if (ReleaseResource(hInst, szCmdLine, iCmdShow)) return 1;
	return 0;
}
BOOL tk::MsgBox(HWND hwnd, LPCTSTR lpcText, LPCTSTR lpcCaption, UINT uType, ...)
{
	if (!lpcText)
		return 0;
	if (!lpcCaption)
		return 0;

	TCHAR   szBuffer[STRING_LENGTH * STRING_LENGTH];
	va_list pArgList;
	va_start(pArgList, uType);
	_vsntprintf_s(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), lpcText, pArgList);
	va_end(pArgList);

	return MessageBox(hwnd, szBuffer, lpcCaption, uType);
}

tk::DllApp::DllApp(LPCTSTR szGUID, LPCTSTR szAppName) :hInstance(HINSTANCE(&__ImageBase))
{
	strGUID = szGUID;
	strAppName = szAppName;

	tk::Process::EnableDebugPrivilege();
	InitApp();
}
tk::DllApp::~DllApp()
{
}
BOOL tk::DllApp::InitApp()
{
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(hInstance, szPath, MAX_PATH);
	tk::File::GetFileVersion(szPath, &ffi);
	return TRUE;
}
BOOL tk::DllApp::GetAppName(LPTSTR szBuffer, size_t nSize)
{
	if (!szBuffer) return FALSE;
	size_t iLength = strAppName.length();
	if (iLength + 1 > nSize) return FALSE;
	_tcscpy_s(szBuffer, nSize, strAppName.c_str());
	return TRUE;
}
BOOL tk::DllApp::GetGUID(LPTSTR szBuffer, size_t nSize)
{
	if (!szBuffer) return FALSE;
	size_t iLength = strGUID.length();
	if (iLength + 1 > nSize) return FALSE;
	_tcscpy_s(szBuffer, nSize, strGUID.c_str());
	return TRUE;
}
DWORD tk::DllApp::MajorVerH()
{
	return HIWORD(ffi.dwFileVersionMS);
}
DWORD tk::DllApp::MajorVerL()
{
	return LOWORD(ffi.dwFileVersionMS);
}
DWORD tk::DllApp::MinorVerH()
{
	return HIWORD(ffi.dwFileVersionLS);
}
DWORD tk::DllApp::MinorVerL()
{
	return LOWORD(ffi.dwFileVersionLS);
}
DWORD tk::DllApp::RegReadAppCfg(LPCTSTR lpcValueName, DWORD dwDefault)
{
	TCHAR szSubKey[MAX_PATH] = { 0 };
	HKEY hKey = NULL;

	if (!lpcValueName)
		return FALSE;
	_stprintf_s(szSubKey, TEXT("Software\\%s"), strAppName.c_str());

	DWORD dwTemp, dwLength;
	RegOpenKeyEx(HKEY_CURRENT_USER, szSubKey, 0, KEY_READ, &hKey);
	if (RegQueryValueEx(hKey, lpcValueName, 0, NULL, (LPBYTE)&dwTemp, &dwLength))
		if (RegQueryValueEx(hKey, lpcValueName, 0, NULL, (LPBYTE)&dwTemp, &dwLength))
			dwTemp = dwDefault;
	RegCloseKey(hKey);

	return dwTemp;
}
LSTATUS tk::DllApp::RegWriteAppCfg(LPCTSTR lpcValueName, DWORD dwValue, BOOL bDelete)
{
	TCHAR szSubKey[MAX_PATH] = { 0 };
	HKEY hKey = NULL;

	if (!lpcValueName)
		return FALSE;
	_stprintf_s(szSubKey, TEXT("Software\\%s"), strAppName.c_str());

	LSTATUS lsResult = NULL;
	if (bDelete)
	{
		RegOpenKeyEx(HKEY_CURRENT_USER, szSubKey, 0, KEY_WRITE, &hKey);
		lsResult = RegDeleteValue(hKey, lpcValueName);
	}
	else
	{
		RegCreateKeyEx(HKEY_CURRENT_USER, szSubKey, NULL, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
		lsResult = RegSetValueEx(hKey, lpcValueName, 0, REG_DWORD, (const BYTE*)&dwValue, sizeof(DWORD));
	}
	RegCloseKey(hKey);
	return lsResult;
}
BOOL tk::DllApp::RegReadAppCfg(LPVOID lpConfig, size_t cbSize, LPCTSTR lpcValueName)
{
	TCHAR szSubKey[MAX_PATH] = TEXT("");
	TCHAR szValueName[MAX_PATH] = TEXT("config");
	_stprintf_s(szSubKey, TEXT("Software\\%s"), strAppName.c_str());
	if (lpcValueName)
		_tcscpy_s(szValueName, lpcValueName);

	return tk::RegGetKey(HKEY_CURRENT_USER, szSubKey, szValueName, lpConfig, cbSize);
}
BOOL tk::DllApp::RegWriteAppCfg(LPCVOID lpcConfig, size_t cbSize, LPCTSTR lpcValueName)
{
	TCHAR szSubKey[MAX_PATH] = TEXT("");
	TCHAR szValueName[MAX_PATH] = TEXT("config");
	_stprintf_s(szSubKey, TEXT("Software\\%s"), strAppName.c_str());
	if (lpcValueName)
		_tcscpy_s(szValueName, lpcValueName);

	return tk::RegSetKey(HKEY_CURRENT_USER, szSubKey, szValueName, REG_BINARY, (CONST BYTE*)lpcConfig, cbSize);
}
BOOL tk::DllApp::ResourceToFile(LPCTSTR lpcName, LPCTSTR lpcType, LPCTSTR lpcFileName)
{
	BOOL result = TRUE;
	HRSRC hResInfo = FindResource(hInstance, lpcName, lpcType);
	HGLOBAL hResource = LoadResource(hInstance, hResInfo);
	DWORD dwSize = SizeofResource(hInstance, hResInfo);
	LPVOID p = LockResource(hResource);

	HANDLE hFile = CreateFile(lpcFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	DWORD dwWritten;
	WriteFile(hFile, p, dwSize, &dwWritten, NULL);
	CloseHandle(hFile);

	FreeResource(hResource);

	if (!hResInfo || !hResource || !dwSize || !dwWritten)
		result = FALSE;
	return result;
}
BOOL tk::DllApp::About(HWND hwnd, LPCTSTR lpcOtherStuff, HICON hIcon, ...)
{
	if (!lpcOtherStuff)
		return 0;

	TCHAR   szBuffer[STRING_LENGTH * STRING_LENGTH];
	va_list pArgList;
	va_start(pArgList, hIcon);
	_vsntprintf_s(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), lpcOtherStuff, pArgList);
	va_end(pArgList);

	return ShellAbout(hwnd, strAppName.c_str(), szBuffer, hIcon);
}
INT_PTR tk::DllApp::InitializeResource(HINSTANCE hInst)
{
	return 0;
}
INT_PTR tk::DllApp::ReleaseResource(HINSTANCE hInst)
{
	return 0;
}
INT_PTR tk::DllApp::RegisterClasses(HINSTANCE hInst)
{
	return 0;
}
INT_PTR tk::DllApp::InitializeInstance(HINSTANCE hInst)
{
	return 0;
}
INT_PTR tk::DllApp::Execute(HINSTANCE hInst)
{
	return 0;
}
INT_PTR tk::DllApp::Run(HINSTANCE hInst)
{
	if (InitializeResource(hInst)) return 1;
	if (RegisterClasses(hInst)) return 1;
	if (InitializeInstance(hInst)) return 1;
	if (Execute(hInst)) return 1;
	if (ReleaseResource(hInst)) return 1;
	return 0;
}

tk::GDIP::GDIP()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}
tk::GDIP::~GDIP()
{
	Gdiplus::GdiplusShutdown(gdiplusToken);
}
#ifdef UNICODE
BOOL tk::GDIP::DrawStringShadow(Gdiplus::Graphics& graphics, LPCWSTR lpcText, const Gdiplus::Font* font, const Gdiplus::PointF& origin, const Gdiplus::Brush* brush)
{
	INT length = wcslen(lpcText);
	Gdiplus::SolidBrush brush_B(Gdiplus::Color(255, 0, 0, 0));
	graphics.DrawString(lpcText, length, font, Gdiplus::PointF(origin.X + 1, origin.Y), &brush_B);
	graphics.DrawString(lpcText, length, font, Gdiplus::PointF(origin.X - 1, origin.Y), &brush_B);
	graphics.DrawString(lpcText, length, font, Gdiplus::PointF(origin.X, origin.Y + 1), &brush_B);
	graphics.DrawString(lpcText, length, font, Gdiplus::PointF(origin.X, origin.Y - 1), &brush_B);
	graphics.DrawString(lpcText, length, font, Gdiplus::PointF(origin.X + 1, origin.Y + 1), &brush_B);
	graphics.DrawString(lpcText, length, font, Gdiplus::PointF(origin.X + 1, origin.Y + 2), &brush_B);
	graphics.DrawString(lpcText, length, font, Gdiplus::PointF(origin.X + 2, origin.Y + 1), &brush_B);
	graphics.DrawString(lpcText, length, font, Gdiplus::PointF(origin.X + 2, origin.Y + 2), &brush_B);
	return graphics.DrawString(lpcText, length, font, origin, brush);
}
BOOL tk::GDIP::DrawStringShadow(Gdiplus::Graphics& graphics, LPCWSTR lpcText, const Gdiplus::Font* font, const Gdiplus::RectF& layoutRect, const Gdiplus::StringFormat* stringFormat, const Gdiplus::Brush* brush)
{
	INT length = wcslen(lpcText);
	Gdiplus::SolidBrush brush_B(Gdiplus::Color(255, 63, 63, 63));
	const INT n = 12;
	Gdiplus::RectF rects[n];
	for (int i = 0; i < n; i++) rects[i] = layoutRect;
	rects[0].Offset(1, 0);
	rects[1].Offset(-1, 0);
	rects[2].Offset(0, 1);
	rects[3].Offset(0, -1);
	rects[4].Offset(1, 1);
	//rects[5].Offset(1, 2);
	//rects[6].Offset(2, 1);
	//rects[7].Offset(2, 2);
	rects[8].Offset(-1, -1);
	//rects[9].Offset(-1, -2);
	//rects[10].Offset(-2, -1);
	//rects[11].Offset(-2, -2);
	for (int i = 0; i < n; i++)
	{
		graphics.DrawString(lpcText, length, font, rects[i], stringFormat, &brush_B);
	}
	return graphics.DrawString(lpcText, length, font, layoutRect, stringFormat, brush);
}
#endif

tk::PrivateFont::PrivateFont() : fHandle(NULL), lpFont(nullptr), dwSize(NULL)
{
}
tk::PrivateFont::~PrivateFont()
{
	Unload();
}
VOID tk::PrivateFont::Unload()
{
	if (!lpFont) return;

	RemoveFontMemResourceEx(fHandle);
	dwSize = NULL;
	free(lpFont);
	lpFont = nullptr;
}
BOOL tk::PrivateFont::Init(HINSTANCE hInst, LPCTSTR lpcName, LPCTSTR lpcType)
{
	if (lpFont) Unload();

	HRSRC hResInfo = FindResource(hInst, lpcName, lpcType);
	if (!hResInfo) return FALSE;
	HGLOBAL hResource = LoadResource(hInst, hResInfo);
	if (!hResource) return FALSE;
	dwSize = SizeofResource(hInst, hResInfo);
	lpFont = malloc(dwSize);
	LPVOID pTemp = LockResource(hResource);
	CopyMemory(lpFont, pTemp, dwSize);
	FreeResource(hResource);

	DWORD num = NULL;
	fHandle = AddFontMemResourceEx(lpFont, dwSize, NULL, &num);
	return num;
}
BOOL tk::PrivateFont::Init(LPCTSTR lpcFileName)
{
	if (lpFont) Unload();

	HANDLE hFile = CreateFile(lpcFileName, FILE_READ_ACCESS, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hFile) return 0;
	dwSize = GetFileSize(hFile, NULL);
	lpFont = malloc(dwSize);
	DWORD dwRead = NULL;
	ReadFile(hFile, lpFont, dwSize, &dwRead, NULL);
	CloseHandle(hFile);

	DWORD num = NULL;
	fHandle = AddFontMemResourceEx(lpFont, dwSize, NULL, &num);
	return num;
}

tk::PrivateFontPlus::~PrivateFontPlus()
{
	Unload();
}
VOID tk::PrivateFontPlus::Unload()
{
	if (!pfc) return;
	delete pfc;
	pfc = nullptr;
	tk::PrivateFont::Unload();
}
BOOL tk::PrivateFontPlus::Init(HINSTANCE hInst, LPCTSTR lpcName, LPCTSTR lpcType)
{
	if (lpFont) Unload();
	tk::PrivateFont::Init(hInst, lpcName, lpcType);
	pfc = new Gdiplus::PrivateFontCollection;
	return pfc->AddMemoryFont(lpFont, dwSize);
}
BOOL tk::PrivateFontPlus::Init(LPCTSTR lpcFileName)
{
	if (lpFont) Unload();
	tk::PrivateFont::Init(lpcFileName);
	pfc = new Gdiplus::PrivateFontCollection();
	return pfc->AddMemoryFont(lpFont, dwSize);
	pfc->GetFamilyCount();
}
INT tk::PrivateFontPlus::GetFamilyCount()
{
	return pfc->GetFamilyCount();
}
INT tk::PrivateFontPlus::GetFamilies(INT numSought, Gdiplus::FontFamily* gpfamilies, INT* numFound)
{
	return pfc->GetFamilies(numSought, gpfamilies, numFound);
}

tk::ComStr::ComStr()
{
	mes[0] = _tcsdup(TEXT("Jan."));
	mes[1] = _tcsdup(TEXT("Feb."));
	mes[2] = _tcsdup(TEXT("Mar."));
	mes[3] = _tcsdup(TEXT("Apr."));
	mes[4] = _tcsdup(TEXT("May"));
	mes[5] = _tcsdup(TEXT("June"));
	mes[6] = _tcsdup(TEXT("July"));
	mes[7] = _tcsdup(TEXT("Aug."));
	mes[8] = _tcsdup(TEXT("Sept."));
	mes[9] = _tcsdup(TEXT("Oct."));
	mes[10] = _tcsdup(TEXT("Nov."));
	mes[11] = _tcsdup(TEXT("Dec."));

	mef[0] = _tcsdup(TEXT("January"));
	mef[1] = _tcsdup(TEXT("February"));
	mef[2] = _tcsdup(TEXT("March"));
	mef[3] = _tcsdup(TEXT("April"));
	mef[4] = _tcsdup(TEXT("May"));
	mef[5] = _tcsdup(TEXT("June"));
	mef[6] = _tcsdup(TEXT("July"));
	mef[7] = _tcsdup(TEXT("August"));
	mef[8] = _tcsdup(TEXT("September"));
	mef[9] = _tcsdup(TEXT("October"));
	mef[10] = _tcsdup(TEXT("November"));
	mef[11] = _tcsdup(TEXT("December"));

	mc[0] = _tcsdup(TEXT("一月"));
	mc[1] = _tcsdup(TEXT("二月"));
	mc[2] = _tcsdup(TEXT("三月"));
	mc[3] = _tcsdup(TEXT("四月"));
	mc[4] = _tcsdup(TEXT("五月"));
	mc[5] = _tcsdup(TEXT("六月"));
	mc[6] = _tcsdup(TEXT("七月"));
	mc[7] = _tcsdup(TEXT("八月"));
	mc[8] = _tcsdup(TEXT("九月"));
	mc[9] = _tcsdup(TEXT("十月"));
	mc[10] = _tcsdup(TEXT("十一月"));
	mc[11] = _tcsdup(TEXT("十二月"));

	des[0] = _tcsdup(TEXT("Sun."));
	des[1] = _tcsdup(TEXT("Mon."));
	des[2] = _tcsdup(TEXT("Tues."));
	des[3] = _tcsdup(TEXT("Wed."));
	des[4] = _tcsdup(TEXT("Thur."));
	des[5] = _tcsdup(TEXT("Fri."));
	des[6] = _tcsdup(TEXT("Sat."));

	def[0] = _tcsdup(TEXT("Sunday"));
	def[1] = _tcsdup(TEXT("Monday"));
	def[2] = _tcsdup(TEXT("Tuesday"));
	def[3] = _tcsdup(TEXT("Wednesday"));
	def[4] = _tcsdup(TEXT("Thurday"));
	def[5] = _tcsdup(TEXT("Friday"));
	def[6] = _tcsdup(TEXT("Saturday"));

	dc[0] = _tcsdup(TEXT("周日"));
	dc[1] = _tcsdup(TEXT("周一"));
	dc[2] = _tcsdup(TEXT("周二"));
	dc[3] = _tcsdup(TEXT("周三"));
	dc[4] = _tcsdup(TEXT("周四"));
	dc[5] = _tcsdup(TEXT("周五"));
	dc[6] = _tcsdup(TEXT("周六"));
}
tk::ComStr::~ComStr()
{
	for (int i = 0; i < 12; i++)
	{
		if (mes[i]) free(mes[i]);
		if (mef[i]) free(mef[i]);
		if (mc[i]) free(mc[i]);
	}
	for (int i = 0; i < 7; i++)
	{
		if (des[i]) free(des[i]);
		if (def[i]) free(def[i]);
		if (dc[i]) free(dc[i]);
	}
}
LPCTSTR tk::ComStr::Month_En_s(INT index)
{
	if (index >= 0 && index < 12) return mes[index];
	return nullptr;
}
LPCTSTR tk::ComStr::Month_En_f(INT index)
{
	if (index >= 0 && index < 12) return mef[index];
	return nullptr;
}
LPCTSTR tk::ComStr::Month_Cn(INT index)
{
	if (index >= 0 && index < 12) return mc[index];
	return nullptr;
}
LPCTSTR tk::ComStr::DayOfWeek_En_s(INT index)
{
	if (index >= 0 && index < 7) return des[index];
	return nullptr;
}
LPCTSTR tk::ComStr::DayOfWeek_En_f(INT index)
{
	if (index >= 0 && index < 7) return def[index];
	return nullptr;
}
LPCTSTR tk::ComStr::DayOfWeek_Cn(INT index)
{
	if (index >= 0 && index < 7) return dc[index];
	return nullptr;
}

tk::Timer::Timer(TIMERPROC pProc) : tp(nullptr), hWnd(NULL), iTimerId(NULL), bSet(FALSE)
{
	SetProc(pProc);
}
tk::Timer::~Timer()
{
	Kill();
}
VOID tk::Timer::SetProc(TIMERPROC pProc)
{
	if (bSet) Kill();
	tp = pProc;
}
UINT tk::Timer::Set(UINT uElapse)
{
	if (!tp) return NULL;
	Kill();

	bSet = TRUE;
	iTimerId = SetTimer(NULL, NULL, uElapse, tp);

	if (!uElapse) tp(NULL, WM_TIMER, iTimerId, GetTickCount());
	return iTimerId;
}
UINT tk::Timer::Set(HWND hwnd, UINT_PTR nId_Event, UINT uElapse)
{
	if (!tp) return NULL;
	Kill();

	hWnd = hwnd;
	iTimerId = nId_Event;
	bSet = TRUE;
	SetTimer(hwnd, nId_Event, uElapse, tp);

	if (!uElapse) tp(hwnd, WM_TIMER, iTimerId, GetTickCount());
	return iTimerId;
}
VOID tk::Timer::Kill()
{
	if (bSet)
	{
		KillTimer(hWnd, iTimerId);
		hWnd = NULL;
		iTimerId = NULL;
		bSet = FALSE;
	}
}

tk::WndPos::WndPos(HWND hwnd) :
	bSysDock(NULL), isWin10(NULL),
	iLeft(NULL), iTop(NULL), iWidth(NULL), iHeight(NULL),
	minTrackSize{ NULL },
	bDock(FALSE), LastNCMsg(NULL), bNCMouseDown(FALSE),
	pMouse{ NULL }, rectOrigin{ NULL }, isDocking(NULL), bMaximize(FALSE),
	bInScreen(FALSE), bBar(FALSE), rectWnd{ NULL }
{
	OSVERSIONINFO os;
	tk::GetVersion(&os);
	isWin10 = os.dwMajorVersion == 10;
	SystemParametersInfo(SPI_GETWINARRANGING, NULL, &bSysDock, NULL);
	SetHwnd(hwnd);
}
HWND tk::WndPos::SetHwnd(HWND hwnd)
{
	return hWnd = hwnd;
}
VOID tk::WndPos::SetMinTrackSize(INT cx, INT cy)
{
	minTrackSize = { cx, cy };
}
VOID tk::WndPos::SetDock(BOOL bSet)
{
	bDock = bSet;
}
VOID tk::WndPos::SetInScreen(BOOL bSet)
{
	bInScreen = bSet;
}
LRESULT tk::WndPos::VirtualProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (hWnd != hwnd) return 0;

	switch (message)
	{
	case WM_GETMINMAXINFO:
	{
		if (minTrackSize.x && minTrackSize.y)
		{
			LPMINMAXINFO(lParam)->ptMinTrackSize = minTrackSize;
		}
		break;
	}

	//更新信息
	case WM_SETTINGCHANGE:
	{
		if (wParam == SPI_SETWINARRANGING)
		{
			SystemParametersInfo(SPI_GETWINARRANGING, NULL, &bSysDock, NULL);
		}
		break;
	}
	case WM_NCLBUTTONDOWN:
	{
		LastNCMsg = wParam;
		if (!bSysDock)
		{
			if (bMaximize)
				bNCMouseDown = TRUE;
		}
		//禁止窗口移出屏幕的信息
		GetWindowRect(hwnd, &rectWnd);
		GetCursorPos(&pMouse);
		break;
	}
	case WM_NCLBUTTONUP:
	{
		if (!bSysDock)
		{
			bNCMouseDown = FALSE;
		}
		break;
	}
	case WM_NCLBUTTONDBLCLK:
	{
		if (!bSysDock)
		{
			bNCMouseDown = FALSE;
		}
		break;
	}
	case WM_MOVE:
	{
		iLeft = GET_X_LPARAM(lParam);
		iTop = GET_Y_LPARAM(lParam);
		break;
	}
	case WM_SIZE:
	{
		iWidth = GET_X_LPARAM(lParam);
		iHeight = GET_Y_LPARAM(lParam);
		//停靠窗口的信息
		if (!bSysDock)
		{
			bMaximize = IsZoomed(hwnd);
			if (bMaximize)
			{
				bNCMouseDown = FALSE;
				isDocking = 3;
			}
			else if (isDocking == 3)
			{
				isDocking = 0;
			}
		}
		break;
	}

	//处理最大化
	case WM_NCMOUSEMOVE:
	{
		if (!bSysDock)
		{
			if (bDock && LastNCMsg == HTCAPTION && bNCMouseDown && isDocking == 3)
			{
				bNCMouseDown = FALSE;
				//准备还原
				POINT p; GetCursorPos(&p);
				RECT r; GetWindowRect(hwnd, &r);

				//计算新的窗口位置
				INT nWidthNew = rectOrigin.right - rectOrigin.left;
				INT nHeightNew = rectOrigin.bottom - rectOrigin.top;
				INT nXMouse1 = p.x - r.left;
				INT nXMouse2 = r.right - p.x;
				INT nXMouse = 0; //鼠标相对于窗口的横坐标
				INT nYMouse = p.y - r.top; //鼠标相对于窗口的纵坐标
				//根据停靠的位置来计算最终鼠标相对于窗口的横坐标
				if (nXMouse1 < nWidthNew / 2)
				{
					nXMouse = nXMouse1;
				}
				else if (nXMouse2 < nWidthNew / 2 && nXMouse2 < nWidthNew)
				{
					nXMouse = nWidthNew - nXMouse2;
				}
				else
				{
					nXMouse = double(nXMouse1) * double(nWidthNew) / double(r.right - r.left);
				}
				MoveWindow(hwnd, p.x - nXMouse, p.y - nYMouse, nWidthNew, nHeightNew, TRUE);
				ShowWindow(hwnd, SW_NORMAL);
				MoveWindow(hwnd, p.x - nXMouse, p.y - nYMouse, nWidthNew, nHeightNew, TRUE);

				SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, NULL);
			}
		}
		break;
	}

	//更新窗口大小
	case WM_MOVING:
	{
		RECT& r = *(RECT*)lParam;
		POINT p;
		GetCursorPos(&p);
		//恢复停靠前的大小
		if (!bSysDock)
		{
			if (isDocking)
			{
				INT nWidthOrigin = r.right - r.left;
				//INT nHeightOrigin = r.bottom - r.top;
				INT nWidthNew = rectOrigin.right - rectOrigin.left;
				INT nHeightNew = rectOrigin.bottom - rectOrigin.top;
				INT nXMouse1 = p.x - r.left;
				INT nXMouse2 = r.right - p.x;
				INT nXMouse = 0; //鼠标相对于窗口的横坐标
				INT nYMouse = p.y - r.top; //鼠标相对于窗口的纵坐标
										   //根据停靠的位置来计算最终鼠标相对于窗口的横坐标
				if (isDocking == 1)
				{
					if (nXMouse1 < nWidthNew / 2)
					{
						nXMouse = nXMouse1;
					}
					else if (nXMouse2 < nWidthNew / 2 && nXMouse2 < nWidthNew)
					{
						nXMouse = nWidthNew - nXMouse2;
					}
					else
					{
						nXMouse = double(nXMouse1) * double(nWidthNew) / double(nWidthOrigin);
					}
				}
				else if (isDocking == 2)
				{
					if (nXMouse2 < nWidthNew / 2 && nXMouse2 < nWidthNew)
					{
						nXMouse = nWidthNew - nXMouse2;
					}
					else if (nXMouse1 < nWidthNew / 2)
					{
						nXMouse = nXMouse1;
					}
					else
					{
						nXMouse = double(nXMouse1) * double(nWidthNew) / double(nWidthOrigin);
					}
				}
				//修改窗口最终的坐标
				r.left = p.x - nXMouse;
				r.top = p.y - nYMouse;
				r.right = r.left + nWidthNew;
				r.bottom = r.top + nHeightNew;
				isDocking = FALSE; //没有停靠了
			}
		}
		//设置禁止移出屏幕
		if (bInScreen)
		{
			RECT rWorkArea = tk::Windows::GetWorkArea(); //获取工作区域
			INT nWidth = r.right - r.left;
			INT nHeight = r.bottom - r.top; //获取窗口大小
			rWorkArea.right -= nWidth - 1; //将窗口可能出现的位置更改为窗口左上角的点可能出现的位置
			rWorkArea.bottom -= nHeight - 1;

			r.left = rectWnd.left + (p.x - pMouse.x); //根据鼠标位置初步得出窗口左上角位置
			r.top = rectWnd.top + (p.y - pMouse.y);

			POINT pCorner = { r.left, r.top }; //窗口左上角的点
											   //调整窗口左上角位置
			if (pCorner.x < rWorkArea.left)
			{
				r.left = rWorkArea.left;
			}
			if (pCorner.y < rWorkArea.top)
			{
				r.top = rWorkArea.top;
			}
			if (pCorner.x > rWorkArea.right)
			{
				r.left = rWorkArea.right;
			}
			if (pCorner.y > rWorkArea.bottom)
			{
				r.top = rWorkArea.bottom;
			}
			r.right = r.left + nWidth; //根据左上角位置和大小调整右下角
			r.bottom = r.top + nHeight;
		}
		break;
	}

	//停靠窗口
	case WM_EXITSIZEMOVE:
	{
		if (!bSysDock)
		{
			if (bDock && LastNCMsg == HTCAPTION)
			{
				POINT p;
				GetCursorPos(&p);

				RECT rWorkArea = tk::GetWorkArea();
				GetWindowRect(hwnd, &rectOrigin); //保存原窗口大小
				if (isWin10)
				{
					POINT pClient = { 0, 0 };
					ClientToScreen(hwnd, &pClient);
					rWorkArea.left -= pClient.x - rectOrigin.left - 1;
					RECT rClient;
					GetClientRect(hwnd, &rClient);
					pClient.x = rClient.right - rClient.left;
					pClient.y = rClient.bottom - rClient.top;
					ClientToScreen(hwnd, &pClient);
					rWorkArea.right += rectOrigin.right - pClient.x;
					rWorkArea.bottom += rectOrigin.bottom - pClient.y;
				}
				else
				{
					rWorkArea.right += 2;
				}

				const INT critical = 15;
				if (p.x - rWorkArea.left <= critical && p.y - rWorkArea.top <= critical) //左上
				{
					INT nWidth = (rWorkArea.right - rWorkArea.left) / 2;
					INT nHeight = (rWorkArea.bottom - rWorkArea.top) / 2;
					if (minTrackSize.x && minTrackSize.y)
					{
						nWidth = max(nWidth, minTrackSize.x);
						nHeight = max(nHeight, minTrackSize.y);
					}
					SetWindowPos(hwnd, NULL, rWorkArea.left, rWorkArea.top,
						nWidth, nHeight, SWP_NOZORDER);
					isDocking = 1;
				}
				else if (rWorkArea.right - p.x <= critical && p.y - rWorkArea.top <= critical) //右上
				{
					INT nWidth = (rWorkArea.right - rWorkArea.left) / 2;
					INT nHeight = (rWorkArea.bottom - rWorkArea.top) / 2;
					if (minTrackSize.x && minTrackSize.y)
					{
						nWidth = max(nWidth, minTrackSize.x);
						nHeight = max(nHeight, minTrackSize.y);
					}
					SetWindowPos(hwnd, NULL, rWorkArea.right - nWidth,
						rWorkArea.top,
						nWidth, nHeight, SWP_NOZORDER);
					isDocking = 2;
				}
				else if (p.x - rWorkArea.left <= critical && rWorkArea.bottom - p.y <= critical) //左下
				{
					INT nWidth = (rWorkArea.right - rWorkArea.left) / 2;
					INT nHeight = (rWorkArea.bottom - rWorkArea.top) / 2;
					if (minTrackSize.x && minTrackSize.y)
					{
						nWidth = max(nWidth, minTrackSize.x);
						nHeight = max(nHeight, minTrackSize.y);
					}
					SetWindowPos(hwnd, NULL, rWorkArea.left,
						rWorkArea.bottom - nHeight,
						nWidth, nHeight, SWP_NOZORDER);
					isDocking = 1;
				}
				else if (rWorkArea.right - p.x <= critical && rWorkArea.bottom - p.y <= critical) //右下
				{
					INT nWidth = (rWorkArea.right - rWorkArea.left) / 2;
					INT nHeight = (rWorkArea.bottom - rWorkArea.top) / 2;
					if (minTrackSize.x && minTrackSize.y)
					{
						nWidth = max(nWidth, minTrackSize.x);
						nHeight = max(nHeight, minTrackSize.y);
					}
					SetWindowPos(hwnd, NULL, rWorkArea.right - nWidth,
						rWorkArea.bottom - nHeight,
						nWidth, nHeight, SWP_NOZORDER);
					isDocking = 2;
				}
				else if (p.x - rWorkArea.left <= critical) //左
				{
					INT nWidth = (rWorkArea.right - rWorkArea.left) / 2;
					INT nHeight = rWorkArea.bottom - rWorkArea.top;
					if (minTrackSize.x && minTrackSize.y)
					{
						nWidth = max(nWidth, minTrackSize.x);
						nHeight = max(nHeight, minTrackSize.y);
					}
					SetWindowPos(hwnd, NULL, rWorkArea.left,
						rWorkArea.top,
						nWidth, nHeight, SWP_NOZORDER);
					isDocking = 1;
				}
				else if (rWorkArea.right - p.x <= critical) //右
				{
					INT nWidth = (rWorkArea.right - rWorkArea.left) / 2;
					INT nHeight = rWorkArea.bottom - rWorkArea.top;
					if (minTrackSize.x && minTrackSize.y)
					{
						nWidth = max(nWidth, minTrackSize.x);
						nHeight = max(nHeight, minTrackSize.y);
					}
					SetWindowPos(hwnd, NULL, rWorkArea.right - nWidth,
						rWorkArea.top,
						nWidth, nHeight, SWP_NOZORDER);
					isDocking = 2;
				}
				else if (p.y - rWorkArea.top <= critical) //上
				{
					bNCMouseDown = FALSE;
					ShowWindow(hwnd, SW_MAXIMIZE);
				}
			}
		}
	}
	default:
		break;
	}
	return 0;
}

tk::Tray::Tray(HWND hwnd) :nid({ sizeof(NOTIFYICONDATA) }), is_Inited(FALSE), WM_TASKBARCREATED(RegisterWindowMessage(TEXT("TaskbarCreated")))
{
	Init(hwnd);
}
tk::Tray::~Tray()
{
	if (is_Inited == 2)
	{
		Shell_NotifyIcon(NIM_DELETE, &nid);
	}
}
VOID tk::Tray::Init(HWND hwnd)
{
	if (!hwnd) return;
	nid.hWnd = hwnd;
	nid.uCallbackMessage = WM_TRAY;
	is_Inited = 1;
}
VOID tk::Tray::Add()
{
	if (is_Inited < 2) return;
	nid.uFlags = NIF_MESSAGE | //将会收到WM_TRAY消息
		NIF_ICON |  //将会用到图标
		NIF_TIP;  //将会使用到提示文字
	Shell_NotifyIcon(NIM_ADD, &nid);
}
VOID tk::Tray::Add(HICON hIcon, LPCTSTR lpcInfo)
{
	if (is_Inited < 1) return;
	nid.uFlags = NIF_MESSAGE | //将会收到WM_TRAY消息
		NIF_ICON |  //将会用到图标
		NIF_TIP;  //将会使用到提示文字
	nid.hIcon = hIcon;
	_tcscpy_s(nid.szTip, lpcInfo);
	Shell_NotifyIcon(NIM_ADD, &nid);
	is_Inited = 2;
}
VOID tk::Tray::Delete()
{
	if (is_Inited < 2) return;
	nid.uFlags = NIF_MESSAGE | //将会收到WM_TRAY消息
		NIF_ICON |  //将会用到图标
		NIF_TIP;  //将会使用到提示文字
	Shell_NotifyIcon(NIM_DELETE, &nid);
	is_Inited = 1;
}
BOOL tk::Tray::Balloon(LPCTSTR lpcText, LPCTSTR lpcTitle, DWORD dwInfoFlags, HICON hIconBalloon)
{
	_tcscpy_s(nid.szInfo, lpcText);
	_tcscpy_s(nid.szInfoTitle, lpcTitle);
	nid.dwInfoFlags = dwInfoFlags;
	nid.hBalloonIcon = hIconBalloon;
	nid.uFlags = NIF_MESSAGE | //将会收到WM_TRAY消息
		NIF_ICON |  //将会用到图标
		NIF_TIP | //将会使用到提示文字
		NIF_INFO //气球
		;
	return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

tk::ComDlg::ComDlg()
{
	ofn.lStructSize = sizeof(OPENFILENAME);		//结构大小
	ofn.hwndOwner = NULL;						//对话框拥有者，可为空
	ofn.hInstance = NULL;						//使用系统对话框保持为NULL
	ofn.lpstrFilter = nullptr;					//筛选器，可为空，为空快捷方式不指向。结尾为两个\0，各项用\0分隔
	ofn.lpstrCustomFilter = nullptr;			//保持空，用户自定义过滤器
	ofn.nMaxCustFilter = NULL;					//因lpstrCustomFilter为NULL而保持NULL
	ofn.nFilterIndex = NULL;					//过滤器索引，从1开始，0表示用户自定义过滤器
	ofn.lpstrFile = nullptr;					//获取的文件名的缓冲，多个文件用\0(OFN_EXPLORER)或者空格(!OFN_EXPLORER)分割，若缓冲太小，前两个字节保存了需要的缓冲区大小
	ofn.nMaxFile = NULL;						//缓冲区长度，至少256
	ofn.lpstrFileTitle = nullptr;				//仅包含文件名或扩展名，不含路径名的缓冲，可为NULL
	ofn.nMaxFileTitle = NULL;					//缓冲区长度
	ofn.lpstrInitialDir = nullptr;				//初始目录
	ofn.lpstrTitle = nullptr;					//对话框标题，可为空，默认为保存，打开
	ofn.Flags = NULL;							//标记
	//OFN_ALLOWMULTISELECT	多文件
	//OFN_CREATEPROMPT		提醒是否新建
	//OFN_OVERWRITEPROMPT	提醒是否覆盖
	//OFN_ENABLESIZING		允许调整大小，仅限OFN_EXPLORER
	//OFN_EXPLORER			打开和另存为默认风格
	//OFN_FILEMUSTEXIST		文件必须存在
	ofn.nFileOffset = NULL;						//文件名相对路径名的开始下标，对于多文件对话框，其值为第一个路径的对应值
	ofn.nFileExtension = NULL;					//扩展名相对路径名的开始下标，同上
	ofn.lpstrDefExt = NULL;						//默认扩展名，不加.，仅前三个字符有效
	ofn.lCustData = NULL;						//用于钩子程序，不用
	ofn.lpfnHook = NULL;						//用于钩子程序，不用
	ofn.lpTemplateName;							//用于自定义对话框，不用
	ofn.pvReserved = NULL;						//保留
	ofn.dwReserved = NULL;						//保留
	ofn.FlagsEx = NULL;							//扩展标记
	//OFN_EX_NOPLACESBAR
}
tk::ComDlg::~ComDlg()
{
}
BOOL tk::ComDlg::OpenFileDlg(LPTSTR lpFile, DWORD nMaxFile, HWND hwndOwner, LPCTSTR lpcTitle, LPCTSTR lpcFilter, DWORD dwFilterIndex, LPCTSTR lpcDefPath, LPTSTR lpFileTitle, DWORD nMaxFileTitle, DWORD Flags)
{
	ofn.lpstrFile = lpFile;
	ofn.nMaxFile = nMaxFile;
	ofn.hwndOwner = hwndOwner;
	ofn.lpstrTitle = lpcTitle;
	ofn.lpstrFilter = lpcFilter;
	ofn.nFilterIndex = dwFilterIndex;
	ofn.lpstrInitialDir = lpcDefPath;
	ofn.lpstrFileTitle = lpFileTitle;
	ofn.nMaxFileTitle = nMaxFileTitle;
	ofn.Flags = Flags;
	return GetOpenFileName(&ofn);
}
BOOL tk::ComDlg::SaveFileDlg(LPTSTR lpFile, DWORD nMaxFile, HWND hwndOwner, LPCTSTR lpcTitle, LPCTSTR lpcFilter, DWORD dwFilterIndex, LPCTSTR lpcDefExt, LPCTSTR lpcDefPath, LPTSTR lpFileTitle, DWORD nMaxFileTitle, DWORD Flags)
{
	ofn.lpstrFile = lpFile;
	ofn.nMaxFile = nMaxFile;
	ofn.hwndOwner = hwndOwner;
	ofn.lpstrTitle = lpcTitle;
	ofn.lpstrFilter = lpcFilter;
	ofn.nFilterIndex = dwFilterIndex;
	ofn.lpstrDefExt = lpcDefExt;
	ofn.lpstrInitialDir = lpcDefPath;
	ofn.lpstrFileTitle = lpFileTitle;
	ofn.nMaxFileTitle = nMaxFileTitle;
	ofn.Flags = Flags;
	return GetSaveFileName(&ofn);
}

tk::TextFile::TextFile(LPCTSTR lpcFileName) :lpFileName(nullptr)
{
	SetFileName(lpcFileName);
}
tk::TextFile::~TextFile()
{
}
VOID tk::TextFile::SetFileName(LPCTSTR lpcFileName)
{
	if (!lpcFileName) return;
	if (lpFileName) free(lpFileName);
	lpFileName = _tcsdup(lpcFileName);
}
BOOL tk::TextFile::ReadFile(HWND hwnd)
{
	BYTE   bySwap;
	DWORD  dwBytesRead;
	HANDLE hFile;
	INT    i, iFileLength, iUniTest;
	PBYTE  pBuffer, pText, pConv;

	// Open the file.

	if (INVALID_HANDLE_VALUE ==
		(hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ,
			NULL, OPEN_EXISTING, 0, NULL)))
		return FALSE;

	// Get file size in bytes and allocate memory for read.
	// Add an extra two bytes for zero termination.

	iFileLength = GetFileSize(hFile, NULL);
	pBuffer = (PBYTE)malloc(iFileLength + 2);

	// Read file and put terminating zeros at end.

	::ReadFile(hFile, pBuffer, iFileLength, &dwBytesRead, NULL);
	CloseHandle(hFile);
	pBuffer[iFileLength] = '\0';
	pBuffer[iFileLength + 1] = '\0';

	// Test to see if the text is Unicode

	iUniTest = IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE;

	if (IsTextUnicode(pBuffer, iFileLength, &iUniTest))
	{
		pText = pBuffer + 2;
		iFileLength -= 2;

		if (iUniTest & IS_TEXT_UNICODE_REVERSE_SIGNATURE)
		{
			for (i = 0; i < iFileLength / 2; i++)
			{
				bySwap = ((BYTE*)pText)[2 * i];
				((BYTE*)pText)[2 * i] = ((BYTE*)pText)[2 * i + 1];
				((BYTE*)pText)[2 * i + 1] = bySwap;
			}
		}

		// Allocate memory for possibly converted string

		pConv = (PBYTE)malloc(iFileLength + 2);

		// If the edit control is not Unicode, convert Unicode text to 
		// non-Unicode (ie, in general, wide character).

#ifndef UNICODE
		WideCharToMultiByte(CP_ACP, 0, (PWSTR)pText, -1, (LPSTR)pConv,
			iFileLength + 2, NULL, NULL);

		// If the edit control is Unicode, just copy the string
#else
		lstrcpy((PTSTR)pConv, (PTSTR)pText);
#endif

	}
	else      // the file is not Unicode
	{
		pText = pBuffer;

		// Allocate memory for possibly converted string.

		pConv = (PBYTE)malloc(2 * iFileLength + 2);

		// If the edit control is Unicode, convert ASCII text.

#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pText, -1, (PTSTR)pConv,
			iFileLength + 1);

		// If not, just copy buffer
#else
		lstrcpy((PTSTR)pConv, (PTSTR)pText);
#endif
	}

	::SetWindowText(hwnd, (PTSTR)pConv);
	free(pBuffer);
	free(pConv);

	return TRUE;
}
BOOL tk::TextFile::ReadFile(LPTSTR lpString, size_t nMaxLength)
{
	BYTE bySwap;
	DWORD dwBytesRead;
	HANDLE hFile;
	INT i, iFileLength, iUniTest;
	PBYTE pBuffer, pText, pConv;

	if (!lpFileName) return FALSE;
	if ((hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE) return FALSE;
	iFileLength = GetFileSize(hFile, NULL);
	pBuffer = (PBYTE)malloc(iFileLength + 2);

	::ReadFile(hFile, pBuffer, iFileLength, &dwBytesRead, NULL);
	CloseHandle(hFile);
	pBuffer[iFileLength] = '\0';
	pBuffer[iFileLength + 1] = '\0';

	iUniTest = IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE;

	if (IsTextUnicode(pBuffer, iFileLength, &iUniTest))
	{
		pText = pBuffer + 2;
		iFileLength -= 2;

		if (iUniTest & IS_TEXT_UNICODE_REVERSE_SIGNATURE)
		{
			for (i = 0; i < iFileLength / 2; i++)
			{
				bySwap = ((BYTE*)pText)[2 * i];
				((BYTE*)pText)[2 * i] = ((BYTE*)pText)[2 * i + 1];
				((BYTE*)pText)[2 * i + 1] = bySwap;
			}
		}
		pConv = (PBYTE)malloc(iFileLength + 2);

#ifndef UNICODE
		WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pText, -1, (LPSTR)pConv, iFileLength + 2, NULL, NULL);
#else
		_tcscpy_s((PTSTR)pConv, iFileLength + 2, (PTSTR)pText);
#endif

	}
	else
	{
		pText = pBuffer;
		pConv = (PBYTE)malloc(2 * iFileLength + 2);

#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, (LPSTR)pText, -1, (PTSTR)pConv, iFileLength + 1);
#else
		_tcscpy_s((PTSTR)pConv, 2 * iFileLength + 2, (PTSTR)pText);
#endif

	}
	_tcscpy_s(lpString, nMaxLength, (LPCTSTR)pConv);
	free(pBuffer);
	free(pConv);
	return TRUE;
}
BOOL tk::TextFile::WriteFile(HWND hwnd)
{
	DWORD  dwBytesWritten;
	HANDLE hFile;
	INT    iLength;
	PTSTR  pstrBuffer;
	WORD   wByteOrderMark = 0xFEFF;

	// Open the file, creating it if necessary

	if (INVALID_HANDLE_VALUE ==
		(hFile = CreateFile(lpFileName, GENERIC_WRITE, 0,
			NULL, CREATE_ALWAYS, 0, NULL)))
		return FALSE;

	// Get the number of characters in the edit control and allocate
	// memory for them.

	iLength = GetWindowTextLength(hwnd);
	pstrBuffer = (PTSTR)malloc((iLength + 1) * sizeof(TCHAR));

	if (!pstrBuffer)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	// If the edit control will return Unicode text, write the
	// byte order mark to the file.

#ifdef UNICODE
	::WriteFile(hFile, &wByteOrderMark, 2, &dwBytesWritten, NULL);
#endif

	// Get the edit buffer and write that out to the file.

	GetWindowText(hwnd, pstrBuffer, iLength + 1);
	::WriteFile(hFile, pstrBuffer, iLength * sizeof(TCHAR),
		&dwBytesWritten, NULL);

	if ((iLength * sizeof(TCHAR)) != (int)dwBytesWritten)
	{
		CloseHandle(hFile);
		free(pstrBuffer);
		return FALSE;
	}

	CloseHandle(hFile);
	free(pstrBuffer);

	return TRUE;
}
BOOL tk::TextFile::WriteFile(LPCTSTR lpcString)
{
	DWORD dwBytesWritten;
	HANDLE hFile;
	INT iLength;
	PTSTR pstrBuffer;
	WORD wByteOrderMark = 0xFEFF;

	if (!lpFileName) return FALSE;
	if ((hFile = CreateFile(lpcString, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL)) == INVALID_HANDLE_VALUE) return FALSE;

	iLength = _tcslen(lpcString);
	pstrBuffer = (PTSTR)malloc((iLength + 1) * sizeof(TCHAR));
	if (!pstrBuffer)
	{
		CloseHandle(hFile);
		return FALSE;
	}

#ifdef UNICODE
	::WriteFile(hFile, &wByteOrderMark, 2, &dwBytesWritten, NULL);
#endif
	::WriteFile(hFile, lpcString, iLength * sizeof(TCHAR),
		&dwBytesWritten, NULL);
	if ((iLength * sizeof(TCHAR)) != (int)dwBytesWritten)
	{
		CloseHandle(hFile);
		free(pstrBuffer);
		return FALSE;
	}

	CloseHandle(hFile);
	free(pstrBuffer);
	return TRUE;
}

INT tk::System::cxS()
{
	return GetSystemMetrics(SM_CXSCREEN);
}
INT tk::System::cyS()
{
	return GetSystemMetrics(SM_CYSCREEN);
}
BOOL tk::System::GetDesktopPath(LPTSTR lpPath) //没有尾随\\//
{
	if (!lpPath)
		return FALSE;
	return SHGetSpecialFolderPath(NULL, lpPath, CSIDL_DESKTOP, FALSE);
}
BOOL tk::System::SystemShutdown(DWORD uFlags, BOOL bForce)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	// Get a token for this process.   

	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return(FALSE);

	// Get the LUID for the shutdown privilege.   

	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
		&tkp.Privileges[0].Luid);

	tkp.PrivilegeCount = 1;  // one privilege to set      
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	// Get the shutdown privilege for this process.   

	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
		(PTOKEN_PRIVILEGES)NULL, 0);

	if (GetLastError() != ERROR_SUCCESS)
		return FALSE;

	if (bForce)
	{
		uFlags |= EWX_FORCE; //强制终止进程。当此标志设置，Windows不会发送消息WM_QUERYENDSESSION和WM_ENDSESSION的消息给目前在系统中运行的程序。这可能会导致应用程序丢失数据。  
	}
	if (!ExitWindowsEx(uFlags,
		SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
		SHTDN_REASON_MINOR_UPGRADE |
		SHTDN_REASON_FLAG_PLANNED))
		return FALSE;

	return TRUE;
}
BOOL tk::System::SystemSleep(BOOLEAN Hibernate, BOOLEAN DisableWakeEvent)
{
	HMODULE hModule = NULL;
	BOOL bRet = FALSE;
	BOOLEAN WINAPI SetSuspendState(
		__in          BOOLEAN Hibernate,
		__in          BOOLEAN ForceCritical,
		__in          BOOLEAN DisableWakeEvent
	);
	typedef BOOLEAN(WINAPI* PSetSuspendState)(BOOLEAN Hibernate,
		BOOLEAN ForceCritical,
		BOOLEAN DisableWakeEvent);
	hModule = LoadLibrary(TEXT("PowrProf.dll"));
	if (hModule)
	{
		PSetSuspendState pSetSuspendState = NULL;
		pSetSuspendState = (PSetSuspendState)GetProcAddress(hModule, "SetSuspendState");
		if (pSetSuspendState != NULL)
		{
			bRet = pSetSuspendState(Hibernate, 0, DisableWakeEvent);
		}
		FreeLibrary(hModule);
	}
	return bRet;
}
BOOL tk::System::GetVersion(LPOSVERSIONINFO lpVersionInformation)
{
	if (!lpVersionInformation)
		return FALSE;
	VS_FIXEDFILEINFO ffi;
	TCHAR szBuffer[MAX_PATH];
	SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_SYSTEM, FALSE);
	_tcscat_s(szBuffer, MAX_PATH, TEXT("\\ntoskrnl.exe"));
	tk::File::GetFileVersion(szBuffer, &ffi);

	lpVersionInformation->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	//GetVersionEx(lpVersionInformation);
	lpVersionInformation->dwMajorVersion = HIWORD(ffi.dwFileVersionMS);
	lpVersionInformation->dwMinorVersion = LOWORD(ffi.dwFileVersionMS);
	lpVersionInformation->dwBuildNumber = HIWORD(ffi.dwFileVersionLS);
	return TRUE;
}
BOOL tk::System::RunBat(LPCTSTR lpcBatText, BOOL bNewConsole, BOOL bShow)
{
	if (!lpcBatText)
		return FALSE;

	TCHAR szPath[MAX_PATH];
	//生成文件名
	{
		GetTempPath(MAX_PATH, szPath); //获取临时目录，包含'\\'
		for (size_t i = 0; i < 8; i++)
		{
			char c = (char)(rand() % 36);
			if (c < 10)
			{
				c += '0';
			}
			else
			{
				c += 'A' - 10;
			}
			_stprintf_s(szPath, TEXT("%s%c"), szPath, c);
		}
		_tcscat_s(szPath, MAX_PATH, TEXT(".bat"));
	}
	//写入文件
	{
		tk::File::WriteMultiBytes(szPath, lpcBatText);
		tk::File::WriteMultiBytes(szPath, "\r\n");
		TCHAR szBuffer[STRING_LENGTH];
		_stprintf_s(szBuffer, TEXT("del /f \"%s\""), szPath);
		tk::File::WriteMultiBytes(szPath, szBuffer);
	}
	//运行
	{
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;
		si.dwFlags = STARTF_USESHOWWINDOW; //指定wShowWindow成员有效
		si.wShowWindow = (WORD)bShow; //此成员设为TRUE的话则显示新建进程的主窗口
		if (!CreateProcess(
			NULL, //不在此指定可执行文件的文件名
			szPath, //命令行参数
			NULL, //默认进程安全性  
			NULL, //默认进程安全性
			FALSE, //指定当前进程内句柄不可以被子进程继承
			bNewConsole ? CREATE_NEW_CONSOLE : NULL, //为新进程创建一个新的控制台窗口
			NULL, //使用本进程的环境变量
			NULL, //使用本进程的驱动器和目录
			&si,
			&pi))
		{
			return FALSE;
		}
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	return TRUE;
}
BOOL tk::System::DeleteTree(LPCTSTR lpcPath)
{
	if (!lpcPath) return FALSE;
	DWORD dwLength = _tcslen(lpcPath);
	if (dwLength >= MAX_PATH) return FALSE;

	//确保目录的路径以2个\0结尾
	TCHAR tczFolder[MAX_PATH + 1] = { 0 };
	_tcscpy_s(tczFolder, MAX_PATH + 1, lpcPath);
	tczFolder[dwLength] = TEXT('\0');
	tczFolder[dwLength + 1] = TEXT('\0');

	SHFILEOPSTRUCT FileOp = { NULL };
	FileOp.fFlags = FOF_NO_UI;
	FileOp.wFunc = FO_DELETE;
	FileOp.pFrom = tczFolder; //要删除的目录，必须以2个\0结尾

	//删除目录
	return !SHFileOperation(&FileOp);
}
VOID tk::System::KeybdEvent(BYTE bVk, DWORD dwFlags)
{
	if (dwFlags <= 3)
	{
		keybd_event(bVk, (BYTE)MapVirtualKey(bVk, 0), dwFlags, NULL);
	}
	if (dwFlags == 4)
	{
		keybd_event(bVk, (BYTE)MapVirtualKey(bVk, 0), KEYEVENTF_KEYUP, NULL);
		Sleep(5);
		keybd_event(bVk, (BYTE)MapVirtualKey(bVk, 0), 0, NULL);
	}
	if (dwFlags >= 5)
	{
		keybd_event(bVk, (BYTE)MapVirtualKey(bVk, 0), 0, NULL);
		Sleep(dwFlags);
		keybd_event(bVk, (BYTE)MapVirtualKey(bVk, 0), KEYEVENTF_KEYUP, NULL);
		Sleep(dwFlags);
	}
}
VOID tk::System::MouseEvent(DWORD dwFlags, DWORD dwData, DWORD dx, DWORD dy, HWND hwnd)
{
	if (dx != SKIP && dy != SKIP)
	{
		if (hwnd)
		{
			POINT point = { 0,0 };
			ClientToScreen(hwnd, &point);
			SetCursorPos((int)(point.x + dx), (int)(point.y + dy));
		}
		else
		{
			SetCursorPos((int)(dx), (int)(dy));
		}
	}

	if (dwFlags == LBDOWN)
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, NULL, NULL);
	}
	else if (dwFlags == LBUP)
	{
		mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, NULL, NULL);
	}
	else if (dwFlags == LBCLICK)
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, NULL, NULL);
		Sleep(dwData);
		mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, NULL, NULL);
	}
	else if (dwFlags == LBDBCLICK)
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, NULL, NULL);
		Sleep(dwData);
		mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, NULL, NULL);
		Sleep(GetDoubleClickTime());
		mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, NULL, NULL);
		Sleep(dwData);
		mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, NULL, NULL);
	}
	else if (dwFlags == RBDOWN)
	{
		mouse_event(MOUSEEVENTF_RIGHTDOWN, NULL, NULL, NULL, NULL);
	}
	else if (dwFlags == RBUP)
	{
		mouse_event(MOUSEEVENTF_RIGHTUP, NULL, NULL, NULL, NULL);
	}
	else if (dwFlags == RBCLICK)
	{
		mouse_event(MOUSEEVENTF_RIGHTDOWN, NULL, NULL, NULL, NULL);
		Sleep(dwData);
		mouse_event(MOUSEEVENTF_RIGHTUP, NULL, NULL, NULL, NULL);
	}
	else if (dwFlags == RBDBCLICK)
	{
		mouse_event(MOUSEEVENTF_RIGHTDOWN, NULL, NULL, NULL, NULL);
		Sleep(dwData);
		mouse_event(MOUSEEVENTF_RIGHTUP, NULL, NULL, NULL, NULL);
		Sleep(GetDoubleClickTime());
		mouse_event(MOUSEEVENTF_RIGHTDOWN, NULL, NULL, NULL, NULL);
		Sleep(dwData);
		mouse_event(MOUSEEVENTF_RIGHTUP, NULL, NULL, NULL, NULL);
	}
	else if (dwFlags == MBDOWN)
	{
		mouse_event(MOUSEEVENTF_MIDDLEDOWN, NULL, NULL, NULL, NULL);
	}
	else if (dwFlags == MBUP)
	{
		mouse_event(MOUSEEVENTF_MIDDLEUP, NULL, NULL, NULL, NULL);
	}
	else if (dwFlags == MBCLICK)
	{
		mouse_event(MOUSEEVENTF_MIDDLEDOWN, NULL, NULL, NULL, NULL);
		Sleep(dwData);
		mouse_event(MOUSEEVENTF_MIDDLEUP, NULL, NULL, NULL, NULL);
	}
	else if (dwFlags == MBDBCLICK)
	{
		mouse_event(MOUSEEVENTF_MIDDLEDOWN, NULL, NULL, NULL, NULL);
		Sleep(dwData);
		mouse_event(MOUSEEVENTF_MIDDLEUP, NULL, NULL, NULL, NULL);
		Sleep(GetDoubleClickTime());
		mouse_event(MOUSEEVENTF_MIDDLEDOWN, NULL, NULL, NULL, NULL);
		Sleep(dwData);
		mouse_event(MOUSEEVENTF_MIDDLEUP, NULL, NULL, NULL, NULL);
	}
	else if (dwFlags == MWHEEL)
	{
		mouse_event(MOUSEEVENTF_WHEEL, NULL, NULL, dwData, NULL);
	}
}

BOOL tk::Process::EnableDebugPrivilege(BOOL fEnable)
{
	BOOL fOk = FALSE;
	HANDLE hToken;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = (DWORD)(fEnable ? SE_PRIVILEGE_ENABLED : 0);
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		fOk = (GetLastError() == ERROR_SUCCESS);
		CloseHandle(hToken);
	}
	return(fOk);
}
BOOL tk::Process::SetPriorityClass(INT nPriority)
{
	return ::SetPriorityClass(GetCurrentProcess(), nPriority);
}
BOOL tk::Process::TerminateProcess(DWORD dwProcessId)
{
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
	BOOL bResult = ::TerminateProcess(hProcess, 0);
	CloseHandle(hProcess);
	return bResult;
}
DWORD tk::Process::GetPIdFromWindow(HWND hwnd)
{
	DWORD dwProcessId = NULL;
	GetWindowThreadProcessId(hwnd, &dwProcessId);
	return dwProcessId;
}
DWORD tk::Process::GetPIdFromName(LPCTSTR lpcName)
{
	DWORD dwPId = NULL;
	PROCESSENTRY32 ProcessInfo = { sizeof(PROCESSENTRY32) };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (!hSnapshot) return FALSE;

	BOOL bStatus = Process32First(hSnapshot, &ProcessInfo);
	while (bStatus)
	{
		if (!_tcsicmp(ProcessInfo.szExeFile, lpcName))
		{
			dwPId = ProcessInfo.th32ProcessID;
			break;
		}
		bStatus = Process32Next(hSnapshot, &ProcessInfo);
	}
	CloseHandle(hSnapshot);
	return dwPId;
}
BOOL tk::Process::GetProcessNameFromPId(DWORD dwProcessId, LPTSTR lpString, size_t iMaxLength)
{
	if (!lpString) return FALSE;

	PROCESSENTRY32 ProcessInfo = { sizeof(ProcessInfo) };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (!hSnapshot) return FALSE;

	BOOL bStatus = Process32First(hSnapshot, &ProcessInfo);
	while (bStatus)
	{
		if (dwProcessId == ProcessInfo.th32ProcessID)
		{
			_tcscpy_s(lpString, iMaxLength, ProcessInfo.szExeFile);
			CloseHandle(hSnapshot);
			return TRUE;
		}
		bStatus = Process32Next(hSnapshot, &ProcessInfo);
	}
	CloseHandle(hSnapshot);
	return FALSE;
}
HANDLE tk::Process::CreateProcessEz(LPCTSTR lpcPath, LPTSTR lpCmd, BOOL bShowWindow)
{
	TCHAR szBuffer[STRING_LENGTH * STRING_LENGTH] = { 0 };
	if (!lpcPath) return NULL;
	if (lpCmd) _stprintf_s(szBuffer, TEXT("\"%s\" %s"), lpcPath, lpCmd);
	else _tcscpy_s(szBuffer, lpcPath);

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	si.dwFlags = STARTF_USESHOWWINDOW; //指定wShowWindow成员有效
	si.wShowWindow = (WORD)(bShowWindow ? TRUE : FALSE); //此成员设为TRUE的话则显示新建进程的主窗口
	CreateProcess(
		lpcPath, //不在此指定可执行文件的文件名
		szBuffer, //命令行参数
		NULL, //默认进程安全性  
		NULL, //默认进程安全性
		FALSE, //指定当前进程内句柄不可以被子进程继承
		CREATE_NEW_CONSOLE, //为新进程创建一个新的控制台窗口
		NULL, //使用本进程的环境变量
		NULL, //使用本进程的驱动器和目录
		&si,
		&pi);
	CloseHandle(pi.hThread);
	return pi.hProcess;
}
VOID tk::Process::SuspendProcess(DWORD dwProcessId, BOOL fSuspend)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwProcessId);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 te = { sizeof(te) };
		BOOL fOk = Thread32First(hSnapshot, &te);
		for (; fOk; fOk = Thread32Next(hSnapshot, &te))
		{
			if (te.th32OwnerProcessID == dwProcessId)
			{
				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
				if (hThread != NULL)
				{
					if (fSuspend)
						SuspendThread(hThread);
					else
						ResumeThread(hThread);
				}
				CloseHandle(hThread);
			}
		}
		CloseHandle(hSnapshot);
	}
}
DWORD tk::Process::GetThreadModuleName(HANDLE hProcess, HANDLE hThread, LPTSTR lpName, size_t iMaxLength)
{
	typedef enum _THREADINFOCLASS {
		ThreadBasicInformation,
		ThreadTimes,
		ThreadPriority,
		ThreadBasePriority,
		ThreadAffinityMask,
		ThreadImpersonationToken,
		ThreadDescriptorTableEntry,
		ThreadEnableAlignmentFaultFixup,
		ThreadEventPair_Reusable,
		ThreadQuerySetWin32StartAddress,
		ThreadZeroTlsCell,
		ThreadPerformanceCount,
		ThreadAmILastThread,
		ThreadIdealProcessor,
		ThreadPriorityBoost,
		ThreadSetTlsArrayAddress,
		ThreadIsIoPending,
		ThreadHideFromDebugger,
		ThreadBreakOnTermination,
		MaxThreadInfoClass
	} THREADINFOCLASS;

	if (!lpName) return FALSE;

	typedef LONG(WINAPI* NtQueryInformationThreadProc)(
		_In_       HANDLE ThreadHandle,
		_In_       THREADINFOCLASS ThreadInformationClass,
		_Inout_    PVOID ThreadInformation,
		_In_       ULONG ThreadInformationLength,
		_Out_opt_  PULONG ReturnLength
		);
	NtQueryInformationThreadProc NtQueryInformationThread = NULL;
	HMODULE hNtdll = GetModuleHandle(TEXT("ntdll.dll"));
	if (!hNtdll)
		return FALSE;
	NtQueryInformationThread = (NtQueryInformationThreadProc)GetProcAddress(hNtdll, "NtQueryInformationThread");
	if (!NtQueryInformationThread)
	{
		FreeLibrary(hNtdll);
		return FALSE;
	}

	PVOID pvStart = NULL; //线程起始地址
	NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &pvStart, sizeof(pvStart), NULL); //获取线程起始地址
	GetMappedFileName(hProcess, pvStart, lpName, iMaxLength); //获取线程名

	FreeLibrary(hNtdll);
	return tk::Name::GetFileName(lpName);
}
BOOL tk::Process::InjectDll(LPCSTR lpcDllPath, HANDLE hProcess)
{
	if (!lpcDllPath || !hProcess) return FALSE;

	size_t nLength = strlen(lpcDllPath);
	size_t cbSize = (nLength + 1) * sizeof(CHAR);
	PSTR pszLibFileRemote = NULL;
	HANDLE hThread = NULL;
	BOOL bOk = FALSE;
	__try
	{
		pszLibFileRemote = (PSTR)VirtualAllocEx(hProcess, NULL, cbSize, MEM_COMMIT, PAGE_READWRITE);
		if (!pszLibFileRemote) __leave;

		BOOL bRet = WriteProcessMemory(hProcess, pszLibFileRemote, (PVOID)lpcDllPath, cbSize, NULL);
		if (!bRet) __leave;

		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryA");
		if (!pfnThreadRtn) __leave;

		hThread = CreateRemoteThread(hProcess, NULL, NULL, pfnThreadRtn, pszLibFileRemote, 0, NULL);
		if (!hThread) __leave;
		bOk = TRUE;
	}
	__finally
	{
		WaitForSingleObject(hThread, INFINITE);

		if (pszLibFileRemote)
			VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);

		if (hThread)
			CloseHandle(hThread);
	}
	return bOk;
}
BOOL tk::Process::InjectDll(LPCSTR lpcDllPath, DWORD dwPId)
{
	if (!lpcDllPath || !dwPId) return FALSE;

	size_t nLength = strlen(lpcDllPath);
	size_t cbSize = (nLength + 1) * sizeof(CHAR);
	PSTR pszLibFileRemote = NULL;
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	BOOL bOk = FALSE;
	__try
	{
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
			PROCESS_CREATE_THREAD |
			PROCESS_VM_OPERATION,
			FALSE, dwPId);
		if (!hProcess) __leave;

		pszLibFileRemote = (PSTR)VirtualAllocEx(hProcess, NULL, cbSize, MEM_COMMIT, PAGE_READWRITE);
		if (!pszLibFileRemote) __leave;

		BOOL bRet = WriteProcessMemory(hProcess, pszLibFileRemote, (PVOID)lpcDllPath, cbSize, NULL);
		if (!bRet) __leave;

		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryA");
		if (!pfnThreadRtn) __leave;

		hThread = CreateRemoteThread(hProcess, NULL, NULL, pfnThreadRtn, pszLibFileRemote, 0, NULL);
		if (!hThread) __leave;
		bOk = TRUE;
	}
	__finally
	{
		WaitForSingleObject(hThread, INFINITE);

		if (pszLibFileRemote)
			VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);

		if (hThread)
			CloseHandle(hThread);

		if (hProcess)
			CloseHandle(hProcess);
	}
	return bOk;
}
BOOL tk::Process::InjectDll(LPCWSTR lpcDllPath, HANDLE hProcess)
{
	if (!lpcDllPath || !hProcess) return FALSE;

	size_t nLength = wcslen(lpcDllPath);
	size_t cbSize = (nLength + 1) * sizeof(WCHAR);
	PWSTR pszLibFileRemote = NULL;
	HANDLE hThread = NULL;
	BOOL bOk = FALSE;
	__try
	{
		pszLibFileRemote = (PWSTR)VirtualAllocEx(hProcess, NULL, cbSize, MEM_COMMIT, PAGE_READWRITE);
		if (!pszLibFileRemote) __leave;

		BOOL bRet = WriteProcessMemory(hProcess, pszLibFileRemote, (PVOID)lpcDllPath, cbSize, NULL);
		if (!bRet) __leave;

		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
		if (!pfnThreadRtn) __leave;

		hThread = CreateRemoteThread(hProcess, NULL, NULL, pfnThreadRtn, pszLibFileRemote, 0, NULL);
		if (!hThread) __leave;
		bOk = TRUE;
	}
	__finally
	{
		WaitForSingleObject(hThread, INFINITE);

		if (pszLibFileRemote)
			VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);

		if (hThread)
			CloseHandle(hThread);
	}
	return bOk;
}
BOOL tk::Process::InjectDll(LPCWSTR lpcDllPath, DWORD dwPId)
{
	if (!lpcDllPath || !dwPId) return FALSE;

	size_t nLength = wcslen(lpcDllPath);
	size_t cbSize = (nLength + 1) * sizeof(WCHAR);
	PWSTR pszLibFileRemote = NULL;
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	BOOL bOk = FALSE;
	__try
	{
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
			PROCESS_CREATE_THREAD |
			PROCESS_VM_OPERATION,
			FALSE, dwPId);
		if (!hProcess) __leave;

		pszLibFileRemote = (PWSTR)VirtualAllocEx(hProcess, NULL, cbSize, MEM_COMMIT, PAGE_READWRITE);
		if (!pszLibFileRemote) __leave;

		BOOL bRet = WriteProcessMemory(hProcess, pszLibFileRemote, (PVOID)lpcDllPath, cbSize, NULL);
		if (!bRet) __leave;

		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
		if (!pfnThreadRtn) __leave;

		hThread = CreateRemoteThread(hProcess, NULL, NULL, pfnThreadRtn, pszLibFileRemote, 0, NULL);
		if (!hThread) __leave;
		bOk = TRUE;
	}
	__finally
	{
		WaitForSingleObject(hThread, INFINITE);

		if (pszLibFileRemote)
			VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);

		if (hThread)
			CloseHandle(hThread);

		if (hProcess)
			CloseHandle(hProcess);
	}
	return bOk;
}

DWORD tk::Thread::CreateThreadEz(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, INT nPriority, PHANDLE phThread)
{
	DWORD dwCF = dwCreationFlags | CREATE_SUSPENDED;
	DWORD dwThreadID = NULL;
	HANDLE hThread = CreateThread(NULL, NULL, lpStartAddress, lpParameter, dwCF, &dwThreadID);
	if (nPriority)
		SetThreadPriority(hThread, nPriority);
	if (phThread)
		*phThread = hThread;
	if (!(dwCreationFlags & CREATE_SUSPENDED))
		ResumeThread(hThread);

	if (!phThread)
		CloseHandle(hThread);
	return dwThreadID;
}

BOOL tk::Windows::SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha)
{
	DWORD dwFlags = 0;
	SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

	if (crKey != SKIP)
		dwFlags |= LWA_COLORKEY;
	if (bAlpha != SKIP)
		dwFlags |= LWA_ALPHA;
	return ::SetLayeredWindowAttributes(hwnd, crKey, bAlpha, (BYTE)dwFlags);
}
BOOL tk::Windows::SetWindowTransparent(HWND hwnd, BOOL iTransparent)
{
	if (iTransparent)
		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
	else
		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
	return iTransparent;
}
HWND tk::Windows::GetFocusEx()
{
	HWND hwnd;
	DWORD dwThreadId;
	dwThreadId = GetWindowThreadProcessId(GetForegroundWindow(), 0);
	AttachThreadInput(GetCurrentThreadId(), dwThreadId, TRUE);
	hwnd = GetFocus();
	AttachThreadInput(GetCurrentThreadId(), dwThreadId, FALSE);
	return hwnd;
}
BOOL tk::Windows::SetFocusEx(HWND hwnd)
{
	HWND hwndLast;
	DWORD dwThreadId;
	SetForegroundWindow(hwnd);
	dwThreadId = GetWindowThreadProcessId(hwnd, 0);
	AttachThreadInput(GetCurrentThreadId(), dwThreadId, TRUE);
	hwndLast = SetFocus(hwnd);
	AttachThreadInput(GetCurrentThreadId(), dwThreadId, FALSE);
	if (hwndLast == hwnd)
		return FALSE;
	return TRUE;
}
HWND tk::Windows::WindowFromCursor()
{
	POINT mp;
	GetCursorPos(&mp);
	return WindowFromPoint(mp);
}
BOOL tk::Windows::SetWindowTextFormat(HWND hwnd, LPCTSTR lpcText, ...)
{
	TCHAR   szBuffer[STRING_LENGTH * STRING_LENGTH] = { 0 };
	va_list pArgList;
	va_start(pArgList, lpcText);
	_vsntprintf_s(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), lpcText, pArgList);
	va_end(pArgList);

	return SetWindowText(hwnd, szBuffer);
}
BOOL tk::Windows::SetWindowStyle(HWND hwnd, LONG dwNewLong, BOOL bSet)
{
	LONG dwTemp = GetWindowLong(hwnd, GWL_STYLE);
	if (bSet)
	{
		return SetWindowLong(hwnd, GWL_STYLE, dwTemp | dwNewLong);
	}
	else
	{
		return SetWindowLong(hwnd, GWL_STYLE, dwTemp & ~dwNewLong);
	}
}
BOOL tk::Windows::SetWindowExStyle(HWND hwnd, LONG dwNewLong, BOOL bSet)
{
	LONG dwTemp = GetWindowLong(hwnd, GWL_EXSTYLE);
	if (bSet)
	{
		return SetWindowLong(hwnd, GWL_EXSTYLE, dwTemp | dwNewLong);
	}
	else
	{
		return SetWindowLong(hwnd, GWL_EXSTYLE, dwTemp & ~dwNewLong);
	}
}
RECT tk::Windows::GetWorkArea()
{
	RECT rRet/* = { 0,0,cxS(),cyS() }*/;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rRet, 0);
	if (rRet.left)
	{
		rRet.left++;
		rRet.right--;
		rRet.bottom--;
	}
	else if (rRet.top)
	{
		rRet.top++;
		rRet.right--;
		rRet.bottom--;
	}
	else
	{
		rRet.right--;
		rRet.bottom--;
	}
	return rRet; //闭区间
}
POINT tk::Windows::GetPointInWorkArea(RECT rWorkspace, POINT pScreen)
{
	pScreen.x -= rWorkspace.left;
	pScreen.y -= rWorkspace.top;
	return pScreen;
}
BOOL tk::Windows::SetClientPos(HWND hWnd, HWND hWndInsertAfter, INT X, INT Y, INT cx, INT cy, UINT uFlags)
{
	if (!IsWindow(hWnd)) return FALSE;

	RECT rectW, rectC;
	POINT point = { 0,0 };
	GetWindowRect(hWnd, &rectW);
	GetClientRect(hWnd, &rectC);
	ClientToScreen(hWnd, &point);
	int cxNew = cx + ((rectW.right - rectW.left) - (rectC.right - rectC.left));
	int cyNew = cy + ((rectW.bottom - rectW.top) - (rectC.bottom - rectC.top));
	int XNew = X - (point.x - rectW.left);
	int YNew = Y - (point.y - rectW.top);
	return SetWindowPos(hWnd, hWndInsertAfter, XNew, YNew, cxNew, cyNew, uFlags);
}
BOOL tk::Windows::CenterWindow(HWND hWnd)
{
	if (!IsWindow(hWnd)) return FALSE;

	INT cxS = GetSystemMetrics(SM_CXSCREEN);
	INT cyS = GetSystemMetrics(SM_CYSCREEN);
	RECT rectWnd;
	GetWindowRect(hWnd, &rectWnd);
	INT iX = rectWnd.right - rectWnd.left;
	INT iY = rectWnd.bottom - rectWnd.top;
	iX = (cxS - iX) / 2;
	iY = (cyS - iY) / 2;
	return SetWindowPos(hWnd, NULL, iX, iY, NULL, NULL, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

BYTE tk::GDI::R(COLORREF rgb)
{
	return (BYTE)(rgb & 0x0000FF);
}
BYTE tk::GDI::G(COLORREF rgb)
{
	return (BYTE)((rgb & 0x00FF00) >> 8);
}
BYTE tk::GDI::B(COLORREF rgb)
{
	return (BYTE)((rgb & 0xFF0000) >> 16);
}
HFONT tk::GDI::CreateFontEz(HDC hdc, LPCTSTR szFaceName, INT iDeciPtHeight, INT iDeciPtWidth, INT iAttributes, BOOL fLogRes)
{
	FLOAT      cxDpi, cyDpi;
	HFONT      hFont;
	LOGFONT    lf;
	POINT      pt;
	TEXTMETRIC tm;
	BOOL bNULL = FALSE;

	if (!hdc)
	{
		hdc = GetDC(NULL);
		bNULL = TRUE;
	}

	SaveDC(hdc);

	SetGraphicsMode(hdc, GM_ADVANCED);
	ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);
	SetViewportOrgEx(hdc, 0, 0, NULL);
	SetWindowOrgEx(hdc, 0, 0, NULL);

	if (fLogRes)
	{
		cxDpi = (FLOAT)GetDeviceCaps(hdc, LOGPIXELSX);
		cyDpi = (FLOAT)GetDeviceCaps(hdc, LOGPIXELSY);
	}
	else
	{
		cxDpi = (FLOAT)(25.4 * GetDeviceCaps(hdc, HORZRES) /
			GetDeviceCaps(hdc, HORZSIZE));

		cyDpi = (FLOAT)(25.4 * GetDeviceCaps(hdc, VERTRES) /
			GetDeviceCaps(hdc, VERTSIZE));
	}

	pt.x = (int)(iDeciPtWidth * cxDpi / 72);
	pt.y = (int)(iDeciPtHeight * cyDpi / 72);

	DPtoLP(hdc, &pt, 1);

	lf.lfHeight = -(int)(fabs(pt.y) / 10.0 + 0.5);
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = iAttributes & CF_ATTR_BOLD ? 700 : 0;
	lf.lfItalic = (BYTE)(iAttributes & CF_ATTR_ITALIC ? 1 : 0);
	lf.lfUnderline = (BYTE)(iAttributes & CF_ATTR_UNDERLINE ? 1 : 0);
	lf.lfStrikeOut = (BYTE)(iAttributes & CF_ATTR_STRIKEOUT ? 1 : 0);
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = 0;
	lf.lfClipPrecision = 0;
	lf.lfQuality = CLEARTYPE_NATURAL_QUALITY; //NONANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = 0;

	lstrcpy(lf.lfFaceName, szFaceName);

	hFont = CreateFontIndirect(&lf);

	if (iDeciPtWidth != 0)
	{
		hFont = (HFONT)SelectObject(hdc, hFont);

		GetTextMetrics(hdc, &tm);

		DeleteObject(SelectObject(hdc, hFont));

		lf.lfWidth = (int)(tm.tmAveCharWidth *
			fabs(pt.x) / fabs(pt.y) + 0.5);

		hFont = CreateFontIndirect(&lf);
	}

	RestoreDC(hdc, -1);
	if (bNULL)
	{
		ReleaseDC(NULL, hdc);
	}
	return hFont;
}
BOOL tk::GDI::Blt(HDC hdcDest, INT xDest, INT yDest, INT wDest, INT hDest, HDC hdcSrc, INT xSrc, INT ySrc, INT wSrc, INT hSrc, INT alpha)
{
	SetStretchBltMode(hdcDest, HALFTONE);
	if (alpha != SKIP)
	{
		BLENDFUNCTION bf;
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.AlphaFormat = 0;
		bf.SourceConstantAlpha = BYTE(alpha);
		return AlphaBlend(hdcDest, xDest, yDest, wDest, hDest, hdcSrc, xSrc, ySrc, wSrc, hSrc, bf);
	}
	else
	{
		if (wSrc != SKIP && hSrc != SKIP)
			return StretchBlt(hdcDest, xDest, yDest, wDest, hDest, hdcSrc, xSrc, ySrc, wSrc, hSrc, SRCCOPY);
		else
			return BitBlt(hdcDest, xDest, yDest, wDest, hDest, hdcSrc, xSrc, ySrc, SRCCOPY);
	}
	return 0;
}
HDC tk::GDI::CreateCompatibleDC(HDC hdc, INT cx, INT cy)
{
	HDC hdcScr = NULL;
	if (!hdc) hdcScr = GetDC(NULL);
	else hdcScr = hdc;
	HDC hdcMem = NULL;
	hdcMem = ::CreateCompatibleDC(hdcScr);

	if (cx == SKIP || cy == SKIP)
	{
		if (!hdc) ReleaseDC(NULL, hdcScr);
		return hdcMem;
	}
	else
	{
		HBITMAP hBmp;
		hBmp = CreateCompatibleBitmap(hdcScr, cx, cy);
		SelectObject(hdcMem, hBmp);
		DeleteObject(hBmp);
		if (!hdc) ReleaseDC(NULL, hdcScr);
		return hdcMem;
	}
	return NULL;
}
HBITMAP tk::GDI::HDCToHBitmap(HDC hdc, INT nWidth, INT nHeight)
{
	HDC hBufDC;
	HBITMAP hBitmap, hBitTemp;
	//创建设备上下文(HDC)
	hBufDC = tk::GDI::CreateCompatibleDC(hdc);
	//创建HBITMAP
	hBitmap = CreateCompatibleBitmap(hdc, nWidth, nHeight);
	hBitTemp = (HBITMAP)SelectObject(hBufDC, hBitmap);
	//得到位图缓冲区
	StretchBlt(hBufDC, 0, 0, nWidth, nHeight,
		hdc, 0, 0, nWidth, nHeight, SRCCOPY);
	//得到最终的位图信息
	hBitmap = (HBITMAP)SelectObject(hBufDC, hBitTemp);
	//释放内存
	DeleteObject(hBitTemp);
	DeleteDC(hBufDC);
	return hBitmap;
}
BOOL tk::GDI::HBitmapToFile(HBITMAP hBitmap, LPCTSTR lpcFileName)
{
	HDC hDC;
	//当前分辨率下每象素所占字节数       
	int iBits;
	//位图中每象素所占字节数       
	WORD wBitCount;
	//定义调色板大小，     位图中像素字节大小     ，位图文件大小     ，     写入文件字节数           
	DWORD dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	//位图属性结构           
	BITMAP Bitmap;
	//位图文件头结构       
	BITMAPFILEHEADER bmfHdr;
	//位图信息头结构           
	BITMAPINFOHEADER bi;
	//指向位图信息头结构               
	LPBITMAPINFOHEADER lpbi;
	//定义文件，分配内存句柄，调色板句柄           
	HANDLE fh, hDib, hPal, hOldPal = NULL;

	//计算位图文件每个像素所占字节数           
	hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else  if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else
		wBitCount = 24;

	GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;

	dwBmBitsSize = (DWORD)(((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight);

	//为位图内容分配内存           
	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	//处理调色板               
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	//获取该调色板下新的像素值           
	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight,
		(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
		(BITMAPINFO*)lpbi, DIB_RGB_COLORS);

	//恢复调色板               
	if (hOldPal)
	{
		::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	//创建位图文件               
	fh = CreateFile(lpcFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)         return     FALSE;

	//设置位图文件头           
	bmfHdr.bfType = 0x4D42;     //"BM"           
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
	//写入位图文件头           
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	//写入位图文件其余内容           
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	//清除               
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);
	return TRUE;
}

LSTATUS tk::Reg::RegSetKey(HKEY hKey, LPCTSTR lpcSubKey, LPCTSTR lpcValueName, DWORD dwType, CONST BYTE* lpData, DWORD cbData)
{
	HKEY hKeyCreate = NULL;
	RegCreateKeyEx(hKey, lpcSubKey, NULL, NULL, 0, KEY_WRITE, NULL, &hKeyCreate, NULL);
	LSTATUS lsResult = RegSetValueEx(hKeyCreate, lpcValueName, 0, dwType, lpData, cbData);
	RegCloseKey(hKeyCreate);
	return lsResult;
}
DWORD tk::Reg::RegGetKey(HKEY hKey, LPCTSTR lpcSubKey, LPCTSTR lpcValueName, DWORD dwDefault)
{
	DWORD dwTemp = NULL;
	DWORD dwLength = NULL;
	HKEY hKeyOpen = NULL;
	RegOpenKeyEx(hKey, lpcSubKey, 0, KEY_READ, &hKeyOpen);
	if (RegQueryValueEx(hKeyOpen, lpcValueName, 0, NULL, (LPBYTE)&dwTemp, &dwLength))
	{
		if (RegQueryValueEx(hKeyOpen, lpcValueName, 0, NULL, (LPBYTE)&dwTemp, &dwLength))
		{
			dwTemp = dwDefault;
		}
	}
	RegCloseKey(hKeyOpen);
	return dwTemp;
}
DWORD tk::Reg::RegGetKey(HKEY hKey, LPCTSTR lpcSubKey, LPCTSTR lpcValueName, LPTSTR lpData, size_t iMaxLength, LPCTSTR szDefault)
{
	DWORD dwSize = NULL;
	HKEY hKeyOpen = NULL;
	RegOpenKeyEx(hKey, lpcSubKey, 0, KEY_READ, &hKeyOpen);
	if (RegQueryValueEx(hKeyOpen, lpcValueName, 0, NULL, NULL, &dwSize))
	{
		_tcscpy_s(lpData, iMaxLength, szDefault);
		return FALSE;
	}
	TCHAR* szTemp = (TCHAR*)malloc(dwSize);
	if (RegQueryValueEx(hKeyOpen, lpcValueName, 0, NULL, (LPBYTE)szTemp, &dwSize))
	{
		_tcscpy_s(lpData, iMaxLength, szDefault);
		if (szTemp) free(szTemp);
		return FALSE;
	}
	RegCloseKey(hKeyOpen);
	_tcscpy_s(lpData, iMaxLength, szTemp);
	free(szTemp);
	return dwSize;
}
BOOL tk::Reg::RegGetKey(HKEY hKey, LPCTSTR lpcSubKey, LPCTSTR lpcValueName, LPVOID lpData, size_t cbSize)
{
	DWORD dwSize = NULL;
	HKEY hKeyOpen = NULL;
	BOOL bOk = FALSE;
	__try
	{
		RegOpenKeyEx(hKey, lpcSubKey, 0, KEY_READ, &hKeyOpen);
		if (!hKeyOpen) __leave;

		if (RegQueryValueEx(hKeyOpen, lpcValueName, 0, NULL, NULL, &dwSize))
			__leave;

		if (dwSize != cbSize)
			__leave;

		if (RegQueryValueEx(hKeyOpen, lpcValueName, 0, NULL, (LPBYTE)lpData, &dwSize))
			__leave;

		bOk = TRUE;
	}
	__finally
	{
		if (hKeyOpen) RegCloseKey(hKeyOpen);
	}
	return bOk;
}
LSTATUS tk::Reg::RegDeleteKeyEz(HKEY hKey, LPCTSTR lpcSubKey, LPCTSTR lpcValueName)
{
	HKEY hKeyOpen;
	RegOpenKeyEx(hKey, lpcSubKey, 0, KEY_WRITE, &hKeyOpen);
	LSTATUS lsResult = RegDeleteValue(hKeyOpen, lpcValueName);
	RegCloseKey(hKeyOpen);
	return lsResult;
}

BOOL tk::File::IsFileValid(LPCTSTR lpcFileName)
{
	return GetFileAttributes(lpcFileName) != INVALID_FILE_ATTRIBUTES;
}
BOOL tk::File::GetFileVersion(LPCTSTR lpcFileName, VS_FIXEDFILEINFO* ffi)
{
	if (!ffi)
		return FALSE;
	VS_FIXEDFILEINFO* pffi; //在最后我们得到的是pBlock的一个位置，而不是将值复制到变量中，因此一开始不要指向任何地址
	TCHAR szPath[MAX_PATH];
	if (!lpcFileName)
		GetModuleFileName(NULL, szPath, MAX_PATH);
	else
		_tcscpy_s(szPath, MAX_PATH, lpcFileName);
	DWORD dwHandle;
	DWORD dwSize = GetFileVersionInfoSize(szPath, &dwHandle);
	if (!dwSize)
		return FALSE;
	BYTE* pBlock = (BYTE*)calloc(dwSize, sizeof(BYTE));
	GetFileVersionInfo(szPath, dwHandle, dwSize, pBlock);
	VerQueryValue(pBlock, TEXT("\\"), (LPVOID*)&pffi, (PUINT)&dwSize); //将pBlock中的属于ffi的首地址赋给pffi，而不是将值复制
	*ffi = *pffi;
	free(pBlock);
	return TRUE;
}
BOOL tk::File::WriteMultiBytes(LPCTSTR lpcFileName, LPCWSTR lpcText, DWORD dwLength)
{
	if (!lpcFileName || !lpcText)
		return FALSE;
	char* pBytes = nullptr;
	if (dwLength == SKIP) dwLength = wcslen(lpcText);

	DWORD dwSize = WideCharToMultiByte(CP_UTF8, NULL, lpcText, dwLength, NULL, NULL, NULL, NULL);
	pBytes = (char*)calloc(dwSize + 1, sizeof(char));
	WideCharToMultiByte(CP_UTF8, NULL, lpcText, dwLength, pBytes, dwSize, NULL, NULL);
	HANDLE hFile = CreateFile(lpcFileName, GENERIC_WRITE, NULL, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return 0;

	SetFilePointer(hFile, 0, 0, FILE_END);
	DWORD dwWritten;
	WriteFile(hFile, pBytes, dwSize, &dwWritten, nullptr);
	CloseHandle(hFile);
	free(pBytes);
	return dwWritten;
}
BOOL tk::File::WriteMultiBytes(LPCTSTR lpcFileName, LPCSTR lpcText, DWORD dwLength)
{
	if (!lpcFileName || !lpcText)
		return FALSE;
	if (dwLength == SKIP) dwLength = strlen(lpcText);

	HANDLE hFile = CreateFile(lpcFileName, GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(hFile, 0, 0, FILE_END);
	DWORD dwWritten;
	WriteFile(hFile, lpcText, dwLength * sizeof(char), &dwWritten, nullptr);
	CloseHandle(hFile);
	return dwWritten;
}

size_t tk::Name::GetLastPath(LPTSTR lpStr)
{
	if (!lpStr) return 0;
	TCHAR szBuffer[MAX_PATH] = { NULL };
	_tcscpy_s(szBuffer, MAX_PATH, lpStr);
	size_t iLength = _tcslen(lpStr);

	INT mark = 0;
	INT i;
	for (i = 0; i < (signed)iLength; i++)
	{
		if (szBuffer[iLength - i - 1] == TEXT('\\'))
		{
			mark = iLength - i - 1;
			break;
		}
	}
	if (mark > 0)
	{
		_tcsncpy_s(lpStr, _tcslen(lpStr), szBuffer, mark);
		return mark;
	}
	return 0;
}
size_t tk::Name::GetFileName(LPTSTR lpStr)
{
	if (!lpStr) return 0;
	TCHAR szBuffer[MAX_PATH] = { NULL };
	_tcscpy_s(szBuffer, MAX_PATH, lpStr);
	size_t iLength = _tcslen(lpStr);

	INT i;
	for (i = iLength - 1; i >= 0; i--)
	{
		if (szBuffer[i] == TEXT('\\'))
		{
			break;
		}
	}
	if (i >= 0 && i != INT(iLength) - 1)
	{
		_tcsncpy_s(lpStr, _tcslen(lpStr) + 1, &szBuffer[i + 1], iLength - i);
		return iLength - i - 1;
	}
	return 0;
}
size_t tk::Name::GetNameWithoutExtra(LPTSTR lpStr)
{
	if (!lpStr) return 0;
	TCHAR szBuffer[MAX_PATH] = { NULL };
	_tcscpy_s(szBuffer, MAX_PATH, lpStr);
	size_t iLength = _tcslen(lpStr);

	INT mark = 0;
	INT i;
	for (i = 0; i < (signed)iLength; i++)
	{
		if (szBuffer[iLength - i - 1] == TEXT('.'))
		{
			mark = iLength - i - 1;
			break;
		}
	}
	if (mark > 0)
	{
		_tcsncpy_s(lpStr, _tcslen(lpStr), szBuffer, mark);
		return mark;
	}
	return 0;
}
size_t tk::Name::GetExtraName(LPTSTR lpStr)
{
	if (!lpStr) return 0;
	TCHAR szBuffer[MAX_PATH] = { NULL };
	_tcscpy_s(szBuffer, MAX_PATH, lpStr);
	size_t iLength = _tcslen(lpStr);

	INT i;
	for (i = iLength - 1; i >= 0; i--)
	{
		if (szBuffer[i] == TEXT('.'))
		{
			break;
		}
	}
	if (i >= 0 && i != INT(iLength) - 1)
	{
		_tcsncpy_s(lpStr, _tcslen(lpStr) + 1, &szBuffer[i + 1], iLength - i);
		return iLength - i - 1;
	}
	return 0;
}
size_t tk::Name::AddQuotes(LPTSTR lpStr, size_t nSize)
{
	if (!lpStr) return 0;
	size_t iLength = _tcslen(lpStr) + 3;

	TCHAR* szBuffer = (TCHAR*)calloc(iLength, sizeof(TCHAR));
	iLength = _stprintf_s(szBuffer, iLength, TEXT("\"%s\""), lpStr);
	_tcscpy_s(lpStr, nSize, szBuffer);
	free(szBuffer);
	return iLength;
}