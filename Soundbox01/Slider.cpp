#include "Slider.h"
#include <array>
#include "Styleguide.h"

namespace tretton63
{
	constexpr int ThumbWidth = 18;
	constexpr int ThumbHeight = 18;
	constexpr wchar_t ThumbCaption_1[] = L"MAX";
	constexpr wchar_t ThumbCaption_2[] = L"MIN";

	Local int nPos;
	Local bool MouseHeld;

	Local void
		Slider_DrawText(HDC hdc, RECT* SliderRect)
	{
		SetTextColor(hdc, ForegroundColor);
		// Slider max at top
		DrawTextW(hdc, ThumbCaption_1, -1, SliderRect, DT_RIGHT | DT_NOPREFIX);
		// Slider min at bottom
		DrawTextW(hdc, ThumbCaption_2, -1, SliderRect, DT_RIGHT | DT_SINGLELINE | DT_BOTTOM | DT_NOPREFIX);
	}

	Local void
		Slider_DrawThumb(HDC hdc, RECT* SliderRect, RECT* Margin, int x, int y)
	{
		// Slider thumb
		RECT SliderThumb{ 0,0,ThumbWidth, ThumbHeight };
		OffsetRect(&SliderThumb, x, y);

		if (SliderThumb.top <= Margin->top)
		{

			SliderThumb.top = Margin->top;
			SliderThumb.bottom = Margin->top + 18;

			DrawFrameControl(hdc, &SliderThumb, DFC_SCROLL, DFCS_PUSHED | DFCS_SCROLLDOWN | DFCS_FLAT);
		}
		else if (SliderThumb.bottom >= SliderRect->bottom + Margin->bottom)
		{
			SliderThumb.bottom = SliderRect->bottom + Margin->bottom;
			SliderThumb.top = SliderThumb.bottom - 18;
			DrawFrameControl(hdc, &SliderThumb, DFC_SCROLL, DFCS_PUSHED | DFCS_SCROLLUP | DFCS_FLAT);
		}
		else
		{
			DrawFrameControl(hdc, &SliderThumb, DFC_SCROLL, DFCS_PUSHED | DFCS_SCROLLRIGHT | DFCS_FLAT);
		}
	}

	Local void
		Slider_DrawTrackline(HDC hdc, RECT* SliderBackground, RECT* Margin)
	{
		auto Old = SelectObject(hdc, TracklineColor.Value());
		int x = SliderBackground->left + (SliderBackground->right - SliderBackground->left) / 4 + 2;
		POINT TrackLine{ x, SliderBackground->top };
		MoveToEx(hdc, TrackLine.x, TrackLine.y + Margin->top, nullptr);
		LineTo(hdc, TrackLine.x, TrackLine.y + SliderBackground->bottom + Margin->bottom);
		SelectObject(hdc, Old);
	}

	Local void
		Slider_DrawBackground(HDC hdc, RECT* SliderBackground, COLORREF color)
	{
		SetBkMode(hdc, TRANSPARENT);
		FillRect(hdc, SliderBackground, BackgroundBrush.Value());
	}

	Local inline RECT
		Slider_GetRect(HWND hwnd)
	{
		RECT SliderBackground{};
		GetClientRect(hwnd, &SliderBackground);

		return SliderBackground;
	}
	Local void
		Slider_OnPaint(HWND hwnd, HDC hdc)
	{
		SelectObject(hdc, ButtonFont);

		RECT SliderBackground = Slider_GetRect(hwnd);
		RECT Margin{ -5, 5, 5, -5 };

		int x = SliderBackground.left + (SliderBackground.right - SliderBackground.left) / 4 + 2;
		Slider_DrawBackground(hdc, &SliderBackground, RGB(240, 240, 240));
		Slider_DrawTrackline(hdc, &SliderBackground, &Margin);
		Slider_DrawText(hdc, &SliderBackground);
		Slider_DrawThumb(hdc, &SliderBackground, &Margin, x, nPos);
	}

	Local LRESULT CALLBACK
		SliderProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			Slider_OnPaint(hwnd, hdc);
			EndPaint(hwnd, &ps);
		}
		return 0;
		case WM_ERASEBKGND:
			return 1;
		case WM_LBUTTONDOWN:
		{
			OutputDebugString(L"Slider button down\n");
			auto MouseX = GET_X_LPARAM(lparam);
			auto MouseY = GET_Y_LPARAM(lparam);
			std::array<wchar_t, 64> Buf{};
			wsprintf(Buf.data(), L"%d,%d\n", MouseX, MouseY);
			OutputDebugStringW(Buf.data());
			nPos = MouseY;
			MouseHeld = true;
			SetCapture(hwnd);
		}
		return 0;
		case WM_LBUTTONUP:
		{
			OutputDebugStringW(L"Slider button up\n");
			auto MouseX = GET_X_LPARAM(lparam);
			auto MouseY = GET_Y_LPARAM(lparam);
			std::array<wchar_t, 64> Buf{};
			wsprintf(Buf.data(), L"%d,%d\n", MouseX, MouseY);
			OutputDebugStringW(Buf.data());
			MouseHeld = false;
			nPos = MouseY;
			ReleaseCapture();
		}
		return 0;
		case WM_MOUSEMOVE:
		{
			auto MouseX = GET_X_LPARAM(lparam);
			auto MouseY = GET_Y_LPARAM(lparam);
			if (MouseHeld)
			{
				nPos = MouseY;
				SendMessage(GetParent(hwnd), (WM_USER + 5), max(min(nPos, 100), 0), 0); // TODO: fix when we have a better structure.
				InvalidateRect(hwnd, NULL, FALSE);
			}
			

		}
		return 0;
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
	}

	bool
		SliderRegisterClass(HINSTANCE hInst)
	{
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = &SliderProc;
		wc.lpszClassName = L"Slider";
		wc.hInstance = hInst;

		return RegisterClassEx(&wc);
	}

	HWND
		SliderCreateWindow(
			HWND Parent,
			HINSTANCE Instance,
			int X, int Y,
			int Width, int Height,
			int MaxVal, int MinVal) // TODO: make a struct or class to collect all of this and make it available in the clamp function
	{
		HWND hwnd = CreateWindowEx(0,
			L"Slider",
			L"",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			X, Y,
			Width, Height,
			Parent,
			nullptr,
			Instance,
			nullptr);
		return hwnd;
	}

}