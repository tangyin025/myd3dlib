#pragma once

#include "myOctree.h"
#include "Component.h"

class Animator;

class Controller;

class Octree;

class Actor
	: public my::OctActor
	, public Component
{
public:
	boost::shared_ptr<unsigned char> m_SerializeBuff;

	my::AABB m_aabb;

	boost::shared_ptr<Animator> m_Animator;

	boost::shared_ptr<Controller> m_Controller;

	typedef std::vector<ComponentPtr> ComponentPtrList;

	ComponentPtrList m_Cmps;

	PhysXPtr<physx::PxRigidActor> m_PxActor;

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

	Octree * GetLevel(void) const;

	my::Vector3 GetWorldPosition(void) const;

	void CopyFrom(const Actor & rhs);

	virtual ComponentPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnEnterPxScene(PhysXSceneContext * scene);

	virtual void OnLeavePxScene(PhysXSceneContext * scene);

	virtual void Update(float fElapsedTime);

	virtual my::AABB CalculateAABB(void) const;

	void UpdateAABB(void);

	virtual void UpdateWorld(void);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);

	void ClearRigidActor(void);

	void CreateRigidActor(physx::PxActorType::Enum ActorType);

	void AddComponent(ComponentPtr cmp);

	void RemoveComponent(ComponentPtr cmp);

	void ClearAllComponent(ComponentPtr cmp);

	void ChangePose(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale);

	void UpdateRigidActorPose(void);
};

typedef boost::shared_ptr<Actor> ActorPtr;
