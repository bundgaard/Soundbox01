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
}