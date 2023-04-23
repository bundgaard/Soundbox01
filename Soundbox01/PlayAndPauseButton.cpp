#include "PlayAndPauseButton.h"
#include "Win32Layer.h"
#include "WindowMessages.h"

namespace tretton63
{
	PlayAndPauseTransition 
		PlayAndPause_GetState(HWND self)
	{
		auto Caption = Win32Caption(self);
		if (Caption.has_value())
		{
			std::wstring foo{ L"Play" };
			if (*Caption == foo)
			{
				return PlayAndPauseTransition::PLAY;
			}
			else
			{
				return PlayAndPauseTransition::PAUSE;
			}
		}
		return PlayAndPauseTransition::PLAY;
	}

	void
		PlayAndPause_SetCaption(HWND self, PlayAndPauseTransition state)
	{

		if (state == PlayAndPauseTransition::PLAY)
		{
			SetWindowTextW(self, L"Play");
		}
		else if (state == PlayAndPauseTransition::PAUSE)
		{
			SetWindowTextW(self, L"Pause");
		}

	}
	void
		PlayAndPause_OnClick(HWND self, HWND parent)
	{
		// Change the caption of the button Play -> Pause -> Play on each change it should also verify that we are playing the same song, if not we should reload the data for the new song.
		auto state = PlayAndPause_GetState(self);
		if (state == PlayAndPauseTransition::PLAY)
		{
			PlayAndPause_SetCaption(self, PlayAndPauseTransition::PAUSE);
			SendMessage(parent, WM_CM_PAUSEMUSIC, 0, 0);
		}
		else if (state == PlayAndPauseTransition::PAUSE)
		{
			PlayAndPause_SetCaption(self, PlayAndPauseTransition::PLAY);
			SendMessage(parent, WM_CM_PLAYMUSIC, 0, 0);
		}

	}
}