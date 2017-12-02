#pragma once

#include "Component.h"

class Terrain;

class TerrainChunk
	: public my::OctActor
{
public:
	Terrain * m_Owner;

	my::AABB m_aabb;

	int m_Row;

	int m_Column;

public:
	TerrainChunk(Terrain * Owner, int Row, int Column);

	TerrainChunk(void);

	virtual ~TerrainChunk(void);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctActor);
		ar & BOOST_SERIALIZATION_NVP(m_aabb);
		ar & BOOST_SERIALIZATION_NVP(m_Row);
		ar & BOOST_SERIALIZATION_NVP(m_Column);
	}

	void UpdateAABB(void);
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
	enum { value = 0 };
};

class Terrain
	: public RenderComponent
{
public:
	static const int ROW_CHUNKS = 2;

	static const int COL_CHUNKS = 2;

	static const int CHUNK_SIZE = 8;

	typedef boost::array<unsigned short, CHUNK_SIZE + 1> VertexArray;

	class VertexArray2D
		: public boost::array<VertexArray, VertexArray::static_size>
	{
	public:
		VertexArray2D(void);
	};

	static const VertexArray2D m_VertTable;

	float m_HeightScale;

	float m_WrappedU;

	float m_WrappedV;

	my::Texture2D m_HeightMap;

	my::D3DVertexElementSet m_VertexElems;

	DWORD m_VertexStride;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	my::VertexBuffer m_vb;

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

	typedef boost::array<TerrainChunk *, COL_CHUNKS> ChunkArray;

	typedef boost::array<ChunkArray, ROW_CHUNKS> ChunkArray2D;

	ChunkArray2D m_Chunks;

	typedef std::map<float, unsigned int> LodMap;

	LodMap m_LodMap;

	PhysXPtr<physx::PxHeightField> m_PxHeightField;

	void InitLodMap(void);

	unsigned int CalculateLod(int i, int j, const my::Vector3 & LocalViewPos);

	void CreateHeightMap(void);

	void UpdateHeightMap(my::Texture2DPtr HeightMap);

	void UpdateHeightMapNormal(void);

	void UpdateChunks(void);

	unsigned char GetSampleHeight(void * pBits, int pitch, int i, int j);

	my::Vector3 GetSamplePos(void * pBits, int pitch, int i, int j);

	void CreateElements(void);

	const Fragment & GetFragment(unsigned char center, unsigned char left, unsigned char top, unsigned char right, unsigned char bottom);

public:
	Terrain(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, float HeightScale, float WrappedU, float WrappedV);

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

	void UpdateVertices(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, DWORD AttribId);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);

	void CreateHeightFieldShape(const my::Vector3 & Scale);

	virtual void ClearShape(void);
};

typedef boost::shared_ptr<Terrain> TerrainPtr;
