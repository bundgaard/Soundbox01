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

#include <windowsx.h>
#include <CommCtrl.h>
#pragma comment(lib, "comctl32")
#include <atlbase.h>

#include <string>



#define Global static
#define Local static
#define Scoped static

#define PauseAndPlayEvent     (WM_USER + 1)
#define VoiceOneGetStateEvent (WM_USER + 2)

#define WM_CM_LOADFILES       (WM_USER + 4)
#define WM_VOLUME_CHANGED	  (WM_USER + 5)


consteval int32_t to_rgb(int32_t ColorHex) // 00 36 00 ff
{

	int R = (ColorHex >> 24) & 0xff;
	int G = (ColorHex >> 16) & 0xff;
	int B = (ColorHex >> 8) & 0xff;
	return RGB(R, G, B);
}

static_assert(to_rgb(0x003600ff) == 0x003600);


constexpr wchar_t BACKGROUND_WAV[] = L"c:\\code\\10562542_Liquid_Times_Original_Mix.wav";
constexpr wchar_t CAMERASHUTTER[] = L"c:\\code\\camerashutter.wav";
constexpr wchar_t CCLASSNAME[] = L"CSoundBox01Wnd";
constexpr wchar_t CTITLENAME[] = L"Soundbox 01";

struct PauseAndPlay
{
	bool IsPaused;
};

LRESULT CALLBACK SoundboxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);