#pragma once
#include <cstdint>
#include <cmath>

void GenerateSineWave(int16_t* Data, int SampleRate, int Frequency)
{
	const double TS = 1.0 / double(SampleRate);
	const double Freq = double(Frequency);

	int16_t *Ptr = Data;
	double Time = 0.0;
	for (int i = 0; i < SampleRate; ++i, ++Ptr)
	{
		double Angle = (2.0 * 3.14598 * Freq) * Time;
		double Factor = 0.5 * (sin(Angle) + 1.0);
		*Ptr = static_cast<int16_t>(32768 * Factor);
		Time += TS;
	}
};