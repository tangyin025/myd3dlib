#pragma once
#include <myUi.h>
class MapControl :
	public my::Control
{
public:
	MapControl(const char* Name)
		: Control(Name)
	{
	}

	virtual DWORD GetControlType(void) const
	{
		return ControlTypeStatic;
	}

	virtual void Draw(my::UIRender* ui_render, float fElapsedTime, const my::Vector2& Offset, const my::Vector2& Size);
};

