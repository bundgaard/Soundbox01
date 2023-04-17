#include "Application.h"
#include "WaveReader.h"

#include <vector>
#include <sstream>
#include <memory>
#include <optional>
#include <iostream> 
#include <array>

#include <cstdio>
#include <cstdlib>
#include <cstdint>


#include "Defer.h"
#include "Win32Layer.h"
#include "Files.h"
#include "Slider.h"

/*
Chart
High Value rounded up
Min value rounded down
For .wav I can go with unsigned so from 0 to UINT?_MAX + nearest 0 uint16_t::max = 65 536 -> nearest thousand would be 70 000.

Then we need a timeline, conversely that could be each data point or we can take the average bytes per second and read that and plot. This will give us each second as a natual time line.
Simplicity is the key so now we could plot that data line white background and black line.

Another thing to think about is how to limit the time line to our width, Excel plots large amount of numbers in intervals and smaller amounts directly.

*/

/* A
Comment
On
Multiple
Lines
*/

// Comment on a single line


consteval int32_t to_rgb(int32_t ColorHex) // 00 36 00 ff
{

	int R = (ColorHex >> 24) & 0xff;
	int G = (ColorHex >> 16) & 0xff;
	int B = (ColorHex >> 8) & 0xff;
	return RGB(R, G, B);
}

static_assert(to_rgb(0x003600ff) == 0x003600);

using namespace tretton63;

Global CComPtr<IXAudio2> audio = nullptr;
Global IXAudio2MasteringVoice* master;
Global IXAudio2SourceVoice* voice1;

Global HWND PauseAndPlayButton;
Global HWND VoiceOneGetState;
Global HWND MusicFile;
Global HWND SoundProgress;
Global HWND LoadFilesToList;
Global HWND MusicList;
Global HWND VolumeFader;
Global HFONT ButtonFont;

Global WAVEFORMATEX WaveFormatEx;

std::optional<WAVEDATA> g_Data;

Global HANDLE hEvent = nullptr;
Global PTP_WORK WorkItem = nullptr;
Global bool MusicLoaded;

//////////////////////////////////////////////////
//////// SLIDER

Local void Slider_OnPaint(HWND hwnd, HDC hdc)
{
	SelectObject(hdc, ButtonFont);
	SetBkMode(hdc, TRANSPARENT);

	// Slider background
	RECT SliderBackground{};
	GetClientRect(hwnd, &SliderBackground);
	HBRUSH hBackground = CreateSolidBrush(RGB(240, 240, 240));
	FillRect(hdc, &SliderBackground, hBackground);
	DeleteBrush(hBackground);

	int MarginTop = 5;
	int MarginBottom = -5;
	int MarginLeft = -5;
	int MarginRight = 5;


	// Slider Trackline
	auto SliderMiddleX = SliderBackground.left + (SliderBackground.right - SliderBackground.left) / 3;

	RECT SliderTrackbar{ SliderMiddleX, SliderBackground.top + MarginTop, SliderMiddleX, SliderBackground.bottom + MarginBottom };

	MoveToEx(hdc, SliderMiddleX, SliderTrackbar.top, nullptr);
	LineTo(hdc, SliderMiddleX, SliderTrackbar.bottom);


	// Slider max at top

	DrawTextW(hdc, L"Max\0", -1, &SliderBackground, DT_RIGHT | DT_NOPREFIX);
	// Slider min at bottom
	DrawTextW(hdc, L"Min\0", -1, &SliderBackground, DT_RIGHT | DT_SINGLELINE | DT_BOTTOM | DT_NOPREFIX);
	// Slider thumb
	RECT SliderThumb{};
	SliderThumb.left = SliderTrackbar.left - 15;
	SliderThumb.right = SliderTrackbar.right + 15;
	SliderThumb.top = SliderTrackbar.bottom - 15;
	SliderThumb.bottom = SliderTrackbar.bottom;
	//FillRect(hdc, &SliderThumb, (HBRUSH)GetStockObject(BLACK_BRUSH));
	DrawFrameControl(hdc, &SliderThumb, DFC_SCROLL , DFCS_PUSHED| DFCS_SCROLLUP| DFCS_FLAT);
}
Local LRESULT CALLBACK SliderProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		Slider_OnPaint(hwnd, hdc);
		EndPaint(hwnd, &ps);
	}
	return 0;
	case WM_CTLCOLORLISTBOX:
	{

	}
	return 0;
	case WM_LBUTTONDOWN:
	{
		OutputDebugString(L"Slider button down\n");
		auto MouseX = GET_X_LPARAM(lparam);
		auto MouseY = GET_Y_LPARAM(lparam);
		std::array<wchar_t, 64> Buf;
		wsprintf(Buf.data(), L"%d,%d\n", MouseX, MouseY);
		OutputDebugStringW(Buf.data());
	}
	return 0;
	case WM_LBUTTONUP:
	{
	
	}
	return 0;
	default:
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

