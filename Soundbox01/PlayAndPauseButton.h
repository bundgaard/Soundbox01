#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>

namespace tretton63
{
	class PlayAndPauseButton
	{
		HWND m_hwnd;
		WNDPROC m_originalProc;

	public:
		PlayAndPauseButton(
			HWND parent,
			int x, int y,
			int width, int height,
			std::wstring text) :
			m_hwnd(CreateWindowExW(
				0,
				L"BUTTON",
				text.c_str(),
				WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				x, y,
				width, height, parent,
				nullptr, nullptr, nullptr))
		{
			m_originalProc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(m_hwnd, GWLP_WNDPROC));
			SetWindowLongPtrW(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(sWndProc));
			SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		}
		~PlayAndPauseButton()
		{
			SetWindowLongPtrW(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_originalProc));
			DestroyWindow(m_hwnd);
		}
		void OnClick()
		{
			OutputDebugStringW(L"OnClick\n");
		}
		void Reset()
		{

		}
	private:
		static LRESULT CALLBACK sWndProc(HWND hwnd, UINT msg, WPARAM  wparam, LPARAM lparam)
		{
			PlayAndPauseButton* pButton = reinterpret_cast<PlayAndPauseButton*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
			if (pButton != nullptr)
			{
				return pButton->WndProc(hwnd, msg, wparam, lparam);
			}
			return DefWindowProcW(hwnd, msg, wparam, lparam);
		}

		LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
		{
			switch (msg)
			{
			default:
				return CallWindowProcW(m_originalProc, hwnd, msg, wparam, lparam);
			}
		}
	};

	enum class PlayAndPauseTransition
	{
		PLAY,
		PAUSE,
	};


	void PlayAndPause_OnClick(HWND self, HWND parent);
	void PlayAndPause_Reset(HWND self);

	inline PlayAndPauseTransition PlayAndPause_GetState(HWND self);
	inline void PlayAndPause_SetCaption(HWND self, PlayAndPauseTransition state);
	

}

