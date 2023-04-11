#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mmeapi.h>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>

namespace tretton63
{

	static uint32_t Wave = 0x45564157; // <evaw>
	static uint32_t Fmt = 0x20746d66; // < tmf> 
	static uint32_t Data = 0x61746164; // <atad> 

	struct WAVEDATA {
		size_t WaveSize;
		void* Location;
	};

	size_t FindChunk(uint32_t Chunk, uint8_t* Data, size_t Offset, size_t FileSize);
	
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
	};

	void PrintWaveFormat(WAVEFORMATEX* wf, uint32_t WaveDataSize);
	std::optional<WAVEDATA> LoadWaveMMap(WAVEFORMATEX* WaveFormatEx, const std::wstring& Filename);

}
