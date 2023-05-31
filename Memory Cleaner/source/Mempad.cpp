#include "..\TKernel\TKernel.h"
#include "resource.h"
#include "Global Variable.h"

#include "Mempad.h"
#include "Memory Cleaner.h"

//声明
namespace Mempad
{
	const TCHAR szClassName[] = TEXT("Wnd::Mempad");
	const TCHAR szCaption[] = TEXT("Mempad");
}

//杂项
BOOL Mempad::RegisterClasses()
{
	WNDCLASSEX wndclassex = { sizeof(WNDCLASSEX) }; //窗口类
	wndclassex.style = CS_HREDRAW | CS_VREDRAW; //类风格
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

//窗口动画
namespace Mempad
{
	VOID CALLBACK AnimationTimer(HWND hwnd, UINT message, UINT_PTR iTimerId, DWORD dwTime)
	{
		WndStruct& ws = *(WndStruct*)(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		const DWORD delay[] = { 500, 38 };
		const INT times = 4;
		ws.timer.Set(hwnd, 0, delay[mc.mcc.bAnimation]);

		HDC hdc = GetDC(hwnd);
		ws.OnPaint(hdc, hwnd, mc.mcc.bAnimation && ws.iCount < times);
		ReleaseDC(hwnd, hdc);

		if (ws.iCount >= times || !mc.mcc.bAnimation)
		{
			ws.timer.Kill();
			ws.iCount = 0;
		}
	}
}

//消息
namespace Mempad
{
	BOOL WndStruct::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
	{
		return TRUE;
	}
	namespace PaintProc
	{
		using namespace Gdiplus;
		VOID DrawEllipse(WndStruct& ws, Graphics& graphics)
		{
			const INT baseR = dpi(40);
			ws.R = mc.mcc.bAnimation ? min(ws.R + dpi(3), baseR) : baseR;
			Point pCenter(ws.size.cx / 2, ws.size.cy / 2);
			Rect rectEllipse(ws.cxMouse - ws.R, ws.cyMouse - ws.R, 2 * ws.R, 2 * ws.R);
			Color Center(32, 192, 192, 192), Surround(96, 127, 127, 127);
			GraphicsPath gp;
			gp.AddRectangle(Rect(0, 0, ws.size.cx, ws.size.cy));
			PathGradientBrush BrushEllipse(&gp);
			BrushEllipse.SetCenterColor(Center);
			BrushEllipse.SetCenterPoint(pCenter);
			BrushEllipse.SetFocusScales(0, 0);
			INT num = 1;
			BrushEllipse.SetSurroundColors(&Surround, &num);
			graphics.FillEllipse(&BrushEllipse, rectEllipse);
		}
		VOID PaintLine(WndStruct& ws, Graphics& graphics,
			Pen& PenRuler)
		{
			INT cxWidth = dpi(ws.cxWidth);
			//画竖着的线
			for (int cx = (ws.nxInterval - ws.bOdd) * cxWidth;
				cx < ws.size.cx;
				cx += cxWidth * ws.nxInterval)
			{
				graphics.DrawLine(&PenRuler, cx, 0, cx, ws.size.cy);
			}
			//画横着的线
			for (int cy = 0; cy < ws.size.cy; cy += ws.size.cy / 4)
			{
				graphics.DrawLine(&PenRuler, 0, cy, ws.size.cx, cy);
			}
		}
		template <class T>
		VOID PaintArea(WndStruct& ws, Graphics& graphics,
			Pen& PenBorder, Brush& BrushFill,
			Tool::MonotonousQueue<T, WndStruct::maxn>& mq,
			ULONGLONG ullMaxVal, BOOL bFill)
		{
			if (mq.size() >= 2)
			{
				INT cxWidth = dpi(ws.cxWidth);
				INT nCount = 0;
				Point points[WndStruct::maxn + 5];

				points[nCount].X = 0;
				points[nCount].Y = ws.size.cy;
				nCount++;
				INT cxStart = ws.size.cx - (mq.size() - 1) * cxWidth;
				for (size_t i = 0; i < mq.size(); i++)
				{
					points[nCount].X = cxStart + cxWidth * nCount;
					points[nCount].Y = ws.size.cy - double(ws.size.cy) * mq[i] / ullMaxVal;
					nCount++;
				}
				points[nCount].X = ws.size.cx;
				points[nCount].Y = ws.size.cy;
				nCount++;
				if (bFill) graphics.FillPolygon(&BrushFill, points, nCount);
				else graphics.DrawPolygon(&PenBorder, points, nCount);
			}
		}
	}
	VOID WndStruct::OnPaint(HDC hdc, HWND hwnd, BOOL fTransparent)
	{
		using namespace Gdiplus;

		HDC hdcMem = tk::CreateCompatibleDC(hdc, size.cx, size.cy);
		Graphics graphics(hdcMem);
		graphics.SetSmoothingMode(SmoothingModeHighQuality);
		graphics.SetTextRenderingHint(TextRenderingHint::TextRenderingHintClearTypeGridFit);

		SolidBrush BrushBk(Color(255, 255, 255)); //背景底色
		SolidBrush BrushFont(Color(255, 112, 112, 112)); //文字颜色
		FontFamily fontFamily;
		INT found;
		PrivateFont.GetFamilies(1, &fontFamily, &found);
		TCHAR szBuffer[tk::STRING_LENGTH];

		graphics.FillRectangle(&BrushBk, Rect(0, 0, size.cx, size.cy)); //填充背景
		if (bMouseHover && bMouseDown)
		{
			Pen PenRuler(Color(236, 222, 240)); //线的笔
			Pen PenBorder(Color(139, 18, 174)); //边框的笔

			//画线
			graphics.DrawLine(&PenRuler, size.cx / 3, 0, size.cx / 3, size.cy);
			graphics.DrawLine(&PenRuler, size.cx / 3 * 2, 0, size.cx / 3 * 2, size.cy);

			//画鼠标圆
			PaintProc::DrawEllipse(*this, graphics);

			Font font(&fontFamily, dpi(20), FontStyleRegular, UnitPixel);
			StringFormat sf;
			sf.SetAlignment(StringAlignment::StringAlignmentCenter);
			sf.SetLineAlignment(StringAlignment::StringAlignmentCenter); //居中显示
			if (bSide == 0)
			{
				graphics.DrawString(szBuffer, _stprintf_s(szBuffer, TEXT("松开以更改显示内容")),
					&font, RectF(0, 0, size.cx, size.cy), &sf, &BrushFont);
			}
			else if (bSide == 2)
			{
				graphics.DrawString(szBuffer, _stprintf_s(szBuffer, TEXT("松开以清理内存")),
					&font, RectF(0, 0, size.cx, size.cy), &sf, &BrushFont);
			}

			//画边框
			graphics.DrawRectangle(&PenBorder, Rect(0, 0, size.cx - 1, size.cy - 1));
		}
		else
		{
			const INT _cxWidth = dpi(cxWidth);
			if (iShowWhat == bMouseHover) //内存使用状况
			{
				Pen PenRuler(Color(238, 222, 207)); //线的笔
				Pen PenBorder(Color(167, 79, 1)); //边框的笔
				LinearGradientBrush BrushMem(Point(0, 0), Point(0, size.cy),
					Color(32, 252, 243, 235), Color(255, 252, 243, 235)); //填充的刷子

				//画线
				PaintProc::PaintLine(*this, graphics, PenRuler);

				//画块
				PaintProc::PaintArea(*this, graphics, PenBorder, BrushMem,
					memLoad, msex.ullTotalPhys, TRUE);
				PaintProc::PaintArea(*this, graphics, PenBorder, BrushMem,
					memLoad, msex.ullTotalPhys, FALSE);

				Font font(&fontFamily, dpi(14), FontStyleRegular, UnitPixel);
				StringFormat sf;
				//在左上角写"物理内存 - xx GB"
				graphics.DrawString(szBuffer, _stprintf_s(szBuffer, TEXT("物理内存 - %.1f GB"),
					double(msex.ullTotalPhys) / 1024 / 1024 / 1024),
					&font, PointF(0, 0), &sf, &BrushFont);

				//在右上角写比例
				sf.SetAlignment(StringAlignment::StringAlignmentFar);
				graphics.DrawString(szBuffer, _stprintf_s(szBuffer, TEXT("%d %%"), msex.dwMemoryLoad),
					&font, PointF(size.cx, 0), &sf, &BrushFont);

				//画边框
				graphics.DrawRectangle(&PenBorder, Rect(0, 0, size.cx - 1, size.cy - 1));
			}
			else //网络速率
			{
				Pen PenRuler(Color(217, 234, 244)); //线的笔
				Pen PenBorder(Color(17, 125, 187)); //边框的笔
				LinearGradientBrush BrushFill(Point(0, 0), Point(0, size.cy),
					Color(32, 241, 246, 250), Color(255, 241, 246, 250)); //填充的刷子

				//画线
				PaintProc::PaintLine(*this, graphics, PenRuler);

				//画块
                auto iMaxNetSpeed =
                    (std::max)(downSpeed.max_element(), upSpeed.max_element());
                iMaxNetSpeed = MulDiv(iMaxNetSpeed, 5, 4);
                iMaxNetSpeed = (std::max)(iMaxNetSpeed, 64);
				PaintProc::PaintArea(*this, graphics, PenBorder, BrushFill,
					downSpeed, iMaxNetSpeed, TRUE);
				PaintProc::PaintArea(*this, graphics, PenBorder, BrushFill,
					upSpeed, iMaxNetSpeed, TRUE);
				PaintProc::PaintArea(*this, graphics, PenBorder, BrushFill,
					downSpeed, iMaxNetSpeed, FALSE);
				PenBorder.SetDashStyle(DashStyle::DashStyleDash);
				PaintProc::PaintArea(*this, graphics, PenBorder, BrushFill,
					upSpeed, iMaxNetSpeed, FALSE);
				PenBorder.SetDashStyle(DashStyle::DashStyleSolid);

				Font font(&fontFamily, dpi(14), FontStyleRegular, UnitPixel);
				StringFormat sf;
				//在左上角写"网络传输速率 - xx"
				TCHAR szTemp[tk::STRING_LENGTH] = { NULL };
				Tool::FormatSpeedUnit(szTemp, tk::STRING_LENGTH, ns.GetDownloadSpeed() + ns.GetUploadSpeed());
				graphics.DrawString(szBuffer, _stprintf_s(szBuffer, TEXT("网络传输速率 - %s"),
					szTemp),
					&font, PointF(0, 0), &sf, &BrushFont);

				//在右上角写最大网速
				Tool::FormatSpeedUnit(szTemp, tk::STRING_LENGTH, iMaxNetSpeed);
				sf.SetAlignment(StringAlignment::StringAlignmentFar);
				graphics.DrawString(szBuffer, _stprintf_s(szBuffer, TEXT("%s"),
					szTemp),
					&font, PointF(size.cx, 0), &sf, &BrushFont);

				//画边框
				graphics.DrawRectangle(&PenBorder, Rect(0, 0, size.cx - 1, size.cy - 1));
			}

			//画边框
			if (bMouseHover && bSide == 1)
			{
				Pen PenBorder(Color(255, 139, 18, 174)); //边框的笔
				graphics.DrawRectangle(&PenBorder, Rect(0, 0, size.cx - 1, size.cy - 1));
			}
		}

		if (!fTransparent)
			tk::Blt(hdc, 0, 0, size.cx, size.cy, hdcMem, 0, 0, size.cx, size.cy, tk::SKIP);
		else
			tk::Blt(hdc, 0, 0, size.cx, size.cy, hdcMem, 0, 0, size.cx, size.cy, MulDiv(255, 25, 100));
		DeleteDC(hdcMem);
	}
	VOID WndStruct::OnPaint(HWND hwnd)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		OnPaint(hdc, hwnd, FALSE);
		EndPaint(hwnd, &ps);
	}
	VOID WndStruct::OnUpdateInfo(HWND hwnd)
	{
		Insert(msex.ullTotalPhys - msex.ullAvailPhys);
	}
	VOID WndStruct::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
	{
		//更新鼠标位置
		cxMouse = x;
		cyMouse = y;

		//更新鼠标相对位置
		INT side = x / (size.cx / 3);
		side = max(side, 0);
		side = min(side, 2);
		if (bSide != side)
		{
			bSide = side;
			Update();
		}
		if (bMouseDown) Update();

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
	VOID WndStruct::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		SetCapture(hwnd);

		if (x >= size.cx / 3 && x <= size.cx / 3 * 2)
		{
			bMouseDown = TRUE;
			R = 30;
			Update();
		}
	}
	VOID WndStruct::OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
	{
		ReleaseCapture();

		if (bMouseDown)
		{
			if (bSide == 0) //更改显示
			{
				iShowWhat = !iShowWhat;
			}
			else if (bSide == 2) //清理内存
			{
				mc.Clean();
				tk::WriteLogFile(TEXT("Clean"), TEXT("Cleaned the memory manually."), szLogPath);
			}

			bMouseDown = FALSE;
			Update();
		}
	}
	VOID WndStruct::OnDestroy(HWND hwnd)
	{
		delete this;
	}
}
LRESULT CALLBACK Mempad::WndStruct::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hwnd, WM_UPDATEINFO, OnUpdateInfo);
		HANDLE_MSG(hwnd, WM_MOUSEMOVE, OnMouseMove);
		HANDLE_MSG(hwnd, WM_LBUTTONDOWN, OnLButtonDown);
		HANDLE_MSG(hwnd, WM_LBUTTONUP, OnLButtonUp);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);

	case WM_MOUSELEAVE:
	{
		bMouseHover = FALSE;
		Update();
		break;
	}
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}