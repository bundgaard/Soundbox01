#include "Application.h"
#include "WaveReader.h"
#include <vector>
#include <string>

#include "Defer.h"
#include "Win32Layer.h"
using namespace tretton63;

IXAudio2* audio = nullptr;
IXAudio2MasteringVoice* master = nullptr;
IXAudio2SourceVoice* voice1 = nullptr;


//TODO(david): Create nice fonts and use them for each child window

Local void
PlayAndPause_OnClick(HWND self, HWND parent)
{
	std::wstring Text{};
	auto TextLength = GetWindowTextLengthW(self);
	Text.resize((size_t)TextLength + 1, 0);
	GetWindowTextW(self, Text.data(), Text.size());

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

LRESULT CALLBACK
SoundboxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Scoped HWND PauseAndPlayButton;

	switch (msg)
	{
	case WM_CREATE:
	{
		PauseAndPlayButton = CreateWindow(L"BUTTON", L"Play", WS_VISIBLE | WS_CHILD, 10, 10, 100, 25, hwnd, (HMENU)PauseAndPlayEvent, GetModuleHandle(nullptr), nullptr);
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
		}
	}
	return 0;
	case WM_DESTROY:
	{
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

	if (!Win32RegisterClass(hInst))
	{
		return 1;
	}

	Local WAVEFORMATEX WaveFormatEx{};

	auto Data = LoadWaveMMap(&WaveFormatEx, L"C:\\Code\\10847939_Ad_Finem_Original_Mix.wav");
	if (!Data.has_value())
	{
		return 1;
	}

	XAudio2Create(&audio);
	if (SUCCEEDED(hr))
	{
		hr = ComInit.Get();
	}

	if (SUCCEEDED(hr))
	{
		OutputDebugString(L"Create Master voice\n");
		hr = audio->CreateMasteringVoice(&master, 2, 44100);
	}

	if (SUCCEEDED(hr))
	{
		OutputDebugString(L"CreateSourceVoice\n");
		hr = audio->CreateSourceVoice(&voice1, &WaveFormatEx, 0, XAUDIO2_DEFAULT_FREQ_RATIO);
	}

	if (SUCCEEDED(hr))
	{
		if (Data->Location)
		{
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

			//voice1->SetFrequencyRatio(XAudio2SemitonesToFrequencyRatio(880.0f), XAUDIO2_COMMIT_NOW); // Cool effects...
			hr = voice1->SubmitSourceBuffer(&XBuffer);

		}
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

	HWND hwnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		CCLASSNAME,
		CTITLENAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		nullptr,
		nullptr,
		hInst,
		nullptr);
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
	return 0;
}