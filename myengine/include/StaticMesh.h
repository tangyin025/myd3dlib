#pragma once

#include "Component.h"
#include <boost/intrusive/list.hpp>

struct StaticMeshTag;

class StaticMeshChunk
	: public my::OctEntity
	, public boost::intrusive::list_base_hook<boost::intrusive::tag<StaticMeshTag> >
{
public:
	int m_SubMeshId;

public:
	StaticMeshChunk(int SubMeshId)
		: m_SubMeshId(SubMeshId)
	{
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar& BOOST_SERIALIZATION_NVP(m_SubMeshId);
	}
};

class StaticMesh
	: public MeshComponent
	, public my::OctRoot
{
public:
	enum { TypeID = ComponentTypeStaticMesh };

	float m_ChunkWidth;

	typedef std::map<int, StaticMeshChunk> ChunkMap;

	ChunkMap m_Chunks;

protected:
	StaticMesh(void)
	{
	}

public:
	StaticMesh(const char* Name, const my::AABB& LocalRootAabb, float ChunkWidth)
		: MeshComponent(Name)
		, m_ChunkWidth(ChunkWidth)
		, OctRoot(LocalRootAabb.m_min, LocalRootAabb.m_max)
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

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos);

	void AddChunk(int SubMeshId, const my::AABB& aabb);
};

typedef boost::shared_ptr<StaticMesh> StaticMeshPtr;