Local bool
SliderRegisterClass(HINSTANCE hInst)
{
	WNDCLASSEX wc{};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = &SliderProc;
	wc.lpszClassName = L"Slider";
	wc.hInstance = hInst;

	return RegisterClassEx(&wc);
}

Local HWND
SliderCreateWindow(HWND Parent, HINSTANCE Instance, int X, int Y, int Width, int Height, int MaxVal, int MinVal)
{
	HWND hwnd = CreateWindowEx(0, 
		L"Slider", 
		L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER,
		X, Y, 
		Width, Height, 
		Parent, 
		nullptr, 
		Instance,
		nullptr);
	return hwnd;
}

//////////////////////////////////////////////////


void NTAPI
TaskHandler(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
	wchar_t Buf[64] = { 0 };
	wsprintf(Buf, L"Starting new work\n");
	OutputDebugStringW(Buf);
	memset(Buf, 0, sizeof(wchar_t) * 64);
	wsprintf(Buf, L"WaveFormatEx information %d; Size=%zd\n", WaveFormatEx.nChannels, g_Data.has_value() ? g_Data->WaveSize : 0);
	OutputDebugString(Buf);

}


Local std::unique_ptr<XAUDIO2_BUFFER>
LoadBuffer(WAVEDATA const& Data)
{
	std::unique_ptr<XAUDIO2_BUFFER> XBuffer = std::make_unique<XAUDIO2_BUFFER>();

	if (Data.WaveSize <= UINT32_MAX)
		XBuffer->AudioBytes = static_cast<uint32_t>(Data.WaveSize);
	else
	{
		OutputDebugString(L"Failed to resize the WaveSize\n");
	}
	XBuffer->Flags = XAUDIO2_END_OF_STREAM;
	XBuffer->pAudioData = (LPBYTE)Data.Location;
	return XBuffer;
}

Local void
PlayAndPause_OnClick(HWND self, HWND parent)
{
	auto Text = Win32Caption(self);
	auto Index = ListBox_GetCurSel(MusicList);
	auto Selection = ListBox_GetCurSel(MusicList);
	Local int PreviousIndex;
	// TODO: figure out how to release the buffer and reload...
	if (Index != -1 && wcscmp(Text->c_str(), L"Play\0") == 0)
	{
		wchar_t Buf[255] = { 0 };
		ListBox_GetText(MusicList, Index, Buf);

		SetWindowTextW(self, L"Pause");
		{

			HRESULT hr = S_OK;
			// Ask for current selection of MusicList

			OutputDebugStringW(L"");

			if (voice1 == nullptr)
			{
				auto Data = LoadWaveMMap(&WaveFormatEx, Buf);

				if (Data.has_value())
				{

					if (SUCCEEDED(hr))
					{
						OutputDebugString(L"CreateSourceVoice\n");
						hr = audio->CreateSourceVoice(&voice1, &WaveFormatEx, 0, XAUDIO2_DEFAULT_FREQ_RATIO);
					}

					if (Data->Location)
					{
						OutputDebugStringW(L"Try and play the note");
						auto XBuffer = LoadBuffer(Data.value());
						voice1->SetVolume(1.0f, XAUDIO2_COMMIT_NOW);
						hr = voice1->SubmitSourceBuffer(XBuffer.get());
						MusicLoaded = true;
						g_Data = Data;
						SubmitThreadpoolWork(WorkItem);
					}


					if (FAILED(hr))
					{
						DWORD dwError = GetLastError();
						if (dwError != 0x00)
						{
							printf("0x%x\n", dwError);
							printf("failed to create Voice\n");
						}
					}
				}
			}

			if (voice1 && MusicLoaded)
			{
				if (SUCCEEDED(hr))
				{
					hr = voice1->Start();
				}
			}
		}
	}
	else
	{
		if (!MusicLoaded)
			return;
		SetWindowTextW(self, L"Play");

		voice1->Stop();
	}
}

