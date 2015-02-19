#pragma once

#include "myOctree.h"
#include "RenderPipeline.h"

class MeshComponent
	: public my::AABBComponent
	, public RenderPipeline::IShaderSetter
{
public:
	class LOD
	{
	public:
		typedef std::vector<my::MaterialPtr> MaterialPtrList;
	
		MaterialPtrList m_Materials;
	
		my::OgreMeshPtr m_Mesh;
	
	public:
		LOD(void)
		{
		}
	};

	typedef boost::shared_ptr<LOD> LODPtr;

	typedef std::vector<LODPtr> LODPtrList;

	LODPtrList m_lods;

	DWORD m_lodId;

	my::Matrix4 m_World;

public:
	MeshComponent(void)
		: AABBComponent(my::Vector3(-1,-1,-1),my::Vector3(1,1,1))
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
	my::TransformList m_DualQuats;

public:
	SkeletonMeshComponent(void)
		: MeshComponent()
	{
	}

	virtual void QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;
