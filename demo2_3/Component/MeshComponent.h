#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"
#include "MeshAnimator.h"

class RenderComponent
	: public my::AABBComponent
	, public RenderPipeline::IShaderSetter
{
public:
	RenderComponent(const my::AABB & aabb)
		: AABBComponent(aabb)
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

		MaterialPtrList m_Materials;
	
		my::OgreMeshPtr m_Mesh;
	
	public:
		LOD(MeshComponent * owner)
			: m_owner(owner)
		{
		}

		virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage, RenderPipeline::MeshType mesh_type);

		virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
	};

	typedef boost::shared_ptr<LOD> LODPtr;

	typedef std::vector<LODPtr> LODPtrList;

	LODPtrList m_lods;

	DWORD m_lodId;

	my::Matrix4 m_World;

public:
	MeshComponent(const my::AABB & aabb)
		: RenderComponent(aabb)
		, m_lodId(0)
		, m_World(my::Matrix4::Identity())
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
	MeshAnimatorPtr m_Animator;

public:
	SkeletonMeshComponent(const my::AABB & aabb)
		: MeshComponent(aabb)
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;
