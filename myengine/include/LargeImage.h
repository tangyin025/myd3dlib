#pragma once
#include <myUi.h>
#include <boost/unordered_map.hpp>
#include <boost/intrusive/list.hpp>

class LargeImage;

struct LargeImageTag;

class LargeImageChunk
	: public boost::intrusive::list_base_hook<boost::intrusive::tag<LargeImageTag> >
{
public:
	LargeImage * m_Owner;

	int m_Depth;

	int m_Row;

	int m_Col;

	bool m_Requested;

	my::Texture2DPtr m_Texture;

public:
	LargeImageChunk(LargeImage * Owner, int Depth, int Row, int Col)
		: m_Owner(Owner)
		, m_Depth(Depth)
		, m_Row(Row)
		, m_Col(Col)
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

	typedef boost::unordered_map<boost::tuple<int, int, int>, LargeImageChunk> LargeImageChunkMap;

	LargeImageChunkMap m_Chunks;

	typedef boost::intrusive::list<LargeImageChunk, boost::intrusive::base_hook<boost::intrusive::list_base_hook<boost::intrusive::tag<LargeImageTag> > > > ChunkSet;

	ChunkSet m_ViewedChunks;

public:
	LargeImage(void);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	void Draw(my::UIRender * ui_render, const my::Rectangle & rect, DWORD color, const my::Rectangle & clip, int depth);
};
