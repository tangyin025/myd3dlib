#pragma once

class MeshComponent : public my::Component
{
public:
	my::Matrix4 m_World;

	typedef std::pair<my::MaterialPtr, my::EffectPtr> MaterialPair;

	typedef std::vector<MaterialPair> MaterialPairList;

	MaterialPairList m_Materials;

	my::OgreMeshPtr m_Mesh;

public:
	MeshComponent(const my::AABB & aabb)
		: my::Component(aabb)
		, m_World(my::Matrix4::Identity())
	{
	}

	void OnMaterialLoaded(DWORD i, my::DeviceRelatedObjectBasePtr res);

	void OnEffectLoaded(DWORD i, my::DeviceRelatedObjectBasePtr res);

	virtual void UpdateLod(float dist);

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
