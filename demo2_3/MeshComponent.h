#pragma once

class MeshComponent : public my::Component
{
public:
	typedef std::pair<my::MaterialPtr, my::EffectPtr> MaterialPair;

	typedef std::vector<MaterialPair> MaterialPairList;

	MaterialPairList m_Materials;

	my::Matrix4 m_World;

	my::OgreMeshPtr m_Mesh;

public:
	MeshComponent(const my::AABB & aabb)
		: my::Component(aabb)
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
