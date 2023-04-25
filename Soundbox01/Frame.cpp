#include "Frame.h"

Frame::Frame(
	int X, int Y, 
	int Width, int Height, 
	HBRUSH BackgroundBrush, 
	std::wstring const& Title, 
	std::wstring const& Classname)
{
	WNDCLASSEX wc{};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandleW(nullptr);
	wc.lpszClassName = Classname.c_str();
	wc.hbrBackground = BackgroundBrush;
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wc.lpfnWndProc = (WNDPROC)Frame::sWndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClassEx(&wc);

	m_hwnd = CreateWindowExW(
		WS_EX_OVERLAPPEDWINDOW,
		wc.lpszClassName,
		Title.c_str(),
		WS_OVERLAPPEDWINDOW,
		X, Y,
		Width, Height,
		nullptr, nullptr,
		wc.hInstance,
		nullptr);
	SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));


}

Frame::~Frame()
{
	DestroyWindow(m_hwnd);
}

HWND Frame::Handle() const {
	return m_hwnd;
}

inline LRESULT Frame::OnProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		OnCreate();
	}
	return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps{};
		BeginPaint(hwnd, &ps);
		OnPaint(ps.hdc);
		EndPaint(hwnd, &ps);
	}
	return 0;

	default:
		return DefWindowProcW(hwnd, msg, wparam, lparam);
	}
}

inline LRESULT Frame::sWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	Frame* pFrame = reinterpret_cast<Frame*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
	if (pFrame != nullptr)
	{
		return pFrame->OnProc(hwnd, msg, wparam, lparam);
	}
	return DefWindowProcW(hwnd, msg, wparam, lparam);

}
