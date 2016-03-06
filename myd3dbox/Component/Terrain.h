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

	int m_Row;

	int m_Column;

	unsigned char m_lod;

	my::VertexBuffer m_vb;

public:
	TerrainChunk(Terrain * Owner, int Row, int Column);

	TerrainChunk(void);

	virtual ~TerrainChunk(void);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_aabb);
		ar & BOOST_SERIALIZATION_NVP(m_Row);
		ar & BOOST_SERIALIZATION_NVP(m_Column);
	}

	void CreateVertices(void);

	void UpdateVertices(void);

	void DestroyVertices(void);
};

typedef boost::shared_ptr<TerrainChunk> TerrainChunkPtr;

template <int N>
struct Quad
{
	enum { value = Quad<N / 2>::value + 1 };
};

template <>
struct Quad<1>
{
	enum { value = 1 };
};

class Terrain
	: public RenderComponent
{
public:
	static const DWORD m_RowChunks = 8;

	static const DWORD m_ColChunks = 8;

	static const DWORD m_ChunkRows = 64;

	typedef boost::array<unsigned short, m_ChunkRows + 1> VertexArray;

	class VertexArray2D
		: public boost::array<VertexArray, VertexArray::static_size>
	{
	public:
		VertexArray2D(void);
	};

	static const VertexArray2D m_VertTable;

	my::Matrix4 m_World;

	float m_HeightScale;

	float m_RowScale;

	float m_ColScale;

	float m_WrappedU;

	float m_WrappedV;

	typedef boost::array<unsigned char, m_ColChunks * m_ChunkRows + 1> SampleArray;

	typedef boost::array<SampleArray, m_RowChunks * m_ChunkRows + 1> SampleArray2D;

	SampleArray2D m_Samples;

	my::D3DVertexElementSet m_VertexElems;

	DWORD m_VertexStride;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	struct Fragment
	{
		unsigned int VertNum;
		unsigned int PrimitiveCount;
		my::IndexBuffer ib;
	};

	typedef boost::unordered_map<unsigned int, Fragment> FragmentMap;

	FragmentMap m_Fragment;

	MaterialPtr m_Material;

	my::OctRoot m_Root;

	typedef boost::array<TerrainChunkPtr, m_ColChunks> ChunkArray;

	typedef boost::array<ChunkArray, m_RowChunks> ChunkArray2D;

	ChunkArray2D m_Chunks;

	typedef boost::array<float, Quad<m_ChunkRows>::value> LodDistanceList;

	LodDistanceList m_LodDistanceSq;

	PhysXPtr<PxHeightField> m_HeightField;

	PhysXPtr<PxRigidActor> m_RigidActor;

	void CalcLodDistanceSq(void);

	void UpdateSamples(my::Texture2DPtr HeightMap);

	void UpdateChunks(void);

	float GetSampleHeight(float x, float z);

	float GetSampleHeight(int i, int j);

	my::Vector3 GetSamplePos(int i, int j);

	void CreateRigidActor(const my::Matrix4 & World);

	void CreateHeightField(void);

	void CreateShape(void);

	void UpdateShape(void);

	void CreateElements(void);

	const Fragment & GetFragment(unsigned char center, unsigned char left, unsigned char top, unsigned char right, unsigned char bottom);

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

	virtual void UpdateLod(const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);
};

typedef boost::shared_ptr<Terrain> TerrainPtr;
