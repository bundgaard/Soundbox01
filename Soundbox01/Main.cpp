#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef _USE_MM_AUDIO
#include <mmsystem.h>
#pragma comment(lib, "winmm")
#else
#define XAUDIO2_HELPER_FUNCTIONS
#include <xaudio2.h>
#pragma comment(lib, "xaudio2")
#endif

#include <cstdio>
#include <cstdlib>

#include <string>
#include <cstdint>

constexpr wchar_t BACKGROUND_WAV[] = L"c:\\code\\10562542_Liquid_Times_Original_Mix.wav";
constexpr wchar_t CAMERASHUTTER[] = L"c:\\code\\camerashutter.wav";

IXAudio2* audio = nullptr;

IXAudio2MasteringVoice* master = nullptr;
IXAudio2SourceVoice* voice1 = nullptr;

WAVEFORMATEX WaveFormatEx{};

static uint32_t Wave = 0x45564157; // <evaw>
static uint32_t Fmt = 0x20746d66; // < tmf> 
static uint32_t Data = 0x61746164; // <atad> 

void PrintWaveFormat(WAVEFORMATEX wf)
{
	printf("Waveformat\n");
	printf("Tag %d\n", wf.wFormatTag);
	printf("Channels %d\n", wf.nChannels);
	printf("Samples per sec %d\n", wf.nSamplesPerSec);
	printf("Bits %d\n", wf.wBitsPerSample);

}
size_t FindChunk(uint32_t Chunk, uint8_t* Data, size_t Offset, size_t FileSize)
{
	uint32_t Search{};
	bool Found{};
	do {
		memcpy_s(&Search, sizeof(uint32_t), (uint8_t*)Data + Offset, sizeof(uint32_t));
		if (Search == Chunk)
		{
			Found = true;
			return Offset;
		}
		Offset += sizeof(uint32_t);
		Search = 0;
	} while (!Found || Offset > FileSize);
	return Offset;
}

template<typename T>
decltype(auto) ReadChunkAt(uint8_t* Data, size_t Offset, size_t SizeToRead = 0)
{
	T Result{};
	size_t SizeOfResult = sizeof(T);
	if (SizeToRead != 0)
	{
		memcpy_s(&Result, SizeToRead, Data + Offset, SizeToRead);
	}
	else
	{
		memcpy_s(&Result, SizeOfResult, Data + Offset, SizeOfResult);
	}

	return Result;
}
struct WAVEDATA{
	size_t WaveSize;
	void* Location;
};

WAVEDATA LoadWaveMMap(const std::wstring& Filename = L"C:\\Code\\10562542_Liquid_Times_Original_Mix.wav")
{
	WAVEDATA Result{};

	SYSTEM_INFO Si{};
	GetSystemInfo(&Si);

	HANDLE SoundFile = CreateFile(Filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (SoundFile)
	{
		LARGE_INTEGER SoundFileSize{};
		GetFileSizeEx(SoundFile, &SoundFileSize);

		OutputDebugStringW(L"Opened SoundFile\n");
		LPVOID View = nullptr;
		HANDLE SoundFileMapping = CreateFileMapping(SoundFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
		if (SoundFileMapping)
		{
			OutputDebugStringW(L"Opened a mapping to file\n");

			View = MapViewOfFile(SoundFileMapping, FILE_MAP_READ, 0, 0, 0);
			if (View)
			{
				OutputDebugString(L"Before\n");
				size_t WaveChunkOffset = FindChunk(Wave, (uint8_t*)View, 0, SoundFileSize.QuadPart);			
				size_t FmtChunkOffset = FindChunk(Fmt, (uint8_t*)View, WaveChunkOffset, SoundFileSize.QuadPart);
				uint32_t FmtChunkSize = ReadChunkAt<uint32_t>((uint8_t*)View, FmtChunkOffset + sizeof(uint32_t));
				size_t DataChunkOffset = FindChunk(Data, (uint8_t*)View, FmtChunkOffset, SoundFileSize.QuadPart);
				uint32_t DataChunkSize = ReadChunkAt<uint32_t>((uint8_t*)View, DataChunkOffset + sizeof(uint32_t));

				Result.WaveSize = DataChunkSize;
				WaveFormatEx.cbSize = FmtChunkSize;
				memcpy_s(&WaveFormatEx, sizeof(WAVEFORMATEX), ((uint8_t*)View + FmtChunkOffset + sizeof(uint32_t) * 2), FmtChunkSize); // size of the size of the formatlength (uint32_t) and skipping it +4

				printf("Fmt offset %zd\n", FmtChunkOffset);
				printf("Fmt size %u\n", FmtChunkSize);
				printf("Data offset %zd\n", DataChunkOffset);
				printf("Data size %u\n", DataChunkSize);
				PrintWaveFormat(WaveFormatEx);

				Result.Location = VirtualAlloc(nullptr, DataChunkSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
				if (Result.Location)
				{
					OutputDebugString(L"Allocate music buffer\n");
					printf("Allocated soundbuffer %zd\n", DataChunkOffset + sizeof(uint32_t) * 2);
					memcpy_s(Result.Location, DataChunkSize, (uint8_t*)View + DataChunkOffset + sizeof(uint32_t) * 2, DataChunkSize);
				}
			}
		}

		if (View)
			UnmapViewOfFile(View);
		if (SoundFileMapping)
			CloseHandle(SoundFileMapping);
		if (SoundFile)
			CloseHandle(SoundFile);
	}
	return Result;
}


int wmain(int argc, wchar_t** argv)
{
	HRESULT hr = S_OK;
	if (SUCCEEDED(hr))
	{
		hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
	}

	WAVEDATA Data = LoadWaveMMap();

	XAudio2Create(&audio);

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
		if (Data.Location)
		{
			puts("Try and play the note");

			XAUDIO2_BUFFER XBuffer{};
			XBuffer.AudioBytes = Data.WaveSize;
			XBuffer.Flags = XAUDIO2_END_OF_STREAM;
			XBuffer.pAudioData = (LPBYTE)Data.Location;

			voice1->SetVolume(1.0f, XAUDIO2_COMMIT_NOW);

			//voice1->SetFrequencyRatio(XAudio2SemitonesToFrequencyRatio(880.0f), XAUDIO2_COMMIT_NOW); // Cool effects...
			hr = voice1->SubmitSourceBuffer(&XBuffer);

		}
	}


	if (SUCCEEDED(hr))
	{
		OutputDebugString(L"Voice1 Start\n");
		hr = voice1->Start(0);
	}

	if (SUCCEEDED(hr))
	{
		OutputDebugString(L"Playing note\n");
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

	bool isStopped = false;

	while (true)
	{
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			break;
		}
		if (GetAsyncKeyState(VK_SPACE))
		{
			isStopped = true;
		}

		if (GetAsyncKeyState(VK_END))
		{
			isStopped = false;
		}

		if (isStopped)
		{
			voice1->Stop();
		}
		else
		{
			voice1->Start(XAUDIO2_COMMIT_NOW);
		}
		Sleep(1);
	}


	if (Data.Location)
		VirtualFree(Data.Location, 0, MEM_FREE);
	if (master)
		master->DestroyVoice();
	if (audio)
		audio->Release();

	CoUninitialize();
	return 0;
}