#pragma once

class RenderComponent
{
public:
	my::AABB m_AABB;

public:
	RenderComponent(const my::AABB & aabb)
		: m_AABB(aabb)
	{
	}

	virtual ~RenderComponent(void)
	{
	}

	virtual void Draw(void) = 0;
};

typedef boost::shared_ptr<RenderComponent> RenderComponentPtr;

class MeshComponent : public RenderComponent
{
public:
	typedef std::pair<my::MaterialPtr, my::EffectPtr> MaterialPair;

	typedef std::vector<MaterialPair> MaterialPairList;

	MaterialPairList m_Materials;

	my::Matrix4 m_World;

	my::OgreMeshPtr m_Mesh;

public:
	MeshComponent(const my::AABB & aabb)
		: RenderComponent(aabb)
	{
	}

	virtual void Draw(void);
};

typedef boost::shared_ptr<MeshComponent> MeshComponentPtr;

class SkeletonMeshComponent : public MeshComponent
{
public:
	my::TransformList m_DualQuats;

public:
	SkeletonMeshComponent(const my::AABB & aabb)
		: MeshComponent(aabb)
	{
	}

	virtual void Draw(void);
};

typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;
