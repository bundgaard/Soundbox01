#pragma once
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
#include <cstdint>
#include <memory>
#include <optional>