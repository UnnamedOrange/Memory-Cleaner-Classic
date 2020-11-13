#pragma once
#include "..\TKernel\TKernel.h"
#include "resource.h"

namespace AboutWindow
{
	VOID Create();
	INT_PTR CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
}