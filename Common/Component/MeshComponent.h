#pragma once

class MeshComponent : public my::Component
{
public:
	enum MeshType
	{
		MeshTypeStatic,
		MeshTypeAnimation,
	};

	const MeshType MESH_TYPE;

	enum DrawStage
	{
		DrawStageShadow,
		DrawStageNBuffer,
		DrawStageDBuffer,
		DrawStageCBuffer,
	};

	typedef std::vector<my::MaterialPtr> MaterialPtrList;

	MaterialPtrList m_Materials;

	my::OgreMeshPtr m_Mesh;

public:
	MeshComponent(const my::AABB & aabb, MeshType mesh_type)
		: my::Component(aabb)
		, MESH_TYPE(mesh_type)
	{
	}

	virtual void OnPreRender(my::Effect * shader, DrawStage draw_stage, DWORD AttriId) = 0;
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class StaticMeshComponent : public MeshComponent
{
public:
	my::Matrix4 m_World;

public:
	StaticMeshComponent(const my::AABB & aabb)
		: MeshComponent(aabb, MeshTypeStatic)
		, m_World(my::Matrix4::Identity())
	{
	}

	virtual void OnPreRender(my::Effect * shader, DrawStage draw_stage, DWORD AttriId);
};

class SkeletonMeshComponent : public MeshComponent
{
public:
	my::Matrix4 m_World;

	my::TransformList m_DualQuats;

public:
	SkeletonMeshComponent(const my::AABB & aabb)
		: MeshComponent(aabb, MeshTypeAnimation)
		, m_World(my::Matrix4::Identity())
	{
	}

	virtual void OnPreRender(my::Effect * shader, DrawStage draw_stage, DWORD AttriId);
};

typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;
