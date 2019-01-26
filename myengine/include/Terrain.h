#pragma once

#include "myTexture.h"
#include "Component.h"
#include <boost/multi_array.hpp>
#include <boost/unordered_map.hpp>

class Terrain;

class TerrainChunk
	: public my::OctActor
	, public my::Emitter
{
public:
	Terrain * m_Owner;

	my::AABB m_aabb;

	int m_Row;

	int m_Col;

	MaterialPtr m_Material;

protected:
	TerrainChunk(void);

public:
	TerrainChunk(Terrain * Owner, int Row, int Col);

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
};

typedef boost::shared_ptr<TerrainChunk> TerrainChunkPtr;

class Terrain
	: public Component
{
public:
	enum RenderType
	{
		RenderTypeTerrain,
		RenderTypeEmitter,
	};

	int m_RowChunks;

	int m_ColChunks;

	int m_ChunkSize;

	typedef boost::multi_array <unsigned short, 2> VertexArray2D;

	VertexArray2D m_VertexTable;

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

	MaterialPtr m_GrassMaterial;

	PhysXPtr<physx::PxHeightField> m_PxHeightField;

	D3DXHANDLE technique_RenderScene;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_HeightScale;

	D3DXHANDLE handle_HeightTexSize;

	D3DXHANDLE handle_ChunkId;

	D3DXHANDLE handle_ChunkSize;

	D3DXHANDLE handle_HeightTexture;

	D3DXHANDLE technique_emitter_RenderScene;

	D3DXHANDLE handle_emitter_World;

	unsigned int CalculateLod(int i, int j, const my::Vector3 & LocalViewPos);

	void CreateHeightMap(void);

	void UpdateHeightMap(my::Texture2DPtr HeightMap);

	void UpdateHeightMapNormal(void);

	void UpdateChunks(void);

	D3DCOLOR GetSampleValue(void * pBits, int pitch, int i, int j) const;

	float GetSampleHeight(void * pBits, int pitch, int i, int j) const;

	my::Vector3 GetSamplePos(void * pBits, int pitch, int i, int j) const;

	float GetPosHeight(void * pBits, int pitch, float x, float z) const;

	my::Vector3 GetPosByVertexIndex(const void * pVertices, int Row, int Col, int VertexIndex, void * pBits, int pitch);

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

	void UpdateVertices(void);

	virtual my::AABB CalculateAABB(void) const;

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void CreateHeightFieldShape(const my::Vector3 & Scale);

	virtual void ClearShape(void);
};

typedef boost::shared_ptr<Terrain> TerrainPtr;
