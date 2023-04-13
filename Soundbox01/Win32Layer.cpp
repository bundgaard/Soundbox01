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

	std::optional<std::wstring> Win32Caption(HWND hwnd)
	{
		std::wstring Text{};
		auto TextLength = GetWindowTextLengthW(hwnd);
		if (TextLength == 0)
		{
			DWORD dwError = GetLastError();
			if (dwError != 0x0)
			{
				// TODO: add more error handling
				OutputDebugString(L"failed to get the text length from window control\n");
				return std::nullopt;
			}
		}

		Text.resize((size_t)TextLength + 1, L'\0');
		GetWindowTextW(hwnd, Text.data(), Text.size());
		return Text;
	}

	HWND
	Win32CreateButton(HWND Parent, std::wstring const& Caption, int EventId, int X, int Y, int Width, int Height)
	{
		return CreateWindow(L"BUTTON", Caption.c_str(),
			WS_VISIBLE | WS_CHILD,
			X, Y, Width, Height, Parent,
			(HMENU)EventId,
			GetModuleHandle(nullptr),
			nullptr);

	}
}