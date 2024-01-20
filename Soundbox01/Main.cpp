
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
#include <cmath>

#include "Styleguide.h"

#include "Defer.h"
#include "Win32Layer.h"
#include "Files.h"
#include "Slider.h"
#include "Voice.h"
#include "PlayAndPauseButton.h"
#include "WindowMessages.h"
#include "VoiceCallback.h"
#include <wil/resource.h>
#include <wil/com.h>


using namespace tretton63;
using CallbackData = VoiceCallback;
/*
Chart
High Value rounded up
Min value rounded down
For .wav I can go with unsigned so from 0 to UINT?_MAX + nearest 0 uint16_t::max = 65 536 -> nearest thousand would be 70 000.

Then we need a timeline, conversely that could be each data point or we can take the average bytes per second and read that and plot. This will give us each second as a natual time line.
Simplicity is the key so now we could plot that data line white background and black line.

Another thing to think about is how to limit the time line to our width, Excel plots large amount of numbers in intervals and smaller amounts directly.

*/

Global wil::com_ptr<IXAudio2> audio;
Global IXAudio2MasteringVoice* master;
Global IXAudio2SourceVoice* voice1;

Global wil::unique_hwnd PauseAndPlayButton;

Global wil::unique_hwnd VoiceOneGetState;
Global wil::unique_hwnd MusicFile;
Global wil::unique_hwnd SoundProgress;
Global wil::unique_hwnd LoadFilesToList;

Global wil::unique_hwnd MusicList;
Global wil::unique_hfont ButtonFont;

std::unique_ptr<Slider> VolumeFader02;

Global wil::unique_event g_MusicEvent;

Global bool MouseHeld;
Global int nPos;
Global float g_Volume = 1.0f; // TODO: fix so it matches slider.
Global std::unique_ptr<WAVEDATA> g_Data;
Global XAUDIO2_BUFFER* Buffer;
Global std::wstring PreviousSelectedText;
std::unique_ptr<CallbackData> callback; // TODO fix the naming



Local XAUDIO2_BUFFER*
LoadBuffer(std::unique_ptr<WAVEDATA> const& Data)
{
	XAUDIO2_BUFFER* XBuffer = new XAUDIO2_BUFFER{};

	XBuffer->AudioBytes = narrow_cast<uint32_t>(Data->WaveSize);
	XBuffer->Flags = XAUDIO2_END_OF_STREAM;
	XBuffer->pAudioData = (LPBYTE)Data->Location;
	return XBuffer;
}

inline void
CheckBool(bool Value)
{
	if (!Value)
	{
		DWORD dwError = GetLastError();
		wchar_t Buf[64] = { 0 };
		swprintf(Buf, 64, L"Failed to clear memory\nError %d\n", dwError);
		OutputDebugStringW(Buf);
	}
}

inline HBRUSH
Win32CreateSolidBrush(UINT32 r, UINT32 g, UINT32 b)
{
	return CreateSolidBrush(RGB(r, g, b));
}


inline RECT
Win32GetWindowRect(HWND hwnd)
{
	RECT Rect{};
	GetWindowRect(hwnd, &Rect);
	return Rect;
}

inline RECT
Win32GetClientRect(HWND hwnd)
{
	RECT Rect{};
	GetClientRect(hwnd, &Rect);
	return Rect;
}

inline void
CheckHR(HRESULT hr)
{
	if (FAILED(hr))
	{
		DWORD dwError = GetLastError();
		if (dwError != 0x00)
		{
			OutputDebugStringW(L"Failed to create and configure voice\n");
			wchar_t Buf[64] = { 0 };
			swprintf(Buf, 64, L"dwError %x\n", dwError);
			OutputDebugStringW(Buf);
		}
	}
}


inline void
ClearMusic()
{
	g_MusicEvent.ResetEvent();
	if (voice1 != nullptr)
	{
		voice1->Stop();
		voice1->FlushSourceBuffers();
	}

	if (g_Data != nullptr)
	{
		CheckBool(VirtualFree(g_Data->Location, 0, MEM_RELEASE));
		g_Data.reset();
		PlayAndPause_Reset(PauseAndPlayButton.get());
	}
}

