#pragma once

#include "myOctree.h"
#include "Component.h"

class Actor
	: public my::OctActor
	, public Component
{
public:
	my::AABB m_aabb;

	AnimatorPtr m_Animator;

	typedef std::vector<ComponentPtr> ComponentPtrList;

	ComponentPtrList m_Cmps;

protected:
	Actor(ComponentType Type, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb)
		: Component(Type, Position, Rotation, Scale)
		, m_aabb(aabb)
	{
	}

public:
	Actor(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb)
		: Component(ComponentTypeActor, Position, Rotation, Scale)
		, m_aabb(aabb)
	{
	}

	Actor(void)
		: Component(ComponentTypeActor, my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1))
		, m_aabb(-1,1)
	{
	}

	virtual ~Actor(void)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnEnterPxScene(physx::PxScene * scene);

	virtual void OnLeavePxScene(physx::PxScene * scene);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	void UpdateAABB(void);

	virtual void UpdateWorld(const my::Matrix4 & World);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	virtual void UpdateLod(const my::Vector3 & ViewPos);

	void AddComponent(ComponentPtr cmp);

	void RemoveComponent(ComponentPtr cmp);

	void ClearAllComponent(ComponentPtr cmp);
};

typedef boost::shared_ptr<Actor> ActorPtr;
