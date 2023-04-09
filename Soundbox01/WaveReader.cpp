#include "WaveReader.h"

namespace tretton63 
{
	//
	//void PrintWaveFormat(WAVEFORMATEX* wf)
	//{
	//	printf("Waveformat\n");
	//	printf("Tag %d\n", wf.wFormatTag);
	//	printf("Channels %d\n", wf.nChannels);
	//	printf("Samples per sec %d\n", wf.nSamplesPerSec);
	//	printf("Bits %d\n", wf.wBitsPerSample);
	//
	//}

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

	void PrintWaveFormat(WAVEFORMATEX& wf)
	{
		printf("Waveformat\n");
		printf("Tag %d\n", wf.wFormatTag);
		printf("Channels %d\n", wf.nChannels);
		printf("Samples per sec %d\n", wf.nSamplesPerSec);
		printf("Bits %d\n", wf.wBitsPerSample);
	}

}
