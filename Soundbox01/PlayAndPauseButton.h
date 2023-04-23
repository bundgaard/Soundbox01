#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace tretton63
{
	class PlayAndPauseButton
	{
	};

	enum class PlayAndPauseTransition
	{
		PLAY,
		PAUSE,
	};


	void PlayAndPause_OnClick(HWND self, HWND parent);

	inline PlayAndPauseTransition PlayAndPause_GetState(HWND self);
	inline void PlayAndPause_SetCaption(HWND self, PlayAndPauseTransition state);

}

