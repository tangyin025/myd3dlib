#pragma once

#include "myTexture.h"
#include "Component.h"
#include <boost/multi_array.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_container_iterator.hpp>

class Terrain;

class TerrainChunk
	: public my::OctEntity
	, public my::IResourceCallback
{
public:
	int m_Row;

	int m_Col;

	bool m_Requested;

	my::VertexBufferPtr m_vb;

protected:
	TerrainChunk(void);

public:
	TerrainChunk(int Row, int Col, Terrain * terrain);

	virtual ~TerrainChunk(void);

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

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	virtual void OnReady(my::IORequest* request);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	my::AABB CalculateAABB(Terrain * terrain) const;
};

typedef boost::shared_ptr<TerrainChunk> TerrainChunkPtr;

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

	typedef boost::multi_array<unsigned int, 2> IndexTable;

	IndexTable m_IndexTable;

	float m_HeightScale;

	my::D3DVertexElementSet m_VertexElems;

	static const DWORD m_VertexStride = 12 + 4 + 4;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	struct Fragment
	{
		unsigned int VertNum;
		unsigned int PrimitiveCount;
		my::IndexBuffer ib;
	};

	typedef boost::unordered_map<unsigned int, Fragment> FragmentMap;

	FragmentMap m_Fragment;

	std::string m_ChunkPath;

	typedef boost::multi_array<TerrainChunkPtr, 2> ChunkArray2D;

	ChunkArray2D m_Chunks;

	my::VertexBuffer m_RootVb;

	my::IndexBuffer m_RootIb;

	boost::shared_ptr<physx::PxHeightField> m_PxHeightField;

	D3DXHANDLE handle_World;

	unsigned int CalculateLod(int i, int j, const my::Vector3 & LocalViewPos) const;

	TerrainChunk * GetChunk(int i, int j)
	{
		TerrainChunk * ret = m_Chunks[i][j].get(); _ASSERT(ret->m_Row == i && ret->m_Col == j); return ret;
	}

	template <typename T>
	static T GetSampleValue(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc, int i, int j);

	template <typename T>
	static my::Vector3 GetSamplePos(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc, int i, int j, float HeightScale);

	void CreateElements(void);

	const Fragment & GetFragment(unsigned char center, unsigned char left, unsigned char top, unsigned char right, unsigned char bottom);

protected:
	Terrain(void);

public:
	Terrain(const char * Name, int RowChunks, int ColChunks, int ChunkSize, float HeightScale);

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

	virtual bool AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void CreateHeightFieldShape(unsigned int filterWord0);

	virtual void ClearShape(void);

	void UpdateHeightMap(my::Texture2D * HeightMap, float HeightScale);

	void UpdateVerticesNormal(void);

	void UpdateChunkAABB(void);

	void UpdateSplatmap(my::Texture2D * ColorMap);

	bool Raycast(const my::Vector3 & origin, const my::Vector3 & dir, my::Vector3 & hitPos, my::Vector3 & hitNormal);

	void SaveChunkData(const char* path);
};

typedef boost::shared_ptr<Terrain> TerrainPtr;

class TerrainModifier
{
public:
	struct Vertex
	{
		my::Vector3 Pos;
		D3DCOLOR Color;
		D3DCOLOR Normal;
	};

	boost::multi_array<Vertex *, 2> m_Verts;

	boost::multi_array_ref<Vertex, 2> m_RootVerts;

	Terrain * m_terrain;

public:
	explicit TerrainModifier(Terrain * terrain);

	~TerrainModifier(void);

	void Release(void);

	void GetIndices(int i, int j, int & k, int & l, int & m, int & n) const;

	std::pair<
		boost::shared_container_iterator<std::list<Vertex *> >,
		boost::shared_container_iterator<std::list<Vertex *> > >GetVertex(int i, int j);

	const Vertex & GetVertex(int i, int j) const;

	void SetHeight(int i, int j, float Height);

	float GetHeight(int i, int j) const;

	void SetColor(int i, int j, D3DCOLOR Color);

	D3DCOLOR GetColor(int i, int j) const;

	void SetNormal(int i, int j, const my::Vector3 & Normal);

	my::Vector3 GetNormal(int i, int j) const;
};
