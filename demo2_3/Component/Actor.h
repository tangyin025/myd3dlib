#pragma once

#include "RenderPipeline.h"
#include "ActorComponent.h"

class ComponentLevel;

typedef boost::shared_ptr<ComponentLevel> ComponentLevelPtr;

class LODComponent
	: public RenderComponent
{
public:
	typedef std::vector<ComponentLevelPtr> ComponentLevelPtrList;

	ComponentLevelPtrList m_lvls;

	unsigned int m_level;

public:
	LODComponent(const my::AABB & aabb)
		: RenderComponent(aabb, ComponentTypeLOD)
		, m_level(0)
	{
	}

	ComponentLevelPtr GetComponentLevel(unsigned int level);

	virtual void OnQueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<LODComponent> LODComponentPtr;

class ComponentLevel
	: public my::OctRoot
{
public:
	ComponentLevel(const my::AABB & aabb, float MinBlock)
		: OctRoot(aabb, MinBlock)
	{
	}

	template <class CmpClass>
	boost::shared_ptr<CmpClass> CreateComponent(const my::AABB & aabb)
	{
		boost::shared_ptr<CmpClass> ret(new CmpClass(aabb));
		AddComponent(ret, 0.1f);
		return ret;
	}

	MeshComponentPtr CreateMeshComponent(const my::AABB & aabb)
	{
		return CreateComponent<MeshComponent>(aabb);
	}

	ClothComponentPtr CreateClothComponent(const my::AABB & aabb)
	{
		return CreateComponent<ClothComponent>(aabb);
	}

	EmitterComponentPtr CreateEmitterComponent(const my::AABB & aabb)
	{
		return CreateComponent<EmitterComponent>(aabb);
	}

	LODComponentPtr CreateLODComponent(const my::AABB & aabb)
	{
		return CreateComponent<LODComponent>(aabb);
	}

	void QueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);
};

class Actor
	: public ComponentLevel
{
public:
	Actor(const my::AABB & aabb, float MinBlock)
		: ComponentLevel(aabb, MinBlock)
	{
	}

	virtual ~Actor(void)
	{
	}

	void Update(float fElapsedTime);
};

typedef boost::shared_ptr<Actor> ActorPtr;

typedef std::vector<ActorPtr> ActorPtrList;
