#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "Animator.h"

class ActorComponent
	: public my::AABBComponent
{
public:
	my::Matrix4 m_World;

public:
	ActorComponent(const my::AABB & aabb)
		: AABBComponent(aabb)
		, m_World(my::Matrix4::Identity())
	{
	}
};

class RenderComponent
	: public ActorComponent
	, public RenderPipeline::IShaderSetter
{
public:
	RenderComponent(const my::AABB & aabb)
		: ActorComponent(aabb)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage) = 0;
};

class MeshComponent
	: public RenderComponent
{
public:
	class LOD
	{
	public:
		MeshComponent * m_owner;

		MaterialPtr m_Material;
	
		my::OgreMeshPtr m_Mesh;

		DWORD m_AttribId;

		bool m_bInstance;
	
	public:
		LOD(MeshComponent * owner)
			: m_owner(owner)
			, m_AttribId(0)
			, m_bInstance(false)
		{
		}

		virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage, RenderPipeline::MeshType mesh_type);

		virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
	};

	typedef boost::shared_ptr<LOD> LODPtr;

	typedef std::vector<LODPtr> LODPtrList;

	LODPtrList m_lods;

	DWORD m_lodId;

public:
	MeshComponent(const my::AABB & aabb)
		: RenderComponent(aabb)
		, m_lodId(0)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class SkeletonMeshComponent
	: public MeshComponent
{
public:
	AnimatorPtr m_Animator;

public:
	SkeletonMeshComponent(const my::AABB & aabb)
		: MeshComponent(aabb)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;

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
	EmitterMeshComponent(const my::AABB & aabb)
		: RenderComponent(aabb)
		, m_WorldType(WorldTypeWorld)
		, m_DirectionType(DirectionTypeCamera)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<EmitterMeshComponent> EmitterMeshComponentPtr;

