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

#include <string>



#define Global static
#define Local static
#define Scoped static



#define PauseAndPlayEvent     (WM_USER+1)
#define VoiceOneGetStateEvent (WM_USER+2)

constexpr wchar_t BACKGROUND_WAV[] = L"c:\\code\\10562542_Liquid_Times_Original_Mix.wav";
constexpr wchar_t CAMERASHUTTER[] = L"c:\\code\\camerashutter.wav";
constexpr wchar_t CCLASSNAME[] = L"CSoundBox01Wnd";
constexpr wchar_t CTITLENAME[] = L"Soundbox 01";

struct PauseAndPlay
{
	bool IsPaused;
};

LRESULT CALLBACK SoundboxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);