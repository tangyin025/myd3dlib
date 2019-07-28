#pragma once

#include "myTexture.h"
#include "Component.h"
#include <boost/multi_array.hpp>
#include <boost/unordered_map.hpp>

class Terrain;

class TerrainChunk
	: public my::OctActor
{
public:
	my::AABB m_aabb;

	int m_Row;

	int m_Col;

	my::VertexBuffer m_vb;

	MaterialPtr m_Material;

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
	void UpdateVertices(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc);

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

	static const DWORD m_VertexStride = 48;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	struct Fragment
	{
		unsigned int VertNum;
		unsigned int PrimitiveCount;
		my::IndexBuffer ib;
	};

	typedef boost::unordered_map<unsigned int, Fragment> FragmentMap;

	FragmentMap m_Fragment;

	typedef boost::multi_array<TerrainChunk *, 2> ChunkArray2D;

	ChunkArray2D m_Chunks;

	boost::shared_ptr<physx::PxHeightField> m_PxHeightField;

	D3DXHANDLE technique_RenderScene;

	D3DXHANDLE handle_World;

	unsigned int CalculateLod(int i, int j, const my::Vector3 & LocalViewPos);

	void UpdateHeightField(my::Texture2D * HeightMap);

	TerrainChunk * GetChunk(int i, int j)
	{
		TerrainChunk * ret = m_Chunks[i][j]; _ASSERT(ret->m_Row == i && ret->m_Col == j); return ret;
	}

	template <typename T>
	T GetSampleValue(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc, int i, int j) const
	{
		_ASSERT(i >= 0 && i < (int)desc.Height);
		_ASSERT(j >= 0 && j < (int)desc.Width);
		return *(T *)((unsigned char *)lrc.pBits + i * lrc.Pitch + j * sizeof(T));
	}

	template <typename T>
	float GetSampleHeight(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc, int i, int j) const
	{
		return m_HeightScale * GetSampleValue<T>(desc, lrc, i, j);
	}

	template <typename T>
	my::Vector3 GetSamplePos(D3DSURFACE_DESC & desc, D3DLOCKED_RECT & lrc, int i, int j) const
	{
		return my::Vector3((float)j, GetSampleHeight<T>(desc, lrc, my::Clamp<int>(i, 0, desc.Height - 1), my::Clamp<int>(j, 0, desc.Width - 1)), (float)i);
	}

	void CreateElements(void);

	const Fragment & GetFragment(unsigned char center, unsigned char left, unsigned char top, unsigned char right, unsigned char bottom);

protected:
	Terrain(void);

public:
	Terrain(int RowChunks, int ColChunks, int ChunkSize, float HeightScale);

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

	virtual void OnShaderChanged(void);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void CreateHeightFieldShape(void);

	virtual void ClearShape(void);
};

typedef boost::shared_ptr<Terrain> TerrainPtr;
