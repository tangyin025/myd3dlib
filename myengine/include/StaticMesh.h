#pragma once

#include "Component.h"
#include <boost/intrusive/list.hpp>

struct StaticMeshTag;

class StaticMeshChunk
	: public my::OctEntity
	, public boost::intrusive::list_base_hook<boost::intrusive::tag<StaticMeshTag> >
{
public:
	bool m_Requested;

public:
	StaticMeshChunk(void);

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

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	void RequestResource(void);

	void ReleaseResource(void);
};

class StaticMesh
	: public Component
	, public my::OctRoot
{
public:
	enum { TypeID = ComponentTypeStaticMesh };

	float m_ChunkWidth;

	std::string m_ChunkPath;

	typedef std::map<std::pair<int, int>, StaticMeshChunk> ChunkMap;

	ChunkMap m_Chunks;

	typedef boost::intrusive::list<StaticMeshChunk, boost::intrusive::base_hook<boost::intrusive::list_base_hook<boost::intrusive::tag<StaticMeshTag> > > > ChunkSet;

	ChunkSet m_ViewedChunks;

protected:
	StaticMesh(void)
	{
	}

public:
	StaticMesh(const char* Name, const my::AABB& LocalRootAabb, float ChunkWidth)
		: Component(Name)
		, OctRoot(LocalRootAabb.m_min, LocalRootAabb.m_max)
		, m_ChunkWidth(ChunkWidth)
	{
	}

	virtual ~StaticMesh(void)
	{
		ClearAllEntity();
	}

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

	virtual void OnSetShader(IDirect3DDevice9* pd3dDevice, my::Effect* shader, LPARAM lparam);

	virtual void Update(float fElapsedTime);

	virtual void AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos);
};

typedef boost::shared_ptr<StaticMesh> StaticMeshPtr;

