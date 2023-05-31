#include "..\TKernel\TKernel.h"
#include "resource.h"
#include "Global Variable.h"

#include "Main Window.h"
#include "Setting Window.h"
#include "Memory Cleaner.h"
#include "Net Speed.h"
#include "Tools.h"

// 声明
namespace MainWindow
{
	constexpr TCHAR szClassName[] = TEXT("Memory Cleaner");
	constexpr TCHAR szCaption[] = TEXT("Memory Cleaner");
	HWND hwndMain;

	tk::Tray tray;

	BOOL bLaunch;

	INT iWidth = 149;
	INT iHeight = 42;
}

// 杂项
BOOL MainWindow::RegisterClasses()
{
	WNDCLASSEX wndclassex = { sizeof(WNDCLASSEX) }; //窗口类
	wndclassex.style = CS_DBLCLKS; //类风格
	wndclassex.lpfnWndProc = VirtualProc; //窗口过程
	wndclassex.cbClsExtra = 0;
	wndclassex.cbWndExtra = 0;
	wndclassex.hInstance = HINST; //实例句柄
	wndclassex.hIcon = GetIconLarge(); //图标句柄
	wndclassex.hIconSm = GetIconSmall();
	wndclassex.hCursor = LoadCursor(NULL, IDC_ARROW); //鼠标指针句柄
	wndclassex.hbrBackground = CreateSolidBrush(RGB(255, 255, 255)); //客户区颜色画刷
	wndclassex.lpszMenuName = NULL; //菜单句柄
	wndclassex.lpszClassName = szClassName; //窗口类名
	return !!RegisterClassEx(&wndclassex);
}
INT_PTR MainWindow::InitInstance()
{
	hwndMain = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		szClassName, //窗口类名
		szCaption, //窗口标题
		WS_NOFRAME, //窗口样式
		mc.mcc.iLeft, //左边
		mc.mcc.iTop, //顶边
		iWidth = dpi(iWidth), //宽度
		iHeight = dpi(iHeight), //高度
		NULL, //父窗口句柄
		NULL, //菜单句柄
		HINST, //程序实例句柄
		NULL); //创建参数
	if (!hwndMain)
	{
		return 1;
	}

	ShowWindow(hwndMain, SW_HIDE);
	ShowWindow(hwndMain, mc.mcc.bShow ? SW_SHOW : SW_HIDE);
	return 0;
}
DWORD WINAPI MainWindow::MsgLoop(LPVOID lpParam)
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

// 透明效果
namespace MainWindow
{
	DWORD WINAPI AnimationThread(LPVOID lpParam)
	{
		WndStruct& ws = *(WndStruct*)lpParam;

		constexpr INT step = 5;
		constexpr DWORD delay = 15;
		HWND hwnd = ws.hWnd;
		DWORD& cnt = ws.dwTransparent;
		DWORD& cfg = mc.mcc.dwTransparent;
		HANDLE hWait[] = { ws.hSignal, ws.hEvent };

		cnt = cfg;
		DWORD dwRet;
		while (true)
		{
			dwRet = WaitForMultipleObjects(sizeof(hWait) / sizeof(HANDLE), hWait, FALSE, INFINITE);
			if (dwRet == WAIT_OBJECT_0)
				break;

			if (ws.bMouseHover && !ws.bMouseDown)
			{
				if (mc.mcc.bAnimation)
				{
					if (cnt == 100)
					{
						ResetEvent(ws.hEvent);
					}
					else
					{
						cnt = min(cnt + step, 100);
					}
				}
				else
				{
					cnt = 100;
					ResetEvent(ws.hEvent);
				}
			}
			else
			{
				if (mc.mcc.bAnimation)
				{
					if (cnt == cfg)
					{
						ResetEvent(ws.hEvent);
					}
					else
					{
						cnt = max(cnt - step, cfg);
					}
				}
				else
				{
					cnt = cfg;
					ResetEvent(ws.hEvent);
				}
			}
			ws.OnPaint(hwnd);

			dwRet = WaitForSingleObject(ws.hSignal, delay);
			if (dwRet == WAIT_OBJECT_0)
				break;
		}
		return 0;
	}
}

