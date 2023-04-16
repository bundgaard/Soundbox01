#pragma once
#include "Application.h"
#include <array>
#include <sstream>
namespace tretton63
{

	constexpr wchar_t SLIDER_CLASS[] = L"SLIDERCLASS";

	class Slider
	{
		HWND m_hwnd;
		int m_minVal, m_maxVal;
		int m_position;
		HFONT m_font;
		int m_x, m_y;
		int m_width, m_height;
	public:
		static bool Register()
		{
			WNDCLASSEX wc{};
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.cbClsExtra = sizeof(LONG_PTR);
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc = (WNDPROC)Slider::SliderProc;
			wc.hInstance = GetModuleHandleW(nullptr);
			wc.lpszClassName = SLIDER_CLASS;
			return RegisterClassEx(&wc);
		}

		Slider(
			HWND Parent,
			int x, int y,
			int width, int height,
			int minVal, int maxVal,
			HFONT Font) : m_x(x), m_y(y),
			m_width(width), m_height(height),
			m_minVal(minVal), m_maxVal(maxVal),
			m_font(Font), m_position(0)
		{

			m_hwnd = CreateWindowEx(
				0,
				SLIDER_CLASS,
				L"",
				WS_CHILD | WS_VISIBLE,
				x, y,
				width, height,
				Parent,
				nullptr,
				GetModuleHandleW(nullptr),
				this);

		}

		void SetFont(HFONT hFont)
		{
			m_font = hFont;
		}

		HFONT GetFont()
		{
			return m_font;
		}

		HWND GetHandle()
		{
			return m_hwnd;
		}
		int GetPosition()
		{
			RECT Rct{};
			GetClientRect(m_hwnd, &Rct);
			int Pos = MulDiv(m_position, Rct.right - Rct.left, m_maxVal - m_minVal) + Rct.left;
		}

		int SetPosition(int newPosition)
		{
			RECT Rct{};
			GetClientRect(m_hwnd, &Rct);
			m_position = MulDiv(newPosition - Rct.left, m_maxVal - m_minVal, Rct.right - Rct.left);
			RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE);
		}

	private:
		LRESULT CALLBACK Proc(UINT msg, WPARAM wparam, LPARAM lparam)
		{
			std::wstringstream Out{};
			Out << std::hex << msg << "\n";
			OutputDebugString(Out.str().c_str());
			switch (msg)
			{
			case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(m_hwnd, &ps);
				OnPaint(hdc);
				EndPaint(m_hwnd, &ps);
			}
			return 0;

			case WM_LBUTTONDOWN:
			{
				OutputDebugString(L"Slider button down (static)\n");
				auto MouseX = GET_X_LPARAM(lparam);
				auto MouseY = GET_Y_LPARAM(lparam);
				std::array<wchar_t, 64> Buf{};
				wsprintf(Buf.data(), L"%d,%d\n", MouseX, MouseY);
				OutputDebugStringW(Buf.data());
				SetCapture(m_hwnd);
			}
			return 0;
			case WM_MOUSEMOVE:
			{
				auto MouseX = GET_X_LPARAM(lparam);
				auto MouseY = GET_Y_LPARAM(lparam);
				std::wstringstream Out{};
				Out << MouseX << L"," << MouseY << L"\n";
				//OutputDebugStringW(Out.str().c_str());
			}
			return 0;
			case WM_LBUTTONUP:
			{
				ReleaseCapture();
			}
			return 0;				
			}
			return DefWindowProc(m_hwnd, msg, wparam, lparam);
		}

		static LRESULT CALLBACK SliderProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			Slider* self = nullptr;
			if (self == nullptr && msg == WM_NCCREATE)
			{
				CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lparam);
				self = static_cast<Slider*>(createStruct->lpCreateParams);
				self->m_hwnd = hwnd;
				SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
			}
			else
			{
				self = (Slider*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
			}
			return self->Proc(msg, wparam, lparam);
		}

		void OnPaint(HDC hdc)
		{
			if (m_font)
				SelectObject(hdc, m_font);
			// SetBkMode(hdc, TRANSPARENT);

			// Slider background
			RECT SliderBackground{};
			GetClientRect(m_hwnd, &SliderBackground);
			std::wstringstream Out{};
			Out << L"ClientRect: " << SliderBackground.left << L"," << SliderBackground.top << L" " << SliderBackground.right << L"," << SliderBackground.bottom << L"\n";
			OutputDebugString(Out.str().c_str());
			HBRUSH hBackground = CreateSolidBrush(RGB(240, 240, 240));
			FillRect(hdc, &SliderBackground, hBackground);
			DeleteBrush(hBackground);

			int MarginTop = 5;
			int MarginBottom = -5;
			int MarginLeft = -5;
			int MarginRight = 5;


			// Slider Trackline
			auto SliderMiddleX = SliderBackground.left + (SliderBackground.right - SliderBackground.left) / 3;

			RECT SliderTrackbar{ SliderMiddleX, SliderBackground.top + MarginTop, SliderMiddleX, SliderBackground.bottom + MarginBottom };

			MoveToEx(hdc, SliderMiddleX, SliderTrackbar.top, nullptr);
			LineTo(hdc, SliderMiddleX, SliderTrackbar.bottom);


			// Slider max at top

			DrawTextW(hdc, L"Max\0", -1, &SliderBackground, DT_RIGHT | DT_NOPREFIX);
			// Slider min at bottom
			DrawTextW(hdc, L"Min\0", -1, &SliderBackground, DT_RIGHT | DT_SINGLELINE | DT_BOTTOM | DT_NOPREFIX);
			// Slider thumb
			RECT SliderThumb{};
			SliderThumb.left = SliderTrackbar.left + MarginLeft;
			SliderThumb.right = SliderTrackbar.right + MarginRight;
			SliderThumb.top = m_position + SliderTrackbar.bottom + MarginBottom;
			SliderThumb.bottom = m_position + SliderTrackbar.bottom;
			FillRect(hdc, &SliderThumb, (HBRUSH)GetStockObject(BLACK_BRUSH));


			// Slider Ticks

		}


	};


}



/*
#include <windows.h>

LRESULT CALLBACK CustomTrackbarWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CustomTrackbar {

private:
	HWND m_hWnd;
	int m_minVal;
	int m_maxVal;
	int m_position;

	void OnPaint(HDC hdc)
	{
		RECT rc;
		GetClientRect(m_hWnd, &rc);

		// Draw track
		HPEN hPenTrack = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
		SelectObject(hdc, hPenTrack);
		MoveToEx(hdc, rc.left, rc.bottom / 2, NULL);
		LineTo(hdc, rc.right, rc.bottom / 2);
		DeleteObject(hPenTrack);

		// Draw thumb
		int thumbPos = MulDiv(m_position, rc.right - rc.left, m_maxVal - m_minVal) + rc.left;
		RECT thumbRect = { thumbPos - 5, rc.top, thumbPos + 5, rc.bottom };
		HBRUSH hBrushThumb = CreateSolidBrush(RGB(0, 0, 255));
		FillRect(hdc, &thumbRect, hBrushThumb);
		DeleteObject(hBrushThumb);
	}

	void OnLButtonDown(int x)
	{
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		m_position = MulDiv(x - rc.left, m_maxVal - m_minVal, rc.right - rc.left);
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
	}

	void OnMouseMove(int x)
	{
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
			RECT rc;
			GetClientRect(m_hWnd, &rc);
			m_position = MulDiv(x - rc.left, m_maxVal - m_minVal, rc.right - rc.left);
			RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
		}
	}

	void OnSize(int width, int height

*/