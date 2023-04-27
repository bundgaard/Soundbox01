#pragma once

#include <xaudio2.h>

class VoiceCallback : public IXAudio2VoiceCallback
{
public:
	VoiceCallback()
	{

	}

	~VoiceCallback()
	{

	}

	void OnStreamEnd() noexcept
	{
		OutputDebugStringW(L"Stream ended\n");
	}
	void OnVoiceProcessingPassStart(UINT32 BytesRequired) noexcept
	{
		// INFO: never ending stream of output OutputDebugStringW(L"OnVoiceProcessingPassStart\n");
	}
	void OnVoiceProcessingPassEnd() noexcept
	{
		// INFO: never ending stream of output  OutputDebugStringW(L"OnVoiceProcessingPassEnd\n");
	}
	void OnBufferEnd(void* pContext) noexcept
	{
		OutputDebugStringW(L"OnBufferEnd\n");
	}
	void OnBufferStart(void* pContext)noexcept
	{
		OutputDebugStringW(L"OnBufferStart\n");
	}
	void OnLoopEnd(void* pContext)noexcept {}
	void OnVoiceError(void* pContext, HRESULT Error) noexcept {}
};

