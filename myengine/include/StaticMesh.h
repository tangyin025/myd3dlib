#pragma once

#include "Component.h"
#include <boost/intrusive/list.hpp>

struct StaticMeshTag;

class StaticMeshChunk
	: public my::OctEntity
	, public boost::intrusive::list_base_hook<boost::intrusive::tag<StaticMeshTag> >
{
public:
	int m_Row;

	int m_Col;

	bool m_Requested;

	int m_Lod;

	my::OgreMeshPtr m_Mesh;

public:
	StaticMeshChunk(int Row, int Col)
		: m_Row(Row)
		, m_Col(Col)
		, m_Requested(false)
		, m_Lod(INT_MAX)
	{
	}

	virtual ~StaticMeshChunk(void);

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Row);
		ar & BOOST_SERIALIZATION_NVP(m_Col);
	}

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	void RequestResource(void);

	void ReleaseResource(void);

	static std::string MakeChunkPath(const std::string& ChunkPath, int Row, int Col);

	void OnChunkMeshReady(my::DeviceResourceBasePtr res);
};

class StaticMesh
	: public Component
	, public my::OctRoot
{
public:
	enum { TypeID = ComponentTypeStaticMesh };

	float m_ChunkWidth;

	std::string m_ChunkPath;

	static const int LastLod = 1;

	float m_ChunkLodScale;

	typedef std::map<std::pair<int, int>, StaticMeshChunk> ChunkMap;

	ChunkMap m_Chunks;

	typedef boost::intrusive::list<StaticMeshChunk, boost::intrusive::base_hook<boost::intrusive::list_base_hook<boost::intrusive::tag<StaticMeshTag> > > > ChunkSet;

	ChunkSet m_ViewedChunks;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_MeshColor;

protected:
	StaticMesh(void)
		: m_ChunkWidth(1.0f)
		, m_ChunkLodScale(1.0f)
		, handle_World(NULL)
		, handle_MeshColor(NULL)
	{
	}

public:
	StaticMesh(const char* Name, const my::AABB& LocalRootAabb, float ChunkWidth)
		: Component(Name)
		, OctRoot(LocalRootAabb.m_min, LocalRootAabb.m_max)
		, m_ChunkWidth(ChunkWidth)
		, m_ChunkLodScale(1.0f)
		, handle_World(NULL)
		, handle_MeshColor(NULL)
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

	virtual void OnResetShader(void);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(IDirect3DDevice9* pd3dDevice, my::Effect* shader, LPARAM lparam);

	virtual void Update(float fElapsedTime);

	virtual void AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos);

	StaticMeshChunk * GetChunk(int i, int j);
};

typedef boost::shared_ptr<StaticMesh> StaticMeshPtr;
