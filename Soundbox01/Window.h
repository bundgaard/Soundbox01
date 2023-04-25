#pragma once
#include "Application.h"
#include "Frame.h"
class Window :
	public Frame
{
public:
	Window(
		int X, int Y,
		int Width, int Height,
		HBRUSH BackgroundBrush,
		std::wstring const& Title) :
		Frame(X, Y, Width, Height, BackgroundBrush, Title, L"GABOR")
	{

	}

	virtual void OnCreate()
	{
		OutputDebugStringW(L"With love from Window\n");
	}


};