Local void
VoiceOne_OnClick(HWND self)
{
	ClearMusic();
}

inline SIZE
Win32GetFontMetrics(HWND hwnd)
{
	HDC hdc = GetDC(hwnd);
	SIZE S{};
	auto Caption = Win32Caption(hwnd);
	std::wstring ClassName{};
	ClassName.resize(255);
	int ClassNameSize = GetClassNameW(hwnd, ClassName.data(), 255);
	ClassName.resize(ClassNameSize + 1);
	OutputDebugStringW(ClassName.c_str());
	ClassName.resize(ClassNameSize);
	if (ClassName == L"ListBox")
	{
		auto ListBoxTextLen = ListBox_GetTextLen(hwnd, 0);
		std::wstring ListBoxText{};
		ListBoxText.resize(ListBoxTextLen);
		ListBox_GetText(hwnd, 0, ListBoxText.data());
		CheckBool(GetTextExtentPoint32W(hdc, ListBoxText.c_str(), ListBoxText.size(), &S));
	}
	if (Caption.has_value())
	{
		CheckBool(GetTextExtentPoint32W(hdc, Caption->c_str(), Caption->size(), &S));
	}
	ReleaseDC(hwnd, hdc);
	return S;
}


Local BOOL
OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	auto dpi = GetDpiForWindow(hwnd);
	double scaling = dpi / 96.0;

	int posX = 10;
	int posY = 10;
	int Width = 100 * scaling;
	int Height = 25 * scaling;
	int Offset = 30 * scaling;
	int fontSize = 14 * scaling;

	ButtonFont.reset(Win32CreateFont(L"Comic Sans MS", fontSize, FW_BOLD));
	PauseAndPlayButton.reset(Win32CreateButton(hwnd, L"Play", WM_CM_PLAY_AND_PAUSE_BUTTON, posX, posY, Width, Height));
	EnableWindow(PauseAndPlayButton.get(), false);
	posY += Offset;

	VoiceOneGetState.reset(Win32CreateButton(hwnd, L"Stop", WM_CM_STOP_BUTTON, posX, posY, Width, Height));
	posY += Offset;

	LoadFilesToList.reset(Win32CreateButton(hwnd, L"Load from path", WM_CM_LOADFILES, posX, posY, Width, Height));
	posY += Offset;

	MusicFile.reset(CreateWindow(L"EDIT",
		L"C:\\Code",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
		posX, posY,
		Width, Height+7,
		hwnd,
		nullptr,
		GetModuleHandleW(nullptr),
		nullptr));
	posY += Offset;

	SIZE S = Win32GetFontMetrics(MusicFile.get());
	SetWindowPos(MusicFile.get(), nullptr, 0, 0, std::max(S.cx, static_cast<LONG>(Width)), std::min(S.cy + 5, static_cast<LONG>(Height)), SWP_NOMOVE | SWP_NOACTIVATE);

	MusicList.reset(Win32CreateListbox(hwnd, posX, posY, Width, 150));

	Win32SetFont(PauseAndPlayButton.get(), ButtonFont.get());
	Win32SetFont(VoiceOneGetState.get(), ButtonFont.get());
	Win32SetFont(MusicFile.get(), ButtonFont.get());
	Win32SetFont(LoadFilesToList.get(), ButtonFont.get());
	Win32SetFont(MusicList.get(), ButtonFont.get());

	VolumeFader02 = std::make_unique<Slider>(hwnd, 10 + posX + Width, 10, 64, Height * 4 + (3 * 4)); // TODO: reimplement min and max
	Win32SetFont(VolumeFader02->Handle(), ButtonFont.get());

	HRESULT hr = S_OK;

	if (SUCCEEDED(hr))
	{
		hr = XAudio2Create(&audio);
	}

	if (SUCCEEDED(hr))
	{
		OutputDebugString(L"Create Master voice\n");
		hr = audio->CreateMasteringVoice(&master, 2, 44100);
	}

	if (FAILED(hr))
	{
		return FALSE;
	}
	return TRUE;
}

Local void
OnDestroy(HWND hwnd)
{

	if (voice1 != nullptr && g_Data != nullptr)
	{
		ClearMusic();
	}

	if (master)
	{
		master->DestroyVoice();
	}

	OutputDebugString(L"Should have emptied all now\n");
	PostQuitMessage(0);
}

