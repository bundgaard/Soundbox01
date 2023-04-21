#include "Voice.h"

Voice::Voice()
{

}

Voice::~Voice()
{
	if (m_voice)
	{
		m_voice->DestroyVoice();
	}
}

void Voice::SetVolume(FLOAT newVolume)
{
	if (m_volume != newVolume)
	{
		m_volume = newVolume;
	}
}

FLOAT Voice::GetVolume()
{
	return m_volume;
}

void Voice::LoadWave()
{
}
