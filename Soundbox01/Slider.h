#pragma once
#include "Application.h"
#include "WindowMessages.h"
#include "Styleguide.h"
#include <array>

namespace tretton63
{
	constexpr int ThumbWidth = 18;
	constexpr int ThumbHeight = 18;
	constexpr wchar_t ThumbCaption_1[] = L"MAX";
	constexpr wchar_t ThumbCaption_2[] = L"MIN";

	class Slider
	{
		HWND m_hwnd;
		bool m_mouseHeld;
		int nPos;

	public:
		Slider(HWND Parent, int x, int y, int width, int height);

		~Slider();

		HWND Handle() const;

	private:
		void OnPaint(HWND hwnd, PAINTSTRUCT& ps);

		void
			Slider_DrawText(HDC hdc, RECT* SliderRect);

		void
			Slider_DrawThumb(HDC hdc, RECT* SliderRect, RECT* Margin, int x, int y);

		void
			Slider_DrawTrackline(HDC hdc, RECT* SliderBackground, RECT* Margin);

		void
			Slider_DrawBackground(HDC hdc, RECT* SliderBackground, COLORREF color);

		inline RECT
			Slider_GetRect(HWND hwnd);
		LRESULT OnProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	};
}
