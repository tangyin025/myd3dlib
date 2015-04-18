#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "Actor.h"

class ActorComponent
	: public my::AABBComponent
	, public Actor::Attacher
{
public:
	ActorComponent(Actor * Owner)
		: AABBComponent(my::AABB(-FLT_MAX,FLT_MAX))
		, Attacher(Owner)
	{
	}

	virtual ~ActorComponent(void)
	{
	}

	virtual void Update(float fElapsedTime)
	{
	}
};

typedef boost::shared_ptr<ActorComponent> ActorComponentPtr;

class RenderComponent
	: public ActorComponent
	, public RenderPipeline::IShaderSetter
{
public:
	RenderComponent(Actor * Owner)
		: ActorComponent(Owner)
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
	MeshComponent(Actor * Owner)
		: RenderComponent(Owner)
		, m_bInstance(false)
	{
	}

	void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage, RenderPipeline::MeshType mesh_type);

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class SkeletonMeshComponent
	: public MeshComponent
{
public:
	SkeletonMeshComponent(Actor * Owner)
		: MeshComponent(Owner)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;

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
	IndexdPrimitiveUPComponent(Actor * Owner)
		: RenderComponent(Owner)
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
	ClothComponent(Actor * Owner)
		: IndexdPrimitiveUPComponent(Owner)
		, m_Cloth(NULL)
	{
	}

	void OnPxThreadSubstep(float fElapsedTime);

	void UpdateCloth(const my::TransformList & dualQuaternionList);
};

typedef boost::shared_ptr<ClothComponent> ClothComponentPtr;

class EmitterComponent
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
	EmitterComponent(Actor * Owner)
		: RenderComponent(Owner)
		, m_WorldType(WorldTypeWorld)
		, m_DirectionType(DirectionTypeCamera)
	{
	}

	virtual void Update(float fElapsedTime);

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<EmitterComponent> EmitterComponentPtr;
