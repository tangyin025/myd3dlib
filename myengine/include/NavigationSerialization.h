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

	/// These are just sample areas to use consistent values across the samples.
	/// The use should specify these base on his needs.
	enum SamplePolyAreas
	{
		SAMPLE_POLYAREA_GROUND,
		SAMPLE_POLYAREA_WATER,
		SAMPLE_POLYAREA_ROAD,
		SAMPLE_POLYAREA_DOOR,
		SAMPLE_POLYAREA_GRASS,
		SAMPLE_POLYAREA_JUMP,
	};

	enum SamplePolyFlags
	{
		SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
		SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
		SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
		SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
		SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
		SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
	};

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

	void DebugDraw(struct duDebugDraw * dd, const my::Frustum & frustum, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);
};

typedef boost::shared_ptr<Navigation> NavigationPtr;
