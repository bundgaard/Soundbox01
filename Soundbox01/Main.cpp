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

std::unique_ptr<Slider> VolumeFader02;

HFONT ButtonFont;
//std::shared_ptr<Voice> voice1;
Global WAVEFORMATEX WaveFormatEx;

std::optional<WAVEDATA> g_Data;

Global HANDLE hEvent = nullptr;
Global PTP_WORK WorkItem = nullptr;
Global bool MusicLoaded;
Global bool MouseHeld;
Global int nPos;
Global float g_Volume = 1.0f; // TODO: fix so it matches slider.


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
Local void DummyFunctionForLoadingMusic(HWND self)
{
	// Separate out the load of the music and the change of text into something better.
	auto Text = Win32Caption(self);
	auto Index = ListBox_GetCurSel(MusicList);
	auto Selection = ListBox_GetCurSel(MusicList);
	Local int PreviousIndex;
	// TODO: figure out how to release the buffer and reload...
	wchar_t IndexOut[64] = { 0 };
	swprintf(IndexOut, 64, L"Index %d\n", Index);
	OutputDebugString(IndexOut);
	if (Index != nPos && voice1 != nullptr)
	{
		OutputDebugString(L"We have to reload the music...\n");
		HRESULT hr = voice1->FlushSourceBuffers();

		if (SUCCEEDED(hr))
		{
			// ReloadMusic();

		}

	}

	if (Index != -1 && wcscmp(Text->c_str(), L"Play\0") == 0)
	{

		auto TextLen = ListBox_GetTextLen(MusicList, Index);
		std::wstring Text;
		Text.resize(TextLen);
		ListBox_GetText(MusicList, Index, Text.data());

		SetWindowTextW(self, L"Pause");
		HRESULT hr = S_OK;
		// Ask for current selection of MusicList

		OutputDebugStringW(L"");

		if (voice1 == nullptr)
		{
			auto Data = LoadWaveMMap(&WaveFormatEx, Text.data());

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
					voice1->SetVolume(g_Volume, XAUDIO2_COMMIT_NOW);
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

Local BOOL
OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	int posX = 10;
	int posY = 10;
	int Width = 100;
	int Height = 25;
	int Offset = 30;


	HRESULT hr = S_OK;
	
	hEvent = CreateEvent(nullptr, true, false, nullptr);

	ButtonFont = Win32CreateFont(L"Tahoma", 14);

	PauseAndPlayButton = Win32CreateButton(hwnd, L"Play", PauseAndPlayEvent, posX, posY, Width, Height);
	posY += Offset;
	VoiceOneGetState = Win32CreateButton(hwnd, L"Voice1 State", VoiceOneGetStateEvent, posX, posY, Width, Height);
	posY += Offset;
	auto LoadFilesToList = Win32CreateButton(hwnd, L"Load from path", (WM_USER + 4), posX, posY, Width, Height);
	posY += Offset;
	MusicFile = CreateWindow(L"EDIT",
		L"C:\\Code",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
		posX, posY,
		Width, Height,
		hwnd,
		(HMENU)WM_USER + 3,
		GetModuleHandleW(nullptr),
		nullptr);
	posY += Offset;

	auto MusicLocationCaption = Win32Caption(MusicFile);

	HDC hdc = GetDC(MusicFile);
	TEXTMETRICW Metrics{};
	GetTextMetricsW(hdc, &Metrics);
	SelectObject(hdc, ButtonFont);
	
	SIZE S{};
	if (MusicLocationCaption->size() <= INT_MAX)
		GetTextExtentPoint32W(hdc, MusicLocationCaption->c_str(), static_cast<int>(MusicLocationCaption->size()), &S);
	SetWindowPos(MusicFile, nullptr, 0, 0, S.cx, S.cy + 5, SWP_NOMOVE | SWP_NOACTIVATE);
	ReleaseDC(MusicFile, hdc);

	MusicList = Win32CreateListbox(hwnd, posX, posY, Width, 150);
	
	Win32SetFont(PauseAndPlayButton, ButtonFont);
	Win32SetFont(VoiceOneGetState, ButtonFont);
	Win32SetFont(MusicFile, ButtonFont);
	Win32SetFont(LoadFilesToList, ButtonFont);
	Win32SetFont(MusicList, ButtonFont);

	VolumeFader02 = std::make_unique<Slider>(hwnd, 500, 10, 64, 128); // TODO: reimplement min and max
	
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
	if (ButtonFont)
		DeleteObject(ButtonFont);
	if (voice1)
		voice1->FlushSourceBuffers();
	if (g_Data.has_value() && g_Data->Location)
	{
		VirtualFree(g_Data->Location, 0, MEM_FREE);
		g_Data->Location = nullptr;
	}
	if (master)
		master->DestroyVoice();

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
	case PauseAndPlayEvent:
	{
		PlayAndPause_OnClick(PauseAndPlayButton, Parent);
	}
	break;
	case VoiceOneGetStateEvent:
	{
		VoiceOne_OnClick(VoiceOneGetState);
	}
	break;
	case WM_CM_LOADFILES:
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
				ListBox_AddString(MusicList, Filename.c_str());
				if (Filename.size() < INT_MAX)
				{
					SIZE TextSize{};
					GetTextExtentPoint32W(hdc, Filename.c_str(), static_cast<int>(Filename.size()), &TextSize);
					if (TextSize.cx > LongestWidth)
					{
						LongestWidth = TextSize.cx;
					}
					LongestHeight += TextSize.cy;
				}
				else
				{
					OutputDebugString(L"Could not redeclare size_t as int, it was too large\n");
				}
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

Local std::wstring MusicList_GetSelectedItem()
{
	auto SelectedIndex = ListBox_GetCurSel(MusicList);
	auto SelectedTextLength = ListBox_GetTextLen(MusicList, SelectedIndex);
	std::wstring SelectedText(SelectedTextLength + 1, L'\0');
	ListBox_GetText(MusicList, SelectedIndex, SelectedText.data());
	SelectedText.resize(SelectedTextLength);
	return SelectedText;
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

	case WM_VOLUME_CHANGED:
	{
		int Volume = (int)wParam;
		wchar_t Buf[64] = { 0 };
		float ToVolume = (100 - Volume) / 100.0f;
		swprintf(Buf, 64, L"Volume %d -> %.2f\n", Volume, ToVolume);
		OutputDebugStringW(Buf);
		g_Volume = ToVolume;
		if (voice1)
		{
			voice1->SetVolume(g_Volume, XAUDIO2_COMMIT_NOW);
		}
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
			// create voice
		}
		else
		{

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
		}
	}
	return 0;

	HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
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
	if (!Win32RegisterClass(hInst, SoundboxProc, hbrBackground))
	{
		return 1;
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