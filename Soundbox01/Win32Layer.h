#pragma once
#include "Application.h"
#include <optional>

namespace tretton63
{
	ATOM Win32RegisterClass(HINSTANCE hInst, HBRUSH hbrBackground);
	HFONT Win32CreateFont(std::wstring const& Fontface, int FontSize);
	HWND Win32CreateWindow(std::wstring const& Title, int X, int Y, int Width, int Height, HINSTANCE hInst);
	std::optional<std::wstring> Win32Caption(HWND hwnd);
	HWND Win32CreateButton(HWND Parent, std::wstring const& Caption, uintptr_t EventId, int X, int Y, int Width, int Height);
	void Win32SetFont(HWND hwnd, HFONT Font);
	void Win32CustomButton(DRAWITEMSTRUCT const* pDis);
	void Win32CustomListBox(DRAWITEMSTRUCT const* pDis);
	HWND Win32CreateListbox(HWND Parent, int X, int Y, int Width, int Height);
}