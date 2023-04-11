#include "Win32Layer.h"
namespace tretton63
{
	ATOM Win32RegisterClass(HINSTANCE hInst)
	{
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.hInstance = hInst;
		wc.lpszClassName = CCLASSNAME;
		wc.hbrBackground = (HBRUSH)GetStockObject(COLOR_APPWORKSPACE + 1);
		wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hIconSm = wc.hIcon;
		wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wc.lpfnWndProc = &SoundboxProc;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		return RegisterClassEx(&wc);
	}

	HFONT Win32CreateFont(std::wstring const& Fontface, int FontSize)
	{
		HFONT Font = CreateFontW(
			FontSize,
			0,
			0,
			0,
			FW_NORMAL,
			false,
			false,
			false,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH,
			Fontface.c_str());
		return Font;
	}

	HWND Win32CreateWindow(std::wstring const& Title, int X, int Y, int Width, int Height, HINSTANCE hInst)
	{
		return CreateWindowEx(
			WS_EX_OVERLAPPEDWINDOW,
			CCLASSNAME,
			Title.c_str(),
			WS_OVERLAPPEDWINDOW,
			X, Y, Width, Height,
			nullptr,
			nullptr,
			hInst,
			nullptr);
	}
}