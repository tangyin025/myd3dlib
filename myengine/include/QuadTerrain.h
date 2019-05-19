#pragma once

#include "Component.h"

class QuadTerrainNode;

class QuadTerrain;

typedef boost::shared_ptr<QuadTerrainNode> TerrainNodePtr;

class QuadTerrainNode
{
public:
	QuadTerrain * m_Owner;

	int m_iStart;

	int m_jStart;

	int m_NodeSize;

	my::AABB m_aabb;

	typedef boost::array<TerrainNodePtr, 4> ChildArray;

	ChildArray m_Childs;

public:
	QuadTerrainNode(QuadTerrain * Owner, int iStart, int jStart, int NodeSize);

	void Build(void);

	bool OnQuery(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void QueryAll(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void Query(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);
};

class QuadTerrain : public Component, public QuadTerrainNode
{
public:
	int m_Size;

	my::D3DVertexElementSet m_VertexElems;

	DWORD m_VertexStride;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	my::VertexBuffer m_vb;

	my::IndexBuffer m_ib;

	MaterialPtr m_Material;

	D3DXHANDLE technique_RenderScene;

	D3DXHANDLE handle_World;

	void CreateElements(void);

	void UpdateVertices(void);

public:
	QuadTerrain(int Size);

	QuadTerrain(void);

	virtual ~QuadTerrain(void);

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

typedef boost::shared_ptr<QuadTerrain> Terrain2Ptr;
