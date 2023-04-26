#include "Win32Layer.h"
#include "Styleguide.h"
#include <array>
#include <vector>


namespace tretton63
{
	ATOM
		Win32RegisterClass(
			HINSTANCE hInst,
			WNDPROC aWndProc,
			HBRUSH hbrBackground,
			std::wstring const& Classname)
	{
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.hInstance = hInst;
		wc.lpszClassName = Classname.c_str();
		wc.hbrBackground = hbrBackground;
		wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hIconSm = wc.hIcon;
		wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wc.lpfnWndProc = (WNDPROC)aWndProc;
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		return RegisterClassEx(&wc);
	}

	HFONT
		Win32CreateFont(std::wstring const& Fontface, int FontSize, int Weight = FW_NORMAL)
	{
		HFONT Font = CreateFontW(
			FontSize,
			0,
			0,
			0,
			Weight,
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
		Win32CreateWindow(std::wstring const& Title,
			int X, int Y,
			int Width, int Height,
			HINSTANCE hInst)
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

		std::wstring Text(TextLength + 1, L'\0');
		GetWindowTextW(hwnd, Text.data(), TextLength + 1);
		Text.resize(TextLength);
		return Text;
	}

	HWND
		Win32CreateButton(
			HWND Parent,
			std::wstring const& Caption,
			uintptr_t EventId,
			int X, int Y,
			int Width, int Height)
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

		// TODO: Figure out a better way of writing this.
		if (pDis->itemState & ODS_FOCUS)
		{
			auto old = SelectObject(pDis->hDC, HilitePen.Value());
			RoundRect(pDis->hDC, pDis->rcItem.left, pDis->rcItem.top, pDis->rcItem.right, pDis->rcItem.bottom, 2, 2);
			SelectObject(pDis->hDC, old);
			FillRect(pDis->hDC, &pDis->rcItem, BackgroundBrush.Value());
		}

		if (pDis->itemState & ODS_SELECTED)
		{
			FillRect(pDis->hDC, &pDis->rcItem, (HBRUSH)GetStockObject(GRAY_BRUSH));
		}
		else
		{
			auto old = SelectObject(pDis->hDC, ForegroundPen.Value());
			RoundRect(pDis->hDC, pDis->rcItem.left, pDis->rcItem.top, pDis->rcItem.right, pDis->rcItem.bottom, 2, 2);
			SelectObject(pDis->hDC, old);
			FillRect(pDis->hDC, &pDis->rcItem, BackgroundBrush.Value());

		}

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
		int Index = pDis->itemID;


		auto TextLen = ListBox_GetTextLen(pDis->hwndItem, Index);
		std::wstring Text{};
		Text.resize(TextLen);

		ListBox_GetText(pDis->hwndItem, Index, Text.data());
		if (pDis->itemState & ODS_SELECTED || pDis->itemState & ODS_FOCUS)
		{
			FillRect(pDis->hDC, &pDis->rcItem, (HBRUSH)GetStockObject(WHITE_BRUSH));
		}
		else
		{
			FillRect(pDis->hDC, &pDis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
		}

		SetTextColor(pDis->hDC, ForegroundColor);
		DrawTextW(pDis->hDC, Text.c_str(), static_cast<int>(Text.size()), const_cast<LPRECT>(&pDis->rcItem), DT_SINGLELINE | DT_NOPREFIX);
	}

	void Win32DrawText(HDC hdc, std::wstring Text, RECT* BoundingBox, UINT Format, COLORREF color)
	{

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