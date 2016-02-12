#pragma once

#include "Component.h"

class Terrain;

class TerrainChunk
	: public RenderComponent
{
public:
	Terrain * m_Owner;
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

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);

	virtual void AddToPipeline(RenderPipeline * pipeline, unsigned int PassMask);
};

typedef boost::shared_ptr<TerrainChunk> TerrainChunkPtr;

class Terrain
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
	typedef std::vector<TerrainChunkPtr> TerrainChunkPtrList;
	TerrainChunkPtrList m_Chunks;
	MaterialPtr m_Material;
public:
	Terrain(DWORD RowChunks, DWORD ColChunks, DWORD ChunkRows, DWORD ChunkCols, float HeightScale, float RowScale, float ColScale);
};

typedef boost::shared_ptr<Terrain> TerrainPtr;
