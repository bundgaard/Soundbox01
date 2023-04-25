#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

class Frame
{
	HWND m_hwnd;
public:

	Frame(Frame&) = delete;
	Frame& operator=(Frame&) = delete;

	Frame(
		int X, int Y,
		int Width, int Height,
		HBRUSH BackgroundBrush,
		std::wstring const& Title,
		std::wstring const& Classname);

	~Frame();
	HWND Handle() const;
	
	virtual void OnPaint(HDC hdc) = 0;
	virtual void OnCreate() = 0;
	virtual LRESULT OnProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	static LRESULT CALLBACK sWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};

