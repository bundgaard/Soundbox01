#include "WaveReader.h"

namespace tretton63
{



	constexpr int Convert(const char* szText)
	{
		int result{};
		char* ptr = const_cast<char*>(szText);
		do {
			result += '0' - *ptr;
		} while (*ptr != '\0');
		return result;
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

	void PrintWaveFormat(WAVEFORMATEX* const  wf)
	{
		printf("Waveformat\n");
		printf("Tag %d\n", wf->wFormatTag);
		printf("Channels %d\n", wf->nChannels);
		printf("Samples per sec %d\n", wf->nSamplesPerSec);
		printf("Bits %d\n", wf->wBitsPerSample);
	}

	std::optional<WAVEDATA> LoadWaveMMap(WAVEFORMATEX* WaveFormatEx, const std::wstring& Filename = L"C:\\Code\\10562542_Liquid_Times_Original_Mix.wav")
	{

		WAVEDATA Result{};

		HANDLE SoundFile = CreateFile(Filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		DWORD dwError = GetLastError();
		if (dwError == ERROR_FILE_NOT_FOUND)
		{
			OutputDebugString(L"File not found\n");
			return {};
		}

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
					WaveFormatEx->cbSize = FmtChunkSize;
					memcpy_s(WaveFormatEx, sizeof(WAVEFORMATEX), ((uint8_t*)View + FmtChunkOffset + sizeof(uint32_t) * 2), FmtChunkSize); // size of the size of the formatlength (uint32_t) and skipping it +4

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
}
