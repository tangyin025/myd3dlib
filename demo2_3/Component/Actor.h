#pragma once

#include "RenderPipeline.h"
#include "ActorComponent.h"

class ComponentLevel;

typedef boost::shared_ptr<ComponentLevel> ComponentLevelPtr;

class RenderComponentLod
	: public RenderComponent
{
public:
	typedef std::vector<ComponentLevelPtr> ComponentLevelPtrList;

	ComponentLevelPtrList m_lvls;

public:
	RenderComponentLod(const my::AABB & aabb)
		: RenderComponent(aabb, ComponentTypeLOD)
	{
	}

	ComponentLevelPtr GetComponentLevel(unsigned int level);
};

class ComponentLevel
	: public my::OctRoot
{
public:
	ComponentLevel(const my::AABB & aabb, float MinBlock)
		: OctRoot(aabb, MinBlock)
	{
	}

	template <class CmpClass>
	boost::shared_ptr<CmpClass> CreateComponent(const my::AABB & aabb = my::AABB(-FLT_MAX,FLT_MAX))
	{
		boost::shared_ptr<CmpClass> ret(new CmpClass(aabb));
		AddComponent(ret, 0.1f);
		return ret;
	}
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

	void QueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);
};

typedef boost::shared_ptr<Actor> ActorPtr;

typedef std::vector<ActorPtr> ActorPtrList;
