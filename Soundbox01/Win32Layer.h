#pragma once
#include "Application.h"
#include <optional>

namespace tretton63
{
	ATOM Win32RegisterClass(HINSTANCE hInst, HBRUSH hbrBackground);
	HFONT Win32CreateFont(std::wstring const& Fontface, int FontSize);
	HWND Win32CreateWindow(std::wstring const& Title, int X, int Y, int Width, int Height, HINSTANCE hInst);
	std::optional<std::wstring> Win32Caption(HWND hwnd);
	HWND Win32CreateButton(HWND Parent, std::wstring const& Caption, int EventId, int X, int Y, int Width, int Height);

}