// 消息
namespace MainWindow
{
	BOOL WndStruct::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
	{
		// 确保句柄赋值
		hwndMain = hwnd;

		// 初始化类
		tray.Init(hwnd);
		tray.Add(GetIconSmall(), szAppName);

		// 发送初始化消息
		SEND_WM_RESETPOSITION(hwnd, mc.mcc.bShow, FALSE);
		if (!bLaunch)
			PostMessage(hwnd, WM_COMMAND, IDM_SETTING, 0);

		SetTimer(hwnd, 0, 1000, nullptr);

		Update();
		return TRUE;
	}
	VOID WndStruct::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
	{
		switch (id)
		{
		case IDM_EXIT:
		{
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		}
		case IDM_SETTING:
		{
			if (SettingWindow::hThread)
			{
				PostMessage(SettingWindow::hwndSetting, WM_CLOSE, 0, 0);
			}
			else
			{
				SettingWindow::Create();
			}
			break;
		}
		default:
			break;
		}
	}
	VOID WndStruct::OnMoving(HWND hwnd, RECT* pRect)
	{
		mc.mcc.iLeft = pRect->left;
		mc.mcc.iTop = pRect->top;
	}
	VOID WndStruct::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
	{
		TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = hwnd;
		tme.dwHoverTime = 0;
		TrackMouseEvent(&tme);
		if (!bMouseHover)
		{
			bMouseHover = TRUE;
			Update();
		}
	}
	VOID WndStruct::OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		POINT point = { x, y };
		ClientToScreen(hwnd, &point);
		TrackPopupMenu(GetRBMenu(), TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL);
	}
	VOID WndStruct::OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
	{
		bMouseDown = FALSE;
		Update();

		mc.WriteConfig();
	}
	VOID WndStruct::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		bMouseDown = TRUE;
		Update();

		if (fDoubleClick)
		{
			PostMessage(hwnd, WM_COMMAND, IDM_SETTING, 0);
		}
		else
		{
			PostMessage(hwnd, WM_NCLBUTTONDOWN, 2, MAKELPARAM(x, y));
		}
	}
	VOID WndStruct::OnNCLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest)
	{
		FORWARD_WM_NCLBUTTONDOWN(hwnd, fDoubleClick, x, y, codeHitTest, DefWindowProc);
		PostMessage(hwnd, WM_LBUTTONUP, 0, 0);
	}
	VOID WndStruct::OnDisplayChange(HWND hwnd, UINT bitsPerPixel, UINT cxScreen, UINT cyScreen)
	{
		SEND_WM_RESETPOSITION(hwnd, mc.mcc.bShow, TRUE);
	}
	BOOL WndStruct::OnQueryEndSession(HWND hwnd)
	{
		mc.ReleaseResource(HINST, NULL, NULL);
		return TRUE;
	}
	VOID WndStruct::OnDestroy(HWND hwnd)
	{
		SendMessage(SettingWindow::hwndSetting, WM_CLOSE, 0, 0);

		tray.Delete();
		PostQuitMessage(0);
		hwndMain = NULL;
		delete this;
	}
	VOID WndStruct::OnTimer(HWND hwnd, UINT id)
	{
		if (id == 0)
		{
			if (mc.mcc.bShow)
				SetWindowPos(hwndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			SetTimer(hwnd, 0, 1000, nullptr);
		}
	}

	VOID WndStruct::OnPaint(HWND hwnd)
	{
		using namespace Gdiplus;
		using namespace Tool;

		HDC hdcMem = tk::CreateCompatibleDC(NULL, iWidth, iHeight);

		BLENDFUNCTION bf = { 0 };
		bf.BlendOp = AC_SRC_OVER;
		//bf.SourceConstantAlpha = MulDiv(255, dwTransparent, 100);
		bf.SourceConstantAlpha = 255;
		bf.AlphaFormat = AC_SRC_ALPHA;
		POINT pSrc = { 0, 0 };
		SIZE sizeWnd = { iWidth, iHeight };
		TCHAR szBuffer[tk::STRING_LENGTH];

		int num = 0;
		FontFamily fontFamily;
		PrivateFont.GetFamilies(1, &fontFamily, &num);
		Font fontLeft(&fontFamily, dpi(16), FontStyleRegular, UnitPixel);
		Font fontRight(&fontFamily, dpi(15), FontStyleRegular, UnitPixel);
		// Your Paint On hdcMem
		if (!IsForegroundFullscreen())
		{
			Graphics graphics(hdcMem);
			graphics.SetSmoothingMode(SmoothingMode::SmoothingModeHighQuality);
			graphics.SetTextRenderingHint(TextRenderingHint::TextRenderingHintClearTypeGridFit);
			int length = 0;
			int cxBegin = 0;

			//背景底色
			Rect WndRect(0, 0, iWidth, iHeight);
			graphics.SetCompositingMode(CompositingMode::CompositingModeSourceCopy);
			auto t = Color(MulDiv(255, dwTransparent, 100), 255, 255, 255);
			LinearGradientBrush bk(Point(), Point(0, 1), t, t);
			graphics.FillRectangle(&bk, WndRect);
			graphics.SetCompositingMode(CompositingMode::CompositingModeSourceOver);

			SolidBrush BrushBlack(Color(255, 0, 0, 0));
			SolidBrush BrushWhite(Color(255, 255, 255, 255));
			SolidBrush BrushLeft(msex.dwMemoryLoad < 75 ? Color(91 + MulDiv(164, dwTransparent, 100), 17, 125, 187) : Color(91 + MulDiv(164, dwTransparent, 100), 255, 127, 12));

			//初步计算边框
			RectF sizeMax, size;
			length = _stprintf_s(szBuffer, TEXT("888%%"));
			graphics.MeasureString(szBuffer, length, &fontLeft, PointF(0, 0), &sizeMax);
			cxBegin = (int)sizeMax.Width + 2;

			//绘制边框
			Pen PenBorder(Color(91 + MulDiv(164, dwTransparent, 100), 217, 234, 244), 3);
			graphics.DrawRectangle(&PenBorder, Rect(0, 0, iWidth - 1, iHeight - 1));
			graphics.DrawRectangle(&PenBorder, Rect(0, 0, cxBegin, iHeight - 1));

			//绘制内存使用情况
			graphics.FillRectangle(&BrushLeft, Rect(0, iHeight - MulDiv(iHeight, msex.dwMemoryLoad, 100), cxBegin, MulDiv(iHeight, msex.dwMemoryLoad, 100)));
			graphics.DrawRectangle(&PenBorder, Rect(0, iHeight - MulDiv(iHeight, msex.dwMemoryLoad, 100), cxBegin, MulDiv(iHeight, msex.dwMemoryLoad, 100)));

			SolidBrush BrushLeftFont(Color::White);
			//打印内存占用百分比
			//graphics.SetTextRenderingHint(TextRenderingHintSystemDefault);
			length = _stprintf_s(szBuffer, TEXT("%d%%"), msex.dwMemoryLoad);
			graphics.MeasureString(szBuffer, length, &fontLeft, PointF(0, 0), &size);
			tk::GDIP::DrawStringShadow(graphics, szBuffer, &fontLeft, PointF((cxBegin - size.Width) / 2, iHeight - size.Height - 4), &BrushLeftFont);

			RectF rectUp(cxBegin + dpi(3), dpi(2.0), iWidth - cxBegin - dpi(6.0), iHeight / 2);
			RectF rectDown(cxBegin + dpi(3), iHeight / 2, iWidth - cxBegin - dpi(6.0), iHeight / 2 - dpi(2.0));
			StringFormat sf;
			SolidBrush BrushRightFont(Color(239 + MulDiv(16, dwTransparent, 100), 255, 255, 255));
			//打印上传速度
			length = _stprintf_s(szBuffer, TEXT("↑"));
			sf.SetAlignment(StringAlignment::StringAlignmentNear);
			sf.SetLineAlignment(StringAlignment::StringAlignmentCenter);
			tk::GDIP::DrawStringShadow(graphics, szBuffer, &fontRight, rectUp, &sf, &BrushRightFont);
			length = FormatSpeedUnit(szBuffer, tk::STRING_LENGTH, ns.GetUploadSpeed());
			sf.SetAlignment(StringAlignment::StringAlignmentFar);
			sf.SetLineAlignment(StringAlignment::StringAlignmentCenter);
			tk::GDIP::DrawStringShadow(graphics, szBuffer, &fontRight, rectUp, &sf, &BrushRightFont);

			//打印下载速度
			length = _stprintf_s(szBuffer, TEXT("↓"));
			sf.SetAlignment(StringAlignment::StringAlignmentNear);
			sf.SetLineAlignment(StringAlignment::StringAlignmentCenter);
			tk::GDIP::DrawStringShadow(graphics, szBuffer, &fontRight, rectDown, &sf, &BrushRightFont);
			length = FormatSpeedUnit(szBuffer, tk::STRING_LENGTH, ns.GetDownloadSpeed());
			sf.SetAlignment(StringAlignment::StringAlignmentFar);
			sf.SetLineAlignment(StringAlignment::StringAlignmentCenter);
			tk::GDIP::DrawStringShadow(graphics, szBuffer, &fontRight, rectDown, &sf, &BrushRightFont);
		}
		UpdateLayeredWindow(hwndMain, NULL, NULL, &sizeWnd, hdcMem, &pSrc, NULL, &bf, ULW_ALPHA);
		DeleteDC(hdcMem);
	}
	VOID WndStruct::OnTray(HWND hwnd, UINT message, UINT identifier)
	{
		switch (message)
		{
		case WM_LBUTTONDOWN:
		{
			SendMessage(hwnd, WM_COMMAND, IDM_SETTING, 0);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			POINT point;
			GetCursorPos(&point);
			SetForegroundWindow(hwnd); // 解决在菜单外单击左键菜单不消失的问题
			TrackPopupMenu(GetRBMenu(), TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL);
			break;
		}
		case WM_MOUSEMOVE:
		{
			SetWindowPos(hwndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			break;
		}
		default:
			break;
		}
	}
	VOID WndStruct::OnFindInstance(HWND hwnd)
	{
		tray.Balloon(TEXT("已经运行 Memory Cleaner"), TEXT("Notice"));
	}
	VOID WndStruct::OnResetPosition(HWND hwnd, BOOL fShow, BOOL fDisplayChanged)
	{
		RECT rect = tk::GetWorkArea();
		rect.right -= iWidth;
		rect.bottom -= iHeight;
		INT iLeft = min(max(rect.left, mc.mcc.iLeft), rect.right + 1);
		INT iTop = min(max(rect.top, mc.mcc.iTop), rect.bottom + 1);
		if (!fDisplayChanged)
		{
			mc.mcc.iLeft = iLeft;
			mc.mcc.iTop = iTop;
		}
		SetWindowPos(hwnd, HWND_TOPMOST, iLeft, iTop, NULL, NULL, SWP_NOSIZE);
		ShowWindow(hwnd, fShow ? SW_SHOW : SW_HIDE);
	}
	VOID WndStruct::OnUpdateInfo(HWND hwnd)
	{
		OnPaint(hwnd);
	}
}
LRESULT CALLBACK MainWindow::WndStruct::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	wp.VirtualProc(hwnd, message, wParam, lParam);
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hwnd, WM_MOVING, OnMoving);
		HANDLE_MSG(hwnd, WM_MOUSEMOVE, OnMouseMove);
		HANDLE_MSG(hwnd, WM_LBUTTONDOWN, OnLButtonDown);
		HANDLE_MSG(hwnd, WM_LBUTTONDBLCLK, OnLButtonDown);
		HANDLE_MSG(hwnd, WM_LBUTTONUP, OnLButtonUp);
		HANDLE_MSG(hwnd, WM_NCLBUTTONDOWN, OnNCLButtonDown);
		HANDLE_MSG(hwnd, WM_RBUTTONDOWN, OnRButtonDown);
		HANDLE_MSG(hwnd, WM_DISPLAYCHANGE, OnDisplayChange);
		HANDLE_MSG(hwnd, WM_QUERYENDSESSION, OnQueryEndSession);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hwnd, WM_TIMER, OnTimer);

		HANDLE_MSG(hwnd, WM_TRAY, OnTray);
		HANDLE_MSG(hwnd, WM_FINDINSTANCE, OnFindInstance);
		HANDLE_MSG(hwnd, WM_RESETPOSITION, OnResetPosition);
		HANDLE_MSG(hwnd, WM_UPDATEINFO, OnUpdateInfo);

	case WM_MOUSELEAVE:
	{
		bMouseHover = FALSE;
		Update();
		break;
	}
	default:
	{
		if (message == tray.WM_TASKBARCREATED)
		{
			tray.Add();
			SetWindowPos(hwnd, HWND_TOPMOST, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE);
		}
		else
		{
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
	}
	}
	return 0;
}