#pragma once

#include "myOctree.h"
#include "Component.h"

class Animator;

class Controller;

class Actor;

typedef boost::shared_ptr<Actor> ActorPtr;

class Actor
	: public my::OctActor
{
public:
	boost::shared_ptr<unsigned char> m_SerializeBuff;

	my::AABB m_aabb;

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Vector3 m_Scale;

	my::Matrix4 m_World;

	bool m_Requested;

	boost::shared_ptr<Animator> m_Animator;

	boost::shared_ptr<Controller> m_Controller;

	typedef std::vector<ComponentPtr> ComponentPtrList;

	ComponentPtrList m_Cmps;

	PhysXPtr<physx::PxRigidActor> m_PxActor;

	//Actor * m_Parent;

	//enum SlotType
	//{
	//	SlotTypeOffset,
	//	SlotTypeBone,
	//	SlotTypeCloth,
	//};

	//SlotType m_SlotType;

	//unsigned int m_SlotParam;

	//typedef std::vector<ActorPtr> ActorPtrList;

	//ActorPtrList m_Childs;

protected:
	Actor(void)
		: m_aabb(my::AABB::Invalid())
		, m_Position(0, 0, 0)
		, m_Rotation(my::Quaternion::Identity())
		, m_Scale(1, 1, 1)
		, m_World(my::Matrix4::Identity())
		, m_Requested(false)
	{
	}

public:
	Actor(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb)
		: m_aabb(aabb)
		, m_Position(Position)
		, m_Rotation(Rotation)
		, m_Scale(Scale)
		, m_World(my::Matrix4::Identity())
		, m_Requested(false)
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

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	void CopyFrom(const Actor & rhs);

	virtual ActorPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void OnEnterPxScene(PhysXSceneContext * scene);

	virtual void OnLeavePxScene(PhysXSceneContext * scene);

	virtual void OnUpdatePxTransform(const physx::PxTransform & trans);

	virtual void Update(float fElapsedTime);

	void UpdatePose(const my::Vector3 & Pos, const my::Quaternion & Rot);

	virtual my::AABB CalculateAABB(void) const;

	void UpdateAABB(void);

	void UpdateWorld(void);

	virtual void OnWorldChanged(void);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos);

	void ClearRigidActor(void);

	void CreateRigidActor(physx::PxActorType::Enum ActorType);

	void AddComponent(ComponentPtr cmp);

	void RemoveComponent(ComponentPtr cmp);

	void ClearAllComponent(ComponentPtr cmp);
};
