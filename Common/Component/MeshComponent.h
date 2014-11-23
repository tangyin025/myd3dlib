#pragma once

class MeshLOD
{
public:
	typedef std::vector<my::MaterialPtr> MaterialPtrList;

	MaterialPtrList m_Materials;

	my::OgreMeshPtr m_Mesh;

public:
	MeshLOD(void)
	{
	}

	virtual ~MeshLOD(void)
	{
	}

	virtual void OnPreRender(my::Effect * shader, DWORD draw_stage, DWORD AttriId);
};

typedef boost::shared_ptr<MeshLOD> MeshLODPtr;

class MeshComponent
{
public:
	enum MeshType
	{
		MeshTypeStatic,
		MeshTypeAnimation,
	};

	const MeshType m_MeshType;

	typedef std::vector<MeshLODPtr> MeshLODPtrList;

	MeshLODPtrList m_Lod;

public:
	MeshComponent(MeshType mesh_type)
		: m_MeshType(mesh_type)
	{
	}

	virtual void OnPreRender(my::Effect * shader, DWORD draw_stage);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class StaticMeshComponent : public MeshComponent
{
public:
	my::Matrix4 m_World;

public:
	StaticMeshComponent(void)
		: MeshComponent(MeshTypeStatic)
		, m_World(my::Matrix4::Identity())
	{
	}

	virtual void OnPreRender(my::Effect * shader, DWORD draw_stage);
};

typedef boost::shared_ptr<StaticMeshComponent> StaticMeshComponentPtr;

class SkeletonMeshComponent : public MeshComponent
{
public:
	my::Matrix4 m_World;

	my::TransformList m_DualQuats;

public:
	SkeletonMeshComponent(void)
		: MeshComponent(MeshTypeAnimation)
		, m_World(my::Matrix4::Identity())
	{
	}

	virtual void OnPreRender(my::Effect * shader, DWORD draw_stage);
};

typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;
