#pragma once

#include "myTexture.h"
#include "Component.h"
#include <boost/multi_array.hpp>
#include <boost/unordered_map.hpp>

class TerrainChunk
	: public my::OctEntity
{
public:
	my::AABB m_aabb;

	int m_Row;

	int m_Col;

	my::VertexBuffer m_vb;

protected:
	TerrainChunk(void);

public:
	TerrainChunk(int Row, int Col, int ChunkSize);

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

	void UpdateAABB(void);

	template <typename T>
	void UpdateVertices(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc, float HeightScale);

	void UpdateColors(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc);
};

typedef boost::shared_ptr<TerrainChunk> TerrainChunkPtr;

class Terrain
	: public Component
	, public my::OctRoot
{
public:
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

	typedef boost::multi_array<TerrainChunkPtr, 2> ChunkArray2D;

	ChunkArray2D m_Chunks;

	boost::shared_ptr<physx::PxHeightField> m_PxHeightField;

	D3DXHANDLE handle_World;

	unsigned int CalculateLod(int i, int j, const my::Vector3 & LocalViewPos);

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

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void CreateHeightFieldShape(unsigned int filterWord0);

	virtual void ClearShape(void);

	void UpdateHeightMap(my::Texture2D * HeightMap, float HeightScale);

	void UpdateSplatmap(my::Texture2D * ColorMap);
};

typedef boost::shared_ptr<Terrain> TerrainPtr;
