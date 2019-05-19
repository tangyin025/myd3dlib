#pragma once

#include "Component.h"
#include "myTexture.h"

class TerrainNode;

class Terrain2;

typedef boost::shared_ptr<TerrainNode> TerrainNodePtr;

class TerrainNode
{
public:
	Terrain2 * m_Owner;

	int m_iStart;

	int m_jStart;

	int m_NodeSize;

	my::AABB m_aabb;

	typedef boost::array<TerrainNodePtr, 4> ChildArray;

	ChildArray m_Childs;

public:
	TerrainNode(Terrain2 * Owner, int iStart, int jStart, int NodeSize);

	void Build(void);

	bool OnQuery(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void QueryAll(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void Query(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);
};

class Terrain2 : public Component, public TerrainNode
{
public:
	int m_Size;

	my::D3DVertexElementSet m_VertexElems;

	DWORD m_VertexStride;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	my::VertexBuffer m_vb;

	my::IndexBuffer m_ib;

	MaterialPtr m_Material;

	float m_HeightScale;

	my::Texture2D m_HeightMap;

	D3DXHANDLE technique_RenderScene;

	D3DXHANDLE handle_World;

	D3DXHANDLE handle_HeightScale;

	D3DXHANDLE handle_HeightTexSize;

	D3DXHANDLE handle_ChunkId;

	D3DXHANDLE handle_ChunkSize;

	D3DXHANDLE handle_HeightTexture;

	void UpdateVertices(void);

	void CreateHeightMap(void);

	D3DCOLOR GetSampleValue(void * pBits, int pitch, int i, int j) const;

	float GetSampleHeight(void * pBits, int pitch, int i, int j) const;

	my::Vector3 GetSamplePos(void * pBits, int pitch, int i, int j) const;

	float GetPosHeight(void * pBits, int pitch, float x, float z) const;

	void CreateElements(void);

	void UpdateHeightMapNormal(void);

public:
	Terrain2(int Size);

	Terrain2(void);

	virtual ~Terrain2(void);

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

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	virtual void ClearShape(void);
};

typedef boost::shared_ptr<Terrain2> Terrain2Ptr;
