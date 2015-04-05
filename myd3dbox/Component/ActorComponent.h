#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
//#include "Animator.h"

class ActorComponent
	: public my::AABBNode
{
public:
	my::Matrix4 m_World;

public:
	ActorComponent(void)
		: AABBNode(my::AABB(FLT_MIN,FLT_MAX))
		, m_World(my::Matrix4::Identity())
	{
	}

	virtual ~ActorComponent(void)
	{
	}
};

typedef boost::shared_ptr<ActorComponent> ActorComponentPtr;

class RenderComponent
	: public ActorComponent
	, public RenderPipeline::IShaderSetter
{
public:
	RenderComponent(void)
		: ActorComponent()
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage) = 0;
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
	MeshComponent(void)
		: RenderComponent()
		, m_bInstance(false)
	{
	}

	void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage, RenderPipeline::MeshType mesh_type);

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;
//
//class SkeletonMeshComponent
//	: public MeshComponent
//{
//public:
//	AnimatorPtr m_Animator;
//
//public:
//	SkeletonMeshComponent(void)
//		: MeshComponent()
//	{
//	}
//
//	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);
//
//	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
//};
//
//typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;

class IndexdPrimitiveUPComponent
	: public RenderComponent
{
public:
	std::vector<D3DXATTRIBUTERANGE> m_AttribTable;

	CComPtr<IDirect3DVertexDeclaration9> m_Decl;

	my::Cache m_VertexData;

	DWORD m_VertexStride;

	std::vector<unsigned short> m_IndexData;

	MaterialPtrList m_MaterialList;

public:
	IndexdPrimitiveUPComponent(void)
		: RenderComponent()
		, m_VertexStride(0)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

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

	PxCloth * m_Cloth;

public:
	ClothComponent(void)
		: IndexdPrimitiveUPComponent()
		, m_Cloth(NULL)
	{
	}

	void UpdateCloth(const my::TransformList & dualQuaternionList);
};

typedef boost::shared_ptr<ClothComponent> ClothComponentPtr;

class EmitterMeshComponent
	: public RenderComponent
{
public:
	enum WorldType
	{
		WorldTypeWorld,
		WorldTypeLocal,
	};

	WorldType m_WorldType;

	enum DirectionType
	{
		DirectionTypeCamera,
		DirectionTypeVertical,
		DirectionTypeHorizontal,
	};

	DirectionType m_DirectionType;

	my::EmitterPtr m_Emitter;

	MaterialPtr m_Material;

public:
	EmitterMeshComponent(void)
		: RenderComponent()
		, m_WorldType(WorldTypeWorld)
		, m_DirectionType(DirectionTypeCamera)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<EmitterMeshComponent> EmitterMeshComponentPtr;