Local void
VoiceOne_OnClick(HWND self)
{
	if (MusicLoaded)
	{
		XAUDIO2_VOICE_STATE state{};
		voice1->GetState(&state, 0);
		std::wstringstream Out{};
		Out << L"Samples played: " << state.SamplesPlayed << "\n" << L"Buffers queued: " << state.BuffersQueued << "\n";
		OutputDebugString(Out.str().c_str());
	}

}

Local void Win32SetFont(HWND hwnd, HFONT Font)
{
	SendMessage(hwnd, WM_SETFONT, (WPARAM)Font, 0);
}

Local SIZE Win32TextMeasure()
{
	return {};
}


LRESULT CALLBACK
SoundboxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	std::optional<tretton63::WAVEDATA> Data;


	switch (msg)
	{
	case WM_CREATE:
	{
		HRESULT hr = S_OK;
		hEvent = CreateEvent(nullptr, true, false, nullptr);
		ButtonFont = Win32CreateFont(L"Tahoma", 14);
		int posX = 10;
		int posY = 10;
		int Width = 100;
		int Height = 25;
		PauseAndPlayButton = Win32CreateButton(hwnd, L"Play", PauseAndPlayEvent, posX, posY, Width, Height);
		posY += Height;
		VoiceOneGetState = Win32CreateButton(hwnd, L"Voice1 State", VoiceOneGetStateEvent, posX, posY, Width, Height);
		posY += Height;
		auto LoadFilesToList = Win32CreateButton(hwnd, L"Load from path", (WM_USER + 4), posX, posY, Width, Height);
		posY += Height;
		MusicFile = CreateWindow(L"EDIT",
			L"C:\\Code",
			WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
			posX, posY,
			Width, Height,
			hwnd,
			(HMENU)WM_USER + 3,
			GetModuleHandleW(nullptr),
			nullptr);
		posY += Height;



		auto MusicLocationCaption = Win32Caption(MusicFile);

		HDC hdc = GetDC(MusicFile);
		TEXTMETRICW Metrics{};
		GetTextMetricsW(hdc, &Metrics);
		SelectObject(hdc, ButtonFont);

		SIZE S{};
		GetTextExtentPoint32W(hdc, MusicLocationCaption->c_str(), MusicLocationCaption->size(), &S);
		SetWindowPos(MusicFile, nullptr, 0, 0, S.cx, S.cy + 5, SWP_NOMOVE | SWP_NOACTIVATE);
		ReleaseDC(MusicFile, hdc);
		MusicList = CreateWindow(L"LISTBOX", L"", WS_CHILD | WS_BORDER, posX, posY, Width, 150, hwnd, 0, GetModuleHandleW(0), nullptr);

		Win32SetFont(PauseAndPlayButton, ButtonFont);
		Win32SetFont(VoiceOneGetState, ButtonFont);
		Win32SetFont(MusicFile, ButtonFont);
		Win32SetFont(LoadFilesToList, ButtonFont);
		Win32SetFont(MusicList, ButtonFont);
		/////////////////////////////////////

		VolumeFader = CreateWindowEx(
			0,
			TRACKBAR_CLASS,
			L"",
			WS_VISIBLE | WS_CHILD | TBS_VERT,
			150, 150,
			64, 128,
			hwnd,
			0,
			GetModuleHandleW(0),
			nullptr);
		SendMessage(VolumeFader, TBM_SETRANGEMIN, false, 0);
		SendMessage(VolumeFader, TBM_SETRANGEMAX, false, (LPARAM)100);
		SendMessage(VolumeFader, TBM_SETPOS, true, (LPARAM)50);
	
		HWND VolumeFader02 = SliderCreateWindow(hwnd, GetModuleHandleW(0), 300, 150, 64, 128, 100, 0);
	
		// SetWindowLongPtr(VolumeFader, GWLP_USERDATA, i);
		/////////////////////////////////////

		if (SUCCEEDED(hr))
		{
			hr = XAudio2Create(&audio);
		}

		if (SUCCEEDED(hr))
		{
			OutputDebugString(L"Create Master voice\n");
			hr = audio->CreateMasteringVoice(&master, 2, 44100);
		}

	}
	return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		


		EndPaint(hwnd, &ps);
	}
	return 0;
	case WM_LBUTTONDOWN:
	{

		// TODO: get the coordinates and see if we are under a Child control.
		// TODO: Move this code into a subclass of the control, as we need to send back an event to move the object...
		int X = GET_X_LPARAM(lParam);
		int Y = GET_Y_LPARAM(lParam);
		POINT Pt{ .x = X, .y = Y };
		wchar_t Buf[64] = { 0 };
		wsprintf(Buf, L"Mapped: %d,%d\n", X, Y);
		OutputDebugString(Buf);

		HWND Child = ChildWindowFromPointEx(hwnd, Pt, CWP_ALL);
		RECT ChildRect{};
		WINDOWINFO Info{};
		GetWindowInfo(Child, &Info);

		OutputDebugString(L"");
		if (Child == VolumeFader)
		{
			OutputDebugString(L"Captured VolumeFader\n");
			SetCapture(Child);
		}
	}
	return 0;

	case WM_COMMAND:
	{
		switch (wParam)
		{
		case PauseAndPlayEvent:
		{
			PlayAndPause_OnClick(PauseAndPlayButton, hwnd);
		}
		break;
		case VoiceOneGetStateEvent:
		{
			VoiceOne_OnClick(VoiceOneGetState);
		}
		break;
		case (WM_USER + 4):
		{
			auto Path = Win32Caption(MusicFile);
			if (Path.has_value())
			{
				std::vector<std::wstring> Files = ReadFilesIntoList(Path.value());
				ListBox_ResetContent(MusicList);
				HDC hdc = GetDC(MusicList);
				int LongestWidth = 0;
				int LongestHeight = 0;
				for (auto const& Filename : Files)
				{
					SIZE TextSize{};

					ListBox_AddString(MusicList, Filename.c_str());
					GetTextExtentPoint32W(hdc, Filename.c_str(), Filename.size(), &TextSize);
					if (TextSize.cx > LongestWidth)
					{
						LongestWidth = TextSize.cx;
					}
					LongestHeight += TextSize.cy;
				}


				ReleaseDC(MusicList, hdc);

				RECT MusicListRect{};
				GetWindowRect(MusicList, &MusicListRect);
				MusicListRect.right = LongestWidth;
				MusicListRect.bottom = LongestHeight < 50 ? LongestHeight + 5 : LongestHeight;
				SetWindowPos(MusicList, nullptr, 0, 0, MusicListRect.right, MusicListRect.bottom, SWP_NOMOVE | SWP_NOACTIVATE);
				ShowWindow(MusicList, SW_SHOW);
			}
		}
		break;
		}
	}
	return 0;
	case WM_DESTROY:
	{
		if (ButtonFont)
			DeleteObject(ButtonFont);
		if (voice1)
			voice1->FlushSourceBuffers();
		if (Data.has_value() && Data->Location)
		{
			VirtualFree(Data->Location, 0, MEM_FREE);
			Data->Location = nullptr;
		}
		if (master)
			master->DestroyVoice();

		OutputDebugString(L"Should have emptied all now\n");
		PostQuitMessage(0);
	}
	return 0;
	default:
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
}

