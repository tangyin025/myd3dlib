#pragma once

#include "Component.h"
#include <boost/multi_array.hpp>

class Terrain;

class TerrainChunk
	: public my::OctActor
	, public RenderPipeline::IShaderSetter
{
public:
	Terrain * m_Owner;

	my::AABB m_aabb;

	int m_Row;

	int m_Column;

	MaterialPtr m_Material;

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
		ar & BOOST_SERIALIZATION_NVP(m_Material);
	}

	void UpdateAABB(void);

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, DWORD AttribId);
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
	: public Component
{
public:
	static const int ROW_CHUNKS = 8;

	static const int COL_CHUNKS = 8;

	static const int CHUNK_SIZE = 32;

	class VertexArray2D : public boost::multi_array < unsigned short, 2 >
	{
	public:
		VertexArray2D(void);
	};

	static const VertexArray2D m_VertexTable;

	float m_HeightScale;

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

	bool m_bNavigation;

	my::OctRoot m_Root;

	typedef boost::multi_array<TerrainChunk *, 2> ChunkArray2D;

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

	D3DCOLOR GetSampleValue(void * pBits, int pitch, int i, int j);

	my::Vector3 GetSamplePos(void * pBits, int pitch, int i, int j);

	my::Vector3 GetPosByVertexIndex(const void * pVertices, int Row, int Column, int VertexIndex, void * pBits, int pitch);

	void CreateElements(void);

	const Fragment & GetFragment(unsigned char center, unsigned char left, unsigned char top, unsigned char right, unsigned char bottom);

protected:
	Terrain(void);

public:
	Terrain(float HeightScale, float WrappedU, float WrappedV);

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

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);

	void CreateHeightFieldShape(const my::Vector3 & Scale);

	virtual void ClearShape(void);
};

typedef boost::shared_ptr<Terrain> TerrainPtr;
