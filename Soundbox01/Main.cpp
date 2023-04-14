#include "Application.h"
#include "WaveReader.h"
#include <windowsx.h>

#include <vector>
#include <sstream>
#include <memory>
#include <optional>

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <CommCtrl.h>
#pragma comment(lib, "comctl32")

#include "Defer.h"
#include "Win32Layer.h"

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

Global IXAudio2* audio = nullptr;
Global IXAudio2MasteringVoice* master = nullptr;
Global IXAudio2SourceVoice* voice1 = nullptr;

Global HWND PauseAndPlayButton;
Global HWND VoiceOneGetState;
Global HWND MusicFile;
Global HWND SoundProgress;
Global HWND LoadFilesToList;
Global HWND MusicList;

Global HFONT ButtonFont;

Global WAVEFORMATEX WaveFormatEx;

std::optional<WAVEDATA> g_Data;

Global HANDLE hEvent = nullptr;
Global PTP_WORK WorkItem = nullptr;
Global bool MusicLoaded;
std::shared_ptr<Foo> g_Foo;

void NTAPI TaskHandler(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
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

	if (Text.has_value() && wcscmp(Text->c_str(), L"Play\0") == 0)
	{
		SetWindowTextW(self, L"Pause");
		{

			HRESULT hr = S_OK;
			auto Text = Win32Caption(MusicFile);
			if (voice1 == nullptr && Text.has_value())
			{
				auto Data = LoadWaveMMap(&WaveFormatEx, Text->c_str());
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
	XAUDIO2_VOICE_STATE state{};
	voice1->GetState(&state, 0);
	std::wstringstream Out{};
	Out << L"Samples played: " << state.SamplesPlayed << "\n" << L"Buffers queued: " << state.BuffersQueued << "\n";
	OutputDebugString(Out.str().c_str());
}
struct WAVFILE {
	std::wstring Path;
};

std::vector<WAVFILE> VectorOfFiles;

Local void
ReadFilesIntoList(HWND EditControl /*MusicFile*/, HWND List/* MusicList*/)
{
	auto Path = Win32Caption(EditControl);
	WIN32_FIND_DATAW FindBlock{};
	if (Path.has_value())
	{
		std::wstring Foo{};
		Foo += L"\\\\?\\";
		Foo += Path->c_str();		
		Foo += L"\\*";
	
		HANDLE MyFile = FindFirstFileW(Foo.c_str(), &FindBlock);
		if (MyFile == nullptr)
		{
			DWORD dwError = GetLastError();
			wchar_t Buf[64] = { 0 };
			wsprintf(Buf, L"Error %x\n", dwError);
			OutputDebugString(Buf);
		}
		else
		{
			do {
				std::wstring Filename{ FindBlock.cFileName };
				if (Filename.ends_with(L".wav\0"))
				{
					ListBox_ResetContent(List);
					OutputDebugStringW(Filename.c_str());
					VectorOfFiles.push_back(WAVFILE{ .Path = (Foo + Filename) });
					// TODO(fix resize);
					SendMessage(List, LB_ADDSTRING, (WPARAM) 0, (LPARAM)Filename.c_str());
				}


			} while (FindNextFileW(MyFile, &FindBlock));

		}

	}
	

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
		SelectObject(hdc, ButtonFont);

		SIZE S{};
		GetTextExtentPoint32W(hdc, MusicLocationCaption->c_str(), MusicLocationCaption->size(), &S);
		SetWindowPos(MusicFile, nullptr, 0, 0, S.cx, S.cy + 5, SWP_NOMOVE | SWP_NOACTIVATE);
		ReleaseDC(MusicFile, hdc);
		MusicList = CreateWindow(L"listbox", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, posX, posY, Width, 150, hwnd, 0, GetModuleHandleW(0), nullptr);
		SendMessage(MusicList, LB_ADDSTRING, (WPARAM)0,(LPARAM)L"Foobar");


		SendMessage(PauseAndPlayButton, WM_SETFONT, (WPARAM)(HFONT)ButtonFont, 0);
		SendMessage(VoiceOneGetState, WM_SETFONT, (WPARAM)(HFONT)ButtonFont, 0);
		SendMessage(MusicFile, WM_SETFONT, (WPARAM)(HFONT)ButtonFont, 0);
		SendMessage(LoadFilesToList, WM_SETFONT, (WPARAM)(HFONT)ButtonFont, 0);
		SendMessage(MusicList, WM_SETFONT, (WPARAM)(HFONT)ButtonFont, 0);

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
			ReadFilesIntoList(MusicFile, MusicList);
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
		if (audio)
			audio->Release();
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

	if (!Win32RegisterClass(hInst))
	{
		return 1;
	}
	g_Foo = std::make_shared<Foo>();
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