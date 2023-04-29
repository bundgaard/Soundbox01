#include "Slider.h"
#include <array>
#include "Styleguide.h"

namespace tretton63
{
	Slider::Slider(HWND Parent, int x, int y, int width, int height)
	{
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = (WNDPROC)&Slider::WndProc;
		wc.lpszClassName = L"TRETTON63_SLIDER";
		wc.hInstance = GetModuleHandleW(0);

		RegisterClassEx(&wc);

		m_hwnd = CreateWindowEx(
			0,
			wc.lpszClassName,
			L"",
			WS_VISIBLE | WS_CHILD | WS_BORDER,
			x, y,
			width, height,
			Parent,
			nullptr,
			nullptr,
			nullptr);
		SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		m_mouseHeld = false;
		nPos = 0;
		m_font = nullptr;

	}

	Slider::~Slider()
	{
		DestroyWindow(m_hwnd);
	}

	HWND Slider::Handle() const
	{
		return m_hwnd;
	}

	inline void Slider::OnPaint(HWND hwnd, PAINTSTRUCT& ps)
	{
		RECT SliderBackground = Slider_GetRect(hwnd);
		RECT Margin{ -5, 5, 5, -5 };

		int x = SliderBackground.left + (SliderBackground.right - SliderBackground.left) / 4 + 2;
		Slider_DrawBackground(ps.hdc, &SliderBackground, RGB(240, 240, 240));
		Slider_DrawTrackline(ps.hdc, &SliderBackground, &Margin);
		Slider_DrawText(ps.hdc, &SliderBackground);
		Slider_DrawThumb(ps.hdc, &SliderBackground, &Margin, x, nPos);
	}

	inline void Slider::Slider_DrawText(HDC hdc, RECT* SliderRect)
	{
		SetTextColor(hdc, ForegroundColor);
		// Slider max at top
		DrawTextW(hdc, ThumbCaption_1, -1, SliderRect, DT_RIGHT | DT_NOPREFIX);
		// Slider min at bottom
		DrawTextW(hdc, ThumbCaption_2, -1, SliderRect, DT_RIGHT | DT_SINGLELINE | DT_BOTTOM | DT_NOPREFIX);
	}

	inline void Slider::Slider_DrawThumb(HDC hdc, RECT* SliderRect, RECT* Margin, int x, int y)
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

	inline void Slider::Slider_DrawTrackline(HDC hdc, RECT* SliderBackground, RECT* Margin)
	{
		auto Old = SelectObject(hdc, TracklineColor.Value());
		int x = SliderBackground->left + (SliderBackground->right - SliderBackground->left) / 4 + 2;
		POINT TrackLine{ x, SliderBackground->top };
		MoveToEx(hdc, TrackLine.x, TrackLine.y + Margin->top, nullptr);
		LineTo(hdc, TrackLine.x, TrackLine.y + SliderBackground->bottom + Margin->bottom);
		SelectObject(hdc, Old);
	}

	inline void Slider::Slider_DrawBackground(HDC hdc, RECT* SliderBackground, COLORREF color)
	{
		SetBkMode(hdc, TRANSPARENT);
		FillRect(hdc, SliderBackground, BackgroundBrush.Value());
	}

	inline RECT Slider::Slider_GetRect(HWND hwnd)
	{
		RECT SliderBackground{};
		GetClientRect(hwnd, &SliderBackground);

		return SliderBackground;
	}

	inline LRESULT Slider::OnProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			if (m_font != nullptr)
			{
				SelectObject(hdc, m_font);
			}
			OnPaint(hwnd, ps);
			EndPaint(hwnd, &ps);
		}
		return 0;
		case WM_SETFONT:
		{
			HFONT Font = (HFONT)wparam;
			BOOL bRedraw = (BOOL)lparam;
			OutputDebugStringW(L"Recieved the WM_SETFONT\n");
			if (m_font != Font)
			{
				m_font = Font;
				InvalidateRect(hwnd, nullptr, bRedraw);
			}
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
			m_mouseHeld = true;
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
			m_mouseHeld = false;
			nPos = MouseY;
			ReleaseCapture();
		}
		return 0;
		case WM_MOUSEMOVE:
		{
			auto MouseX = GET_X_LPARAM(lparam);
			auto MouseY = GET_Y_LPARAM(lparam);
			if (m_mouseHeld)
			{
				nPos = MouseY;
				SendMessage(
					GetParent(hwnd),
					WM_VOLUME_CHANGED,
					std::max(std::min(nPos, 100), 0), 0); // TODO: fix when we have a better structure.
				InvalidateRect(hwnd, NULL, FALSE);
			}
		}
		return 0;
		default:
			return DefWindowProcW(hwnd, msg, wparam, lparam);
		}
		return DefWindowProcW(hwnd, msg, wparam, lparam);
	}

	inline LRESULT Slider::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		Slider* pSlider = reinterpret_cast<Slider*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if (pSlider != nullptr)
		{
			return pSlider->OnProc(hwnd, msg, wparam, lparam);
		}
		return DefWindowProcW(hwnd, msg, wparam, lparam);

	}

}