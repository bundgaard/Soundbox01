#include "Win32Layer.h"
#include "Styleguide.h"
#include <array>

namespace tretton63
{
	ATOM 
		Win32RegisterClass(HINSTANCE hInst, HBRUSH hbrBackground = (HBRUSH)GetStockObject(COLOR_APPWORKSPACE + 1))
	{
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.hInstance = hInst;
		wc.lpszClassName = CCLASSNAME;
		wc.hbrBackground = hbrBackground;
		wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hIconSm = wc.hIcon;
		wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wc.lpfnWndProc = &SoundboxProc;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		return RegisterClassEx(&wc);
	}

	HFONT 
		Win32CreateFont(std::wstring const& Fontface, int FontSize)
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

	HWND 
		Win32CreateWindow(std::wstring const& Title, int X, int Y, int Width, int Height, HINSTANCE hInst)
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

	std::optional<std::wstring> 
		Win32Caption(HWND hwnd)
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
		GetWindowTextW(hwnd, Text.data(), static_cast<int>(Text.size()));
		return Text;
	}

	HWND
		Win32CreateButton(HWND Parent, std::wstring const& Caption, uintptr_t EventId, int X, int Y, int Width, int Height)
	{
		return CreateWindow(L"BUTTON", Caption.c_str(),
			WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
			X, Y, Width, Height, Parent,
			(HMENU)(EventId),
			GetModuleHandle(nullptr),
			nullptr);

	}



	void 
		Win32SetFont(HWND hwnd, HFONT Font)
	{
		SendMessage(hwnd, WM_SETFONT, (WPARAM)Font, 0);
	}


	void
		Win32CustomButton(DRAWITEMSTRUCT const* pDis)
	{
		FillRect(pDis->hDC, &pDis->rcItem, BackgroundBrush.Value());
		// TODO: somethign weird with the SELECTION and focus... might need to have ifs instead.
		auto old = SelectObject(pDis->hDC, ForegroundPen.Value());
		RoundRect(pDis->hDC, pDis->rcItem.left, pDis->rcItem.top, pDis->rcItem.right, pDis->rcItem.bottom, 2, 2);
		SelectObject(pDis->hDC, old);

		SetTextColor(pDis->hDC, ForegroundColor);

		// TODO: fix state drawing and also some nicer background
		auto Caption = Win32Caption(pDis->hwndItem);
		if (Caption.has_value())
			DrawTextW(pDis->hDC,
				Caption->c_str(),
				static_cast<int>(Caption->size()),
				const_cast<LPRECT>(&pDis->rcItem),
				DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	void
		Win32CustomListBox(DRAWITEMSTRUCT const* pDis)
	{
		OutputDebugStringW(L"ODT LISTBOX\n");
		int Index = pDis->itemID;
		std::array<wchar_t, 1024> Buf{};
		ListBox_GetText(pDis->hwndItem, Index, Buf.data());
		if (pDis->itemState & ODS_SELECTED || pDis->itemState & ODS_FOCUS)
		{
			FillRect(pDis->hDC, &pDis->rcItem, (HBRUSH)GetStockObject(WHITE_BRUSH));
		}
		else
		{
			FillRect(pDis->hDC, &pDis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
		}
		OutputDebugStringW(L"Music list\n");
		SetTextColor(pDis->hDC, ForegroundColor);
		DrawTextW(pDis->hDC, Buf.data(), -1, const_cast<LPRECT>(&pDis->rcItem), DT_SINGLELINE | DT_NOPREFIX);
	}

	HWND
		Win32CreateListbox(HWND Parent, int X, int Y, int Width, int Height)
	{
		return CreateWindow(
			L"LISTBOX",
			L"",
			WS_CHILD | WS_BORDER | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS,
			X, Y, Width, Height,
			Parent,
			0,
			GetModuleHandleW(0),
			nullptr);
	}
}