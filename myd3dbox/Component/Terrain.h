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
	my::VertexBuffer m_vb;
	my::Vector2 m_PosStart;
	my::Vector2 m_PosEnd;
	my::Vector2 m_TexStart;
	my::Vector2 m_TexEnd;

public:
	TerrainChunk(Terrain * Owner, const my::Vector2 & PosStart, const my::Vector2 & PosEnd, const my::Vector2 & TexStart, const my::Vector2 & TexEnd);

	virtual ~TerrainChunk(void);

	void CreateVertices(void);

	void UpdateVertices(void);

	void DestroyVertices(void);
};

typedef boost::shared_ptr<TerrainChunk> TerrainChunkPtr;

class Terrain
	: public RenderComponent
{
public:
	static const DWORD m_RowChunks = 8;
	static const DWORD m_ChunkRows = 64;
	typedef boost::array<unsigned short, m_ChunkRows + 1> VertTable;
	class VertTable2D
		: public boost::array<VertTable, VertTable::static_size>
	{
	public:
		VertTable2D(void);
	};
	static const VertTable2D m_VertTable;
	my::Matrix4 m_World;
	float m_HeightScale;
	float m_RowScale;
	float m_ColScale;
	float m_WrappedU;
	float m_WrappedV;
	typedef boost::array<unsigned char, m_RowChunks * m_ChunkRows + 1> Sample;
	typedef boost::array<Sample, Sample::static_size> Sample2D;
	Sample2D m_Samples;
	my::D3DVertexElementSet m_VertexElems;
	DWORD m_VertexStride;
	CComPtr<IDirect3DVertexDeclaration9> m_Decl;
	my::IndexBuffer m_ib;
	MaterialPtr m_Material;
	my::OctRoot m_Root;
	typedef boost::array<TerrainChunkPtr, m_RowChunks> ChunkArray;
	typedef boost::array<ChunkArray, ChunkArray::static_size> ChunkArray2D;
	ChunkArray2D m_Chunks;
	PhysXPtr<PxHeightField> m_HeightField;
	PhysXPtr<PxRigidActor> m_RigidActor;

	void UpdateSamples(my::Texture2DPtr HeightMap);

	void UpdateChunks(void);

	float GetSampleHeight(float x, float z);

	void CreateChunks(void);

	void CreateRigidActor(const my::Matrix4 & World);

	void CreateHeightField(void);

	void CreateShape(void);

	void UpdateShape(void);

	void CreateElements(void);

public:
	Terrain(const my::Matrix4 & World, float HeightScale, float RowScale, float ColScale, float WrappedU, float WrappedV);

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
