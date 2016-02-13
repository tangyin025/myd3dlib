#pragma once

#include "Component.h"
#include "PhysXPtr.h"

class Terrain;

class TerrainChunk
	: public my::OctComponent
{
public:
	Terrain * m_Owner;
	my::AABB m_aabb;
	CComPtr<IDirect3DVertexDeclaration9> m_Decl;
	DWORD m_VertexStride;
	my::VertexBuffer m_vb;
	my::IndexBuffer m_ib;
	my::Vector2 m_PosStart;
	my::Vector2 m_PosEnd;
	my::Vector2 m_TexStart;
	my::Vector2 m_TexEnd;
	my::D3DVertexElementSet m_VertexElems;

public:
	TerrainChunk(Terrain * Owner, const my::Vector2 & PosStart, const my::Vector2 & PosEnd, const my::Vector2 & TexStart, const my::Vector2 & TexEnd);

	virtual ~TerrainChunk(void);

	void CreateVertices(void);

	void DestroyVertices(void);
};

typedef boost::shared_ptr<TerrainChunk> TerrainChunkPtr;

class Terrain
	: public RenderComponent
{
public:
	my::Matrix4 m_World;
	DWORD m_RowChunks;
	DWORD m_ColChunks;
	DWORD m_ChunkRows;
	DWORD m_ChunkCols;
	float m_HeightScale;
	float m_RowScale;
	float m_ColScale;
	float m_WrappedU;
	float m_WrappedV;
	MaterialPtr m_Material;
	std::vector<PxHeightFieldSample> m_Samples;
	typedef std::vector<TerrainChunkPtr> TerrainChunkPtrList;
	TerrainChunkPtrList m_Chunks;
	my::OctRoot m_Root;
	PhysXPtr<PxHeightField> m_HeightField;
	PhysXPtr<PxRigidActor> m_RigidActor;

	void CreateChunks(void);

	void CreateRigidActor(const my::Matrix4 & World);

	void CreateHeightField(void);

	float GetSampleHeight(float x, float z);

public:
	Terrain(const my::Matrix4 & World, DWORD RowChunks, DWORD ColChunks, DWORD ChunkRows, DWORD ChunkCols, float HeightScale, float RowScale, float ColScale, float WrappedU, float WrappedV);

	Terrain(void);

	virtual ~Terrain(void);

	friend class boost::serialization::access;
	
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);
};

typedef boost::shared_ptr<Terrain> TerrainPtr;
