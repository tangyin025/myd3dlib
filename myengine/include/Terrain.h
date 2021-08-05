#pragma once

#include "myTexture.h"
#include "Component.h"
#include <boost/multi_array.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_container_iterator.hpp>
#include <fstream>

class Terrain;

class TerrainChunk
	: public my::OctEntity
{
public:
	int m_Row;

	int m_Col;

	bool m_Requested;

	unsigned int m_Lod[5];

	my::VertexBufferPtr m_Vb;

	MaterialPtr m_Material;

public:
	TerrainChunk(void)
		: m_Row(0)
		, m_Col(0)
		, m_Requested(false)
	{
		m_Lod[0] = UINT_MAX;
	}

	~TerrainChunk(void);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(m_Row);
		ar & BOOST_SERIALIZATION_NVP(m_Col);
		ar & BOOST_SERIALIZATION_NVP(m_Material);
	}

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	void RequestResource(void);

	void ReleaseResource(void);

	void OnVertexBufferReady(my::DeviceResourceBasePtr res);
};

class Terrain
	: public Component
	, public my::OctRoot
{
public:
	static const float MinBlock;

	static const float Threshold;

	int m_RowChunks;

	int m_ColChunks;

	int m_ChunkSize;

	int m_MinLodChunkSize;

	typedef boost::multi_array<unsigned int, 2> IndexTable;

	IndexTable m_IndexTable;

	struct Fragment
	{
		unsigned int VertNum;
		unsigned int PrimitiveCount;
		my::IndexBuffer ib;
	};

	typedef boost::unordered_map<unsigned int, Fragment> FragmentMap;

	FragmentMap m_Fragment;

	std::string m_ChunkPath;

	typedef boost::multi_array<TerrainChunk, 2> ChunkArray2D;

	ChunkArray2D m_Chunks;

	typedef std::set<TerrainChunk*> ChunkSet;

	ChunkSet m_ViewedChunks;

	my::VertexBuffer m_rootVb;

	my::IndexBuffer m_rootIb;

	my::D3DVertexElementSet m_VertexElems;

	static const DWORD m_VertexStride = 12 + 4 + 4;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	boost::shared_ptr<physx::PxHeightField> m_PxHeightField;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_TerrainSize;

	int CalculateLod(const my::AABB & LocalAabb, const my::Vector3 & LocalViewPos) const;

	TerrainChunk * GetChunk(int i, int j)
	{
		TerrainChunk * ret = &m_Chunks[i][j]; _ASSERT(ret->m_Row == i && ret->m_Col == j); return ret;
	}

	void CreateElements(void);

	const Fragment & GetFragment(unsigned char center, unsigned char left, unsigned char top, unsigned char right, unsigned char bottom);

protected:
	Terrain(void);

public:
	Terrain(const char * Name, int RowChunks, int ColChunks, int ChunkSize, int MinLodChunkSize);

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

	virtual void OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	physx::PxHeightField * CreateHeightField(float HeightScale, bool ShareSerializeCollection, CollectionObjMap & collectionObjs);

	void CreateHeightFieldShape(bool ShareSerializeCollection, CollectionObjMap & collectionObjs);

	virtual void ClearShape(void);
};

typedef boost::shared_ptr<Terrain> TerrainPtr;

class TerrainStream
{
public:
	Terrain* m_terrain;

	boost::multi_array<my::VertexBufferPtr, 2> m_Vbs;

	boost::multi_array<bool, 2> m_AabbDirty;

	boost::multi_array<bool, 2> m_VertDirty;

public:
	explicit TerrainStream(Terrain* terrain);

	~TerrainStream(void);

	void Release(void);

	void GetIndices(int i, int j, int& k, int& l, int& m, int& n, int& o, int& p) const;

	my::VertexBufferPtr GetVB(int k, int l);

	void SetVB(int k, int l, my::DeviceResourceBasePtr res);

	static int CalculateStreamOff(int ColChunks, int Row, int Col, int ChunkSize, int VertexStride);

	my::Vector3 GetPos(int i, int j);

	void SetPos(const my::Vector3& Pos, int i, int j, bool UpdateNormal);

	void SetPos(const my::Vector3& Pos, int k, int l, int m, int n);

	D3DCOLOR GetColor(int i, int j);

	void SetColor(D3DCOLOR Color, int i, int j);

	void SetColor(D3DCOLOR Color, int k, int l, int m, int n);

	my::Vector3 GetNormal(int i, int j);

	void SetNormal(const my::Vector3& Normal, int i, int j);

	void SetNormal(D3DCOLOR dw, int k, int l, int m, int n);

	my::RayResult RayTest(const my::Ray& local_ray);
};
