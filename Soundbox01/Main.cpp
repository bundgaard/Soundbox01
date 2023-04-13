#include "Application.h"
#include "WaveReader.h"

#include <vector>
#include <sstream>
#include <memory>
#include <optional>

#include <cstdio>
#include <cstdlib>
#include <cstdint>

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


using namespace tretton63;

Global IXAudio2* audio = nullptr;
Global IXAudio2MasteringVoice* master = nullptr;
Global IXAudio2SourceVoice* voice1 = nullptr;

Global HWND PauseAndPlayButton;
Global HWND VoiceOneGetState;
Global HWND MusicFile;
Global HWND SoundProgress;

Global HFONT ButtonFont;
Global WAVEFORMATEX WaveFormatEx;
std::optional<WAVEDATA> Data;
struct EventParams {
	HANDLE Event;
	bool shouldContinue;
	size_t Size;
	void* Location;
};

Global HANDLE hEvent = nullptr;
Global PTP_WORK WorkItem = nullptr;

void NTAPI TaskHandler(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
	wchar_t Buf[64] = { 0 };
	wsprintf(Buf, L"Starting new work\n");
	OutputDebugStringW(Buf);

	Sleep(rand() % 255);
}

Local void
PlayAndPause_OnClick(HWND self, HWND parent)
{
	{
		
		HRESULT hr = S_OK;
		auto Data = LoadWaveMMap(&WaveFormatEx, L"C:\\Code\\10562542_Liquid_Times_Original_Mix.wav");
		if (Data.has_value())
		{
			if (Data->Location)
			{

				SubmitThreadpoolWork(WorkItem);
				if (SUCCEEDED(hr))
				{
					OutputDebugString(L"CreateSourceVoice\n");
					hr = audio->CreateSourceVoice(&voice1, &WaveFormatEx, 0, XAUDIO2_DEFAULT_FREQ_RATIO);
				}
				puts("Try and play the note");

				XAUDIO2_BUFFER XBuffer{};
				if (Data->WaveSize <= UINT32_MAX)
					XBuffer.AudioBytes = static_cast<uint32_t>(Data->WaveSize);
				else
				{
					OutputDebugString(L"Failed to resize the WaveSize\n");
				}
				XBuffer.Flags = XAUDIO2_END_OF_STREAM;
				XBuffer.pAudioData = (LPBYTE)Data->Location;

				voice1->SetVolume(1.0f, XAUDIO2_COMMIT_NOW);

				hr = voice1->SubmitSourceBuffer(&XBuffer);
			}
			Data = std::move(Data);
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

	std::wstring Text{};
	auto TextLength = GetWindowTextLengthW(self);
	Text.resize((size_t)TextLength + 1, 0);
	if (TextLength <= INT_MAX)
		GetWindowTextW(self, Text.data(), (int)Text.size());

	if (wcscmp(Text.c_str(), L"Play\0") == 0)
	{
		SetWindowTextW(self, L"Pause");

		voice1->Start();
	}
	else
	{
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

		PauseAndPlayButton = CreateWindow(L"BUTTON", L"Play", WS_VISIBLE | WS_CHILD, 10, 10, 100, 25, hwnd, (HMENU)PauseAndPlayEvent, GetModuleHandle(nullptr), nullptr);
		VoiceOneGetState = CreateWindow(L"BUTTON", L"Voice1 State", WS_VISIBLE | WS_CHILD, 10, 35, 100, 25, hwnd, (HMENU)VoiceOneGetStateEvent, GetModuleHandleW(nullptr), nullptr);
		MusicFile = CreateWindow(L"EDIT", L"C:\\Code\\10847939_Ad_Finem_Original_Mix.wav", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 10, 65, 100, 25, hwnd, (HMENU)WM_USER + 3, GetModuleHandleW(nullptr), nullptr);

		SendMessage(PauseAndPlayButton, WM_SETFONT, (WPARAM)(HFONT)ButtonFont, 0);
		SendMessage(VoiceOneGetState, WM_SETFONT, (WPARAM)(HFONT)ButtonFont, 0);
		SendMessage(MusicFile, WM_SETFONT, (WPARAM)(HFONT)ButtonFont, 0);

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
	WorkItem = CreateThreadpoolWork(TaskHandler, nullptr, nullptr);

	if (FAILED(hr))
	{
		OutputDebugString(L"Failed to do CoInitialize\n");
		return 1;
	}

	if (!Win32RegisterClass(hInst))
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