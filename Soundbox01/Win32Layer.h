#pragma once
#include "Application.h"

namespace tretton63
{
	ATOM Win32RegisterClass(HINSTANCE hInst);
	HFONT Win32CreateFont(std::wstring const& Fontface, int FontSize);
	HWND Win32CreateWindow(std::wstring const& Title, int X, int Y, int Width, int Height, HINSTANCE hInst);
}