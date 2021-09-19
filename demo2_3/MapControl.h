#pragma once
#include <myUi.h>
#include <boost/multi_array.hpp>
class MapControl :
	public my::Control
{
public:
	boost::multi_array<my::Texture2DPtr, 2> m_Textures;

	boost::multi_array<bool, 2> m_IsRequested;

	bool m_bMouseDrag;

	my::Vector2 m_MouseOffset;

public:
	MapControl(const char* Name);

	virtual DWORD GetControlType(void) const
	{
		return ControlTypeStatic;
	}

	void OnTextureReady(my::DeviceResourceBasePtr res, int i, int j);

	virtual void Draw(my::UIRender* ui_render, float fElapsedTime, const my::Vector2& Offset, const my::Vector2& Size);

	virtual bool HandleMouse(UINT uMsg, const my::Vector2& pt, WPARAM wParam, LPARAM lParam);
};

