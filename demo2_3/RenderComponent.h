#pragma once

class RenderComponentBase
{
public:
	enum DrawState
	{
		DrawStateShadow,
		DrawStateOpaque,
	};

	my::AABB m_AABB;

public:
	RenderComponentBase(void)
		: m_AABB(my::Vector3(FLT_MIN,FLT_MIN,FLT_MIN), my::Vector3(FLT_MAX,FLT_MAX,FLT_MAX))
	{
	}

	virtual ~RenderComponentBase(void)
	{
	}

	virtual void Draw(void) = 0;
};

typedef boost::shared_ptr<RenderComponentBase> RenderComponentBasePtr;

class MeshComponent : public RenderComponentBase
{
public:
	typedef std::pair<my::MaterialPtr, my::EffectPtr> MaterialPair;

	typedef std::vector<MaterialPair> MaterialPairList;

	MaterialPairList m_Materials;

	my::Matrix4 m_World;

	my::OgreMeshPtr m_Mesh;

public:
	MeshComponent(void)
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
	SkeletonMeshComponent(void)
	{
	}

	virtual void Draw(void);
};

typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;
