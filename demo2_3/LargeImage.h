#pragma once
#include <myUi.h>
#include <boost/multi_array.hpp>
#include <boost/intrusive/list.hpp>

class LargeImage;

struct LargeImageTag;

class LargeImageChunk
	: public boost::intrusive::list_base_hook<boost::intrusive::tag<LargeImageTag> >
{
public:
	LargeImage* m_Owner;

	int m_Row;

	int m_Col;

	bool m_Requested;

	my::Texture2DPtr m_Texture;

public:
	LargeImageChunk(void)
		: m_Owner(NULL)
		, m_Row(0)
		, m_Col(0)
		, m_Requested(false)
	{
	}

	virtual ~LargeImageChunk(void);

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	void OnTextureReady(my::DeviceResourceBasePtr res);
};

class LargeImage
{
public:
	std::string m_TexturePath;

	typedef boost::multi_array<LargeImageChunk, 2> ChunkArray2D;

	ChunkArray2D m_Chunks;

	typedef boost::intrusive::list<LargeImageChunk, boost::intrusive::base_hook<boost::intrusive::list_base_hook<boost::intrusive::tag<LargeImageTag> > > > ChunkSet;

	ChunkSet m_ViewedChunks;

public:
	LargeImage(void);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Draw(my::UIRender* ui_render, const my::Rectangle& rect, DWORD color, const my::Rectangle& clip);
};
