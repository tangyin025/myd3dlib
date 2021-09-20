#pragma once
#include <myUi.h>
#include <boost/multi_array.hpp>
#include <boost/intrusive/list.hpp>

class LargeImageChunk
	: public boost::intrusive::list_base_hook<>
{
public:
	bool m_Requested;

	my::Texture2DPtr m_Texture;

public:
	LargeImageChunk(void)
		: m_Requested(false)
	{
	}

	virtual ~LargeImageChunk(void);
};

class LargeImage
{
public:
	typedef boost::multi_array<LargeImageChunk, 2> ChunkArray2D;

	ChunkArray2D m_Chunks;

public:
	LargeImage(void);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	void OnTextureReady(my::DeviceResourceBasePtr res, int i, int j);

	virtual void Draw(my::UIRender* ui_render, const my::Rectangle& rect, DWORD color, const my::Rectangle& clip);
};

class MapControl :
	public my::Control
{
public:
	LargeImage m_largeImg;

	bool m_bMouseDrag;

	my::Vector2 m_MouseOffset;

public:
	MapControl(const char* Name);

	virtual DWORD GetControlType(void) const
	{
		return ControlTypeStatic;
	}

	virtual void Draw(my::UIRender* ui_render, float fElapsedTime, const my::Vector2& Offset, const my::Vector2& Size);

	virtual bool HandleMouse(UINT uMsg, const my::Vector2& pt, WPARAM wParam, LPARAM lParam);
};

