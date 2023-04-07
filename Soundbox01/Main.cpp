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


constexpr wchar_t BACKGROUND_WAV[] = L"c:\\code\\10562542_Liquid_Times_Original_Mix.wav";
constexpr wchar_t CAMERASHUTTER[] = L"c:\\code\\camerashutter.wav";

LPVOID SoundBuffer = nullptr;

IXAudio2* audio = nullptr;

IXAudio2MasteringVoice* master = nullptr;
IXAudio2SourceVoice* voice1 = nullptr;

WAVEFORMATEX WaveFormatEx{};
UINT32 DataSize{};


static UINT32 Wave = 0x45564157; // <evaw>
static UINT32 Fmt = 0x20746d66; // < tmf> 
static UINT32 Data = 0x61746164; // <atad> 
void PrintWaveFormat(WAVEFORMATEX wf)
{
	printf("Waveformat\n");
	printf("Tag %d\n", wf.wFormatTag);
	printf("Channels %d\n", wf.nChannels);
	printf("Samples per sec %d\n", wf.nSamplesPerSec);
	printf("Bits %d\n", wf.wBitsPerSample);

}
size_t FindChunk(UINT32 Chunk, PUINT8 Data, size_t Offset, size_t FileSize)
{
	UINT32 Search{};
	bool Found{};
	do {
		memcpy_s(&Search, sizeof(UINT32), (PUINT8)Data + Offset, sizeof(UINT32));
		if (Search == Chunk)
		{
			Found = true;
			return Offset;
		}
		Offset += sizeof(UINT32);
		Search = 0;
	} while (!Found || Offset > FileSize);
	return Offset;
}

template<typename T>
decltype(auto) ReadChunkAt(PUINT8 Data, size_t Offset, size_t FileSize)
{
	T Result{};
	size_t SizeOfResult = sizeof(T);
	memcpy_s(&Result, SizeOfResult, Data + Offset, SizeOfResult);
	return Result;
}

void LoadWaveMMap(const std::wstring& Filename = L"C:\\Code\\10562542_Liquid_Times_Original_Mix.wav")
{
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
				size_t WaveChunkOffset = FindChunk(Wave, (PUINT8)View, 0, SoundFileSize.QuadPart);

				size_t FmtChunkOffset = FindChunk(Fmt, (PUINT8)View, WaveChunkOffset, SoundFileSize.QuadPart);
				
				UINT32 FmtChunkSize = ReadChunkAt<UINT32>((PUINT8)View, FmtChunkOffset + sizeof(UINT32), SoundFileSize.QuadPart);
				
				size_t DataChunkOffset = FindChunk(Data, (PUINT8)View, FmtChunkOffset, SoundFileSize.QuadPart);
				auto DataChunkSize = ReadChunkAt<UINT32>((PUINT8)View, DataChunkOffset+sizeof(UINT32), SoundFileSize.QuadPart);
				DataSize = DataChunkSize; // GLOBAL
				WaveFormatEx.cbSize = FmtChunkSize;
				memcpy_s(&WaveFormatEx, sizeof(WAVEFORMATEX), ((PUINT8)View + FmtChunkOffset + 4 + 4), FmtChunkSize); // size of the size of the formatlength (UINT32) and skipping it +4

				printf("Fmt offset %zd\n", FmtChunkOffset);
				printf("Fmt size %u\n", FmtChunkSize);
				printf("Data offset %zd\n", DataChunkOffset);
				printf("Data size %u\n", DataChunkSize);
				PrintWaveFormat(WaveFormatEx);

				SoundBuffer = VirtualAlloc(nullptr, DataChunkSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
				if (SoundBuffer)
				{
					OutputDebugString(L"Allocate music buffer\n");
					printf("Allocated soundbuffer %zd\n", DataChunkOffset + sizeof(UINT32)*2);
					memcpy_s(SoundBuffer, DataChunkSize, (PUINT8)View + DataChunkOffset + sizeof(UINT32) * 2, DataChunkSize);
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

}

void GenerateSineWave(PINT16 Data, int SampleRate, int Frequency)
{
	const double TS = 1.0 / double(SampleRate);
	const double Freq = double(Frequency);

	PINT16 Ptr = Data;
	double Time = 0.0;
	for (int i = 0; i < SampleRate; ++i, ++Ptr)
	{
		double Angle = (2.0 * 3.14598 * Freq) * Time;
		double Factor = 0.5 * (sin(Angle) + 1.0);
		*Ptr = INT16(32768 * Factor);
		Time += TS;
	}
}

int wmain(int argc, wchar_t** argv)
{
	HRESULT hr = S_OK;
	if (SUCCEEDED(hr))
	{
		hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
	}

	LoadWaveMMap(CAMERASHUTTER);

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
		if (SoundBuffer)
		{
			puts("Try and play the note");

			XAUDIO2_BUFFER XBuffer{};
			XBuffer.AudioBytes = DataSize;
			XBuffer.Flags = XAUDIO2_END_OF_STREAM;
			XBuffer.pAudioData = (LPBYTE)SoundBuffer;

			voice1->SetVolume(1.0f, XAUDIO2_COMMIT_NOW);

			//voice1->SetFrequencyRatio(XAudio2SemitonesToFrequencyRatio(440.0f), XAUDIO2_COMMIT_NOW); // Cool effects...
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


	if (SoundBuffer)
		VirtualFree(SoundBuffer, 0, MEM_FREE);
	if (master)
		master->DestroyVoice();
	if (audio)
		audio->Release();

	CoUninitialize();
	return 0;
}