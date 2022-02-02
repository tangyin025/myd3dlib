#pragma once

#include "Component.h"

class dtNavMesh;

class dtNavMeshQuery;

class NavigationTileChunk
	: public my::OctEntity
{
public:
	int m_tileId;

public:
	NavigationTileChunk(int tileId)
		: m_tileId(tileId)
	{
	}

	virtual ~NavigationTileChunk(void)
	{
	}
};

typedef boost::shared_ptr<NavigationTileChunk> NavigationTileChunkPtr;

class Navigation
	: public Component
	, public my::OctRoot
{
public:
	enum { TypeID = ComponentTypeNavigation };

	boost::shared_ptr<dtNavMesh> m_navMesh;

	boost::shared_ptr<dtNavMeshQuery> m_navQuery;

	typedef std::vector<NavigationTileChunkPtr> TileChunkList;

	TileChunkList m_Chunks;

protected:
	Navigation(void);

public:
	Navigation(const char* Name);

	virtual ~Navigation(void);

	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive& ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive& ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	virtual DWORD GetComponentType(void) const
	{
		return TypeID;
	}

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	void BuildQueryAndChunks(int MaxNodes);

	void DebugDraw(struct duDebugDraw * dd);
};

typedef boost::shared_ptr<Navigation> NavigationPtr;