Local void
OnDrawItem(HWND hwnd, DRAWITEMSTRUCT const* pDis)
{
	SetBkMode(pDis->hDC, TRANSPARENT);

	if (pDis->CtlType == ODT_BUTTON)
	{
		Win32CustomButton(pDis);
	}
	else if (pDis->CtlType == ODT_LISTBOX)
	{
		Win32CustomListBox(pDis);
	}
}


Local HBRUSH
OnColorButton(HWND Parent, HDC hdc, HWND Child, int Type)
{
	SetBkMode(hdc, OPAQUE);
	SetTextColor(hdc, RGB(0, 0, 255));
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}


Local void
OnCommand(HWND Parent, int ID, HWND Child, UINT CodeNotify)
{
	switch (ID)
	{
	case WM_CM_PLAY_AND_PAUSE_BUTTON:
	{
		PlayAndPause_OnClick(PauseAndPlayButton.get(), Parent);
	}
	break;
	case WM_CM_STOP_BUTTON:
	{
		VoiceOne_OnClick(VoiceOneGetState.get());
	}
	break;
	case WM_CM_LOADFILES:
	{
		auto Path = Win32Caption(MusicFile.get());
		if (Path.has_value())
		{
			std::vector<std::wstring> Files = ReadFilesIntoList(Path.value());
			ListBox_ResetContent(MusicList.get());
			for (auto const& Filename : Files)
			{
				ListBox_AddString(MusicList.get(), Filename.c_str());
			}

			SIZE S = Win32GetFontMetrics(MusicList.get());
			RECT MusicListRect = Win32GetWindowRect(MusicList.get());
			MusicListRect.right = S.cx;
			MusicListRect.bottom = S.cy * (Files.size() + 1);

			SetWindowPos(MusicList.get(), nullptr, 0, 0, MusicListRect.right, MusicListRect.bottom, SWP_NOMOVE | SWP_NOACTIVATE);
			ShowWindow(MusicList.get(), SW_SHOW);
			EnableWindow(PauseAndPlayButton.get(), true);
		}
	}
	break;
	}

}

Local std::wstring
MusicList_GetSelectedItem()
{
	auto SelectedIndex = ListBox_GetCurSel(MusicList.get());
	auto SelectedTextLength = ListBox_GetTextLen(MusicList.get(), SelectedIndex);
	std::wstring SelectedText(SelectedTextLength + 1, L'\0');
	ListBox_GetText(MusicList.get(), SelectedIndex, SelectedText.data());
	SelectedText.resize(SelectedTextLength);
	return SelectedText;
}

Local void
CreateVoiceWithFile(std::wstring const& SelectedText)
{

	// create voice
		// Load File from LoadWave
	WAVEFORMATEX FormatEx{};
	g_Data = LoadWaveMMap(&FormatEx, SelectedText);
	HRESULT hr = S_OK;
	if (SUCCEEDED(hr))
	{
		hr = audio->CreateSourceVoice(
			&voice1,
			&FormatEx,
			0,
			XAUDIO2_DEFAULT_FREQ_RATIO, callback.get());
	}
	if (SUCCEEDED(hr))
	{
		hr = voice1->SetVolume(g_Volume, XAUDIO2_COMMIT_NOW);
	}
	if (SUCCEEDED(hr))
	{
		Buffer = LoadBuffer(g_Data);
		hr = voice1->SubmitSourceBuffer(Buffer);

		if (SUCCEEDED(hr))
		{
			hr = voice1->Start(XAUDIO2_COMMIT_NOW); // TODO: need to refactor this into one entry to play and pause as we now have double entries, which is confusing.
			g_MusicEvent.SetEvent();
			PreviousSelectedText = SelectedText;
		}
		if (FAILED(hr))
		{
			OutputDebugStringW(L"Failed to play\n");
		}
	}
	CheckHR(hr);
}

