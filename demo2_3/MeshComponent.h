#pragma once

class MeshComponentBase
{
public:
	enum DrawState
	{
		DrawStateShadow,
		DrawStateOpaque,
	};

public:
	virtual ~MeshComponentBase(void)
	{
	}

	virtual void Draw(DrawState State, const my::Matrix4 & ParentWorld = my::Matrix4::identity) = 0;
};

typedef boost::shared_ptr<MeshComponentBase> MeshComponentBasePtr;

class MeshComponent : public MeshComponentBase
{
public:
	my::OgreMeshPtr m_Mesh;

	typedef std::pair<my::MaterialPtr, my::EffectPtr> MaterialPair;

	typedef std::vector<MaterialPair> MaterialPairList;

	MaterialPairList m_Materials;

	my::Matrix4 m_World;

public:
	MeshComponent(void)
	{
	}

	virtual void Draw(DrawState State, const my::Matrix4 & ParentWorld = my::Matrix4::identity);
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

	virtual void Draw(DrawState State, const my::Matrix4 & ParentWorld = my::Matrix4::identity);
};

typedef boost::shared_ptr<SkeletonMeshComponent> SkeletonMeshComponentPtr;
