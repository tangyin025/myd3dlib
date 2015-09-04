#pragma once

#include "RenderPipeline.h"
#include "ActorComponent.h"
#include "Animator.h"

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
	LODComponent(const my::AABB & aabb, const my::Matrix4 & World)
		: RenderComponent(aabb, World, ComponentTypeLOD)
		, m_level(0)
	{
	}

	ComponentLevel * GetComponentLevel(unsigned int level);

	virtual void OnQueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	virtual void OnSetShader(my::Effect * shader, DWORD AttribId);
};

typedef boost::shared_ptr<LODComponent> LODComponentPtr;

class ComponentLevel
	: public my::OctRoot
{
public:
	typedef std::vector<AnimatorPtr> AnimatorPtrList;

	AnimatorPtrList m_AnimatorList;

public:
	ComponentLevel(const my::AABB & aabb, float MinBlock)
		: OctRoot(aabb, MinBlock)
	{
	}

	void QueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	template <class CmpClass>
	boost::shared_ptr<CmpClass> CreateComponent(const my::AABB & aabb, const my::Matrix4 & World)
	{
		// ! do not call new in header file
		boost::shared_ptr<CmpClass> ret(new CmpClass(aabb.transform(World), World));
		AddComponent(ret, 0.1f);
		return ret;
	}

	template <class AniClass>
	boost::shared_ptr<AniClass> CreateAnimator(void)
	{
		boost::shared_ptr<AniClass> ret(new AniClass());
		m_AnimatorList.push_back(ret);
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

	void OnPxThreadSubstep(float fElapsedTime);
};

typedef boost::shared_ptr<Actor> ActorPtr;

typedef std::vector<ActorPtr> ActorPtrList;
