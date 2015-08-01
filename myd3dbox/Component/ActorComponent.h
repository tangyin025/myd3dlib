#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "physx_ptr.hpp"

class Animator;

class Component
	: public my::AABBComponent
{
public:
	my::Matrix4 m_World;

public:
	Component(const my::AABB & aabb)
		: AABBComponent(aabb)
		, m_World(my::Matrix4::Identity())
	{
	}

	virtual ~Component(void)
	{
	}

	virtual void Update(float fElapsedTime)
	{
	}
};

typedef boost::shared_ptr<Component> ActorComponentPtr;

class RenderComponent
	: public Component
	, public RenderPipeline::IShaderSetter
{
public:
	RenderComponent(const my::AABB & aabb)
		: Component(aabb)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, unsigned int PassMask) = 0;
};

typedef boost::shared_ptr<RenderComponent> RenderComponentPtr;

class MeshComponent
	: public RenderComponent
{
public:
	my::OgreMeshPtr m_Mesh;

	MaterialPtrList m_MaterialList;

	bool m_bInstance;

public:
	MeshComponent(const my::AABB & aabb)
		: RenderComponent(aabb)
		, m_bInstance(false)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, unsigned int PassMask);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class SkeletonMeshComponent
	: public MeshComponent
{
public:
	boost::shared_ptr<Animator> m_Animator;

public:
	SkeletonMeshComponent(const my::AABB & aabb)
		: MeshComponent(aabb)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, unsigned int PassMask);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;

class IndexdPrimitiveUPComponent
	: public my::DeviceRelatedObjectBase
	, public RenderComponent
{
public:
	std::vector<D3DXATTRIBUTERANGE> m_AttribTable;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	my::Cache m_VertexData;

	DWORD m_VertexStride;

	std::vector<unsigned short> m_IndexData;

	MaterialPtrList m_MaterialList;

public:
	IndexdPrimitiveUPComponent(const my::AABB & aabb)
		: RenderComponent(aabb)
		, m_VertexStride(0)
	{
	}

	virtual void OnResetDevice(void);

	virtual void OnLostDevice(void);

	virtual void OnDestroyDevice(void);

	virtual void QueryMesh(RenderPipeline * pipeline, unsigned int PassMask);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<IndexdPrimitiveUPComponent> IndexdPrimitiveUPComponentPtr;

class ClothComponent
	: public IndexdPrimitiveUPComponent
{
public:
	my::D3DVertexElementSet m_VertexElems;

	std::vector<PxClothParticle> m_particles;

	std::vector<PxClothParticle> m_NewParticles;

	physx_ptr<PxCloth> m_Cloth;

	boost::shared_ptr<Animator> m_Animator;

public:
	ClothComponent(const my::AABB & aabb)
		: IndexdPrimitiveUPComponent(aabb)
	{
	}

	virtual void OnResetDevice(void);

	virtual void OnLostDevice(void);

	virtual void OnDestroyDevice(void);

	void OnPxThreadSubstep(float fElapsedTime);

	void UpdateCloth(const my::TransformList & dualQuaternionList);
};

typedef boost::shared_ptr<ClothComponent> ClothComponentPtr;

class EmitterComponent
	: public RenderComponent
{
public:
	my::EmitterPtr m_Emitter;

	MaterialPtr m_Material;

public:
	EmitterComponent(const my::AABB & aabb)
		: RenderComponent(aabb)
	{
	}

	virtual void Update(float fElapsedTime);

	virtual void QueryMesh(RenderPipeline * pipeline, unsigned int PassMask);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<EmitterComponent> EmitterComponentPtr;