LRESULT CALLBACK
SoundboxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_DRAWITEM, OnDrawItem);
		HANDLE_MSG(hwnd, WM_CTLCOLORBTN, OnColorButton);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);

	case WM_VOLUME_CHANGED:
	{
		int Volume = (int)wParam;
		float ToVolume = (100 - Volume) / 100.0f;
		g_Volume = ToVolume;

		if (voice1)
		{
			voice1->SetVolume(g_Volume, XAUDIO2_COMMIT_NOW);
		}
	}
	return 0;
	case WM_CM_STREAM_ENDED:
	{
		ClearMusic();
	}
	return 0;
	case WM_CM_PLAYMUSIC:
	{
		OutputDebugStringW(L"Start playing\n");
		// Extract selected file from listbox
		// Create voice and fill the voice with the music

		auto SelectedText = MusicList_GetSelectedItem();
		OutputDebugStringW(SelectedText.c_str());
		OutputDebugStringW(L"\n");


		if (voice1 != nullptr)
		{
			if (SelectedText != PreviousSelectedText)
			{
				OutputDebugStringW(L"Reload music\n");
				HRESULT hr = S_OK;

				if (SUCCEEDED(hr))
				{
					hr = voice1->FlushSourceBuffers();

					OutputDebugStringW(L"Wait for flushing\n");
				}

				if (SUCCEEDED(hr))
				{
					voice1->DestroyVoice();
					OutputDebugStringW(L"Flush buffers OK\n");
					if (g_Data && g_Data->Location)
					{
						OutputDebugStringW(L"Clear memory\n");
						CheckBool(VirtualFree(g_Data->Location, 0, MEM_RELEASE));
						g_Data.release();

					}
				}
				CreateVoiceWithFile(SelectedText);
			}
			else
			{
				voice1->Start();
				g_MusicEvent.SetEvent();

			}
		}
		else
		{
			CreateVoiceWithFile(SelectedText);
		}
	}
	return 0;

	case WM_CM_PAUSEMUSIC:
	{
		OutputDebugStringW(L"Pause playing\n");
		// Maybe just "stop"/pause the music and on play we decide if we have changed the song or should "resume" / start it again.
		if (voice1 != nullptr)
		{
			voice1->Stop(XAUDIO2_COMMIT_NOW);
			g_MusicEvent.ResetEvent();
		}
	}
	return 0;


	default:
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
}

DWORD WINAPI ThreadTracker(PVOID pArguments)
{
	UINT64 SamplePlayed{};

	while (true)
	{
		WaitForSingleObject(g_MusicEvent.get(), INFINITE);

		if (voice1 != nullptr)
		{
			XAUDIO2_VOICE_STATE voiceState{};
			voice1->GetState(&voiceState);

			if (voiceState.SamplesPlayed != SamplePlayed)
			{
				std::wstringstream Buf{};
				Buf << L"State "
					<< std::to_wstring(voiceState.SamplesPlayed)
					<< L" of "
					<< std::to_wstring(voiceState.BuffersQueued)
					<< L"\n";
				OutputDebugStringW(Buf.str().c_str());
				SamplePlayed = voiceState.SamplesPlayed;
			}


		}

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

	wil::unique_hbrush hbrBackground(CreateSolidBrush(to_rgb(0x003600ff)));

	if (!Win32RegisterClass(hInst, SoundboxProc, hbrBackground.get()))
	{
		OutputDebugStringW(L"Cannot register class name\n");
		return 1;
	}


	wil::unique_hwnd hwnd(Win32CreateWindow(CTITLENAME, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hInst));
	auto dpi = GetDpiForWindow(hwnd.get());
	RECT rct{};
	GetClientRect(hwnd.get(), &rct);
	AdjustWindowRectExForDpi(&rct, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW, dpi);
	SetWindowPos(hwnd.get(), nullptr, rct.left, rct.top, (rct.right - rct.left), rct.bottom - rct.top, SWP_NOMOVE|SWP_NOACTIVATE);
	UpdateWindow(hwnd.get());
	ShowWindow(hwnd.get(), nCmdShow);

	g_MusicEvent.create(wil::EventOptions::ManualReset);
	DWORD ThreadID{};
	HANDLE hTread = CreateThread(nullptr, 0, &ThreadTracker, 0, 0, &ThreadID);
	callback = std::make_unique<CallbackData>(hwnd.get());

	MSG msg{};
	while (GetMessageW(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return 0;
}