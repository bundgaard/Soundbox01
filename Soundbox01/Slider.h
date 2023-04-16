#pragma once
#include "Application.h"

LRESULT CALLBACK SliderProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

class Slider
{
	HWND m_hwnd;
	int m_minVal, m_maxVal;
	int m_position;
public:
	Slider(HWND Parent, int x, int y, int width, int height, int minVal, int maxVal)
	{
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = &SliderProc;
		wc.hInstance = GetModuleHandleW(nullptr);
		wc.lpszClassName = L"Slider";
		RegisterClassEx(&wc);

		m_hwnd = CreateWindowEx(0, wc.lpszClassName, L"", WS_CHILD | WS_VISIBLE, x, y, width, height, Parent, nullptr, GetModuleHandleW(nullptr), nullptr);
		m_minVal = minVal;
		m_maxVal = maxVal;
		m_position = 0;
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
	void OnPaint(HDC hdc)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(m_hwnd, &ps);

		RECT Rct{};
		GetClientRect(m_hwnd, &Rct);
		int ClientWidth = Rct.right - Rct.left;
		int ClientHeight = Rct.bottom - Rct.top;

		HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(240, 240, 240));
		FillRect(hdc, &ps.rcPaint, hBackgroundBrush);
		DeleteObject(hBackgroundBrush);

		// Paint the track

		int trackLeft = 10;
		int trackTop = (ClientHeight - 16) / 2; // TODO: fix magic number
		int trackRight = ClientWidth - 10;
		int trackBottom = trackTop + 16;

		HBRUSH hTrackBrush = CreateSolidBrush(RGB(200, 200, 200));
		RECT TrackRect = RECT{ trackLeft, trackTop, trackRight, trackBottom };
		FillRect(hdc, &TrackRect, hTrackBrush);
		DeleteObject(hTrackBrush);


		// Paint the thumb
		int thumbWidth = 20;
		int thumbHeight = 20;
		// TODO Need to be defined, should possible be m_position; int thumbLeft = thumbPos
		EndPaint(m_hwnd, &ps);
	}


};



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