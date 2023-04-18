#pragma once
#include "Application.h"

extern HFONT ButtonFont;

namespace tretton63
{
	bool SliderRegisterClass(HINSTANCE hInst);
	HWND SliderCreateWindow(
		HWND Parent, HINSTANCE Instance,
		int X,
		int Y,
		int Width,
		int Height,
		int MaxVal, int MinVal);
}
