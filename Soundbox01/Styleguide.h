#pragma once
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>

namespace tretton63
{
	constexpr COLORREF BackgroundColor = RGB(0, 0, 0);
	constexpr COLORREF ForegroundColor = RGB(255, 0, 0);

	template<typename T>
	class unique_gdi
	{
		// TODO: think about cache these, so we don't create new brushes for same color...
		T m_brush;
	public:
		
		unique_gdi(T t) {
			m_brush = t;
		}
		
		
		~unique_gdi()
		{
			DeleteObject(m_brush);
		}

		T Value()
		{
			return m_brush;
		}
	};



	static unique_gdi<HBRUSH> BackgroundBrush(CreateSolidBrush(BackgroundColor));
	/*
	PS_SOLID
The pen is solid.
PS_DASH
The pen is dashed. This style is valid only when the pen width is one or less in device units.
PS_DOT
The pen is dotted. This style is valid only when the pen width is one or less in device units.
PS_DASHDOT
The pen has alternating dashes and dots. This style is valid only when the pen width is one or less in device units.
PS_DASHDOTDOT
The pen has alternating dashes and double dots. This style is valid only when the pen width is one or less in device units.
PS_NULL
The pen is invisible.
PS_INSIDEFRAME
The pen is solid. When this pen is used in any GDI drawing function that takes a bounding rectangle, the dimensions of the figure are shrunk so that it fits entirely in the bounding rectangle, taking into account the width of the pen. This applies only to geometric pens.
	*/
	static unique_gdi<HPEN> ForegroundPen(CreatePen(PS_SOLID, 1, ForegroundColor));
	static unique_gdi<HPEN> TracklineColor(CreatePen(PS_SOLID, 1, RGB(255, 255, 255)));
	

}