int WINAPI WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrev, _In_ LPSTR lpszCmdLine, _In_ int nCmdShow)
{
	HRESULT hr = S_OK;
	auto ComInit = Defer<HRESULT, void()>(CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE), []() -> void { CoUninitialize(); });
	INITCOMMONCONTROLSEX IccEx{};
	IccEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	IccEx.dwICC = ICC_STANDARD_CLASSES;

	InitCommonControlsEx(&IccEx);
	WorkItem = CreateThreadpoolWork(TaskHandler, nullptr, nullptr);
	if (WorkItem == nullptr)
	{
		OutputDebugStringW(L"Failed to create work item\n");
		return -2;
	}

	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to do CoInitialize\n");
		return 1;
	}
	HBRUSH hbrBackground = CreateSolidBrush(to_rgb(0x003600ff));
	if (!Win32RegisterClass(hInst, hbrBackground))
	{
		return 1;
	}
	if (!SliderRegisterClass(hInst))
	{
		return 2;
	}
	HWND hwnd = Win32CreateWindow(CTITLENAME, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hInst);
	if (hwnd == nullptr)
	{
		OutputDebugString(L"CreateWindowEx failed\n");
		return 1;
	}
	UpdateWindow(hwnd);
	ShowWindow(hwnd, nCmdShow);

	MSG msg{};
	while (GetMessageW(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	DestroyWindow(hwnd);
	CloseThreadpoolWork(WorkItem);
	return 0;
}