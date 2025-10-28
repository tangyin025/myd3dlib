// Copyright (c) 2011-2024 tangyin025
// License: MIT
#pragma once

#include "myOctree.h"
#include "Component.h"
#include <boost/intrusive/list_hook.hpp>

class Actor;

typedef boost::shared_ptr<Actor> ActorPtr;

class Action;

class ActionInst;

namespace boost {
	namespace archive {
		class polymorphic_iarchive;

		class polymorphic_oarchive;
	}
}

class ActorSerializationContext
{
public:
	boost::shared_ptr<physx::PxSerializationRegistry> m_Registry;

	ActorSerializationContext(void);
};

struct ActorEventArg : public my::EventArg
{
public:
	Actor* self;

	Component* self_cmp;

	ActorEventArg(Actor* _self, Component* _self_cmp)
		: self(_self)
		, self_cmp(_self_cmp)
	{
	}
};

struct TriggerEventArg : public ActorEventArg
{
public:
	Actor* other;
	Component* other_cmp;
	unsigned int events;

	TriggerEventArg(Actor* _self, Component* _self_cmp, Actor* _other, Component* _other_cmp, unsigned int _events)
		: ActorEventArg(_self, _self_cmp)
		, other(_other)
		, other_cmp(_other_cmp)
		, events(_events)
	{
	}
};

struct ContactEventArg : public TriggerEventArg
{
public:
	my::Vector3 position;
	float separation;
	my::Vector3 normal;
	my::Vector3 impulse;

	ContactEventArg(Actor* _self, Component* _self_cmp, Actor* _other, Component* _other_cmp, unsigned int _events)
		: TriggerEventArg(_self, _self_cmp, _other, _other_cmp, _events)
		, position(0, 0, 0)
		, separation(0)
		, normal(1, 0, 0)
		, impulse(0, 0, 0)
	{
	}
};

struct ControllerEventArg : public ActorEventArg
{
public:
	my::Vector3 worldPos;		//!< Contact position in world space
	my::Vector3 worldNormal;	//!< Contact normal in world space
	my::Vector3 dir;			//!< Motion direction
	float length;				//!< Motion length
	unsigned int flag;

	ControllerEventArg(Actor* _self, Component* _self_cmp)
		: ActorEventArg(_self, _self_cmp)
		, worldPos(0, 0, 0)
		, worldNormal(1, 0, 0)
		, dir(1, 0, 0)
		, length(0)
		, flag(0)
	{
	}
};

struct ShapeHitEventArg : public ControllerEventArg
{
public:
	Actor* other;				//!< Touched actor
	Component* other_cmp;		//!< Touched shape
	unsigned int triangleIndex;	//!< touched triangle index (only for meshes/heightfields)

	ShapeHitEventArg(Actor* _self, Component* _self_cmp, Actor* _other, Component* _other_cmp)
		: ControllerEventArg(_self, _self_cmp)
		, other(_other)
		, other_cmp(_other_cmp)
		, triangleIndex(0)
	{
	}
};

struct ControllerHitEventArg : public ControllerEventArg
{
public:
	Actor* other;				//!< Touched actor
	Component* other_cmp;		//!< Touched shape

	ControllerHitEventArg(Actor* _self, Component* _self_cmp, Actor* _other, Component* _other_cmp)
		: ControllerEventArg(_self, _self_cmp)
		, other(_other)
		, other_cmp(_other_cmp)
	{
	}
};

struct ObstacleHitEventArg : public ControllerEventArg
{
public:
	ObstacleHitEventArg(Actor* _self, Component* _self_cmp)
		: ControllerEventArg(_self, _self_cmp)
	{
	}
};

class AnimationNodeSequence;

struct AnimationEventArg : public ActorEventArg
{
public:
	AnimationNodeSequence * seq;
	unsigned int id;

	AnimationEventArg(Actor* _self, Component* _self_cmp, AnimationNodeSequence* _seq, unsigned int _id)
		: ActorEventArg(_self, _self_cmp)
		, seq(_seq)
		, id(_id)
	{
	}
};

struct ViewedActorTag;

class Actor
	: public my::NamedObject
	, public my::OctEntity
	, public boost::enable_shared_from_this<Actor>
	, public boost::intrusive::list_base_hook<boost::intrusive::tag<ViewedActorTag> >
{
public:
	static const int MaxLod;

	static float MinBlock;

	static float Threshold;

	my::AABB m_aabb;

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Vector3 m_Scale;

	my::Matrix4 m_World;

	bool m_Requested;

	int m_Lod;

	float m_LodDist;

	float m_LodFactor;

	float m_CullingDistSq;

	typedef std::vector<ComponentPtr> ComponentPtrList;

	ComponentPtrList m_Cmps;

	DWORD m_SignatureFlags;

	enum {
		SignatureFlagUpdate = 1 << 0
	};

	boost::shared_ptr<physx::PxRigidActor> m_PxActor;

	Actor * m_Base;

	int m_BaseBoneId;

	typedef std::vector<Actor *> AttachList;

	AttachList m_Attaches;

	typedef std::vector<boost::shared_ptr<physx::PxJoint> > JointPtrList;

	JointPtrList m_Joints;

	boost::shared_ptr<physx::PxAggregate> m_Aggregate;

	typedef std::vector<boost::shared_ptr<ActionInst> > ActionInstPtrList;

	ActionInstPtrList m_ActionInstList;

	my::EventSignal m_EventOnTrigger;

	my::EventSignal m_EventOnContact;

	my::EventSignal m_EventPxThreadShapeHit;

	my::EventSignal m_EventPxThreadControllerHit;

	my::EventSignal m_EventPxThreadObstacleHit;

	my::EventSignal m_EventAnimation;

protected:
	Actor(void)
		: m_aabb(my::AABB::Invalid())
		, m_Position(0, 0, 0)
		, m_Rotation(my::Quaternion::Identity())
		, m_Scale(1, 1, 1)
		, m_World(my::Matrix4::Identity())
		, m_Requested(false)
		, m_Lod(MaxLod)
		, m_LodDist(33.0f)
		, m_LodFactor(2.0f)
		, m_CullingDistSq(69696.0f)
		, m_SignatureFlags(0)
		, m_Base(NULL)
		, m_BaseBoneId(-1)
	{
	}

public:
	Actor(const char * Name, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb)
		: m_aabb(aabb)
		, NamedObject(Name)
		, m_Position(Position)
		, m_Rotation(Rotation)
		, m_Scale(Scale)
		, m_World(my::Matrix4::Compose(Scale, Rotation, Position))
		, m_Requested(false)
		, m_Lod(MaxLod)
		, m_LodDist(33.0f)
		, m_LodFactor(2.0f)
		, m_CullingDistSq(69696.0f)
		, m_SignatureFlags(0)
		, m_Base(NULL)
		, m_BaseBoneId(-1)
	{
	}

	virtual ~Actor(void);

	bool operator ==(const Actor & rhs) const
	{
		return this == &rhs;
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

	static boost::shared_ptr<boost::archive::polymorphic_iarchive> GetIArchive(std::istream & istr, const char * ext);

	static boost::shared_ptr<boost::archive::polymorphic_oarchive> GetOArchive(std::ostream & ostr, const char * ext);

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	virtual ActorPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	void SetPose(const my::Vector3 & Pos);

	void SetPose(const my::Vector3 & Pos, const my::Quaternion & Rot);

	void SetPose(const my::Bone & Pose);

	void SetPxPoseOrbyPxThread(const my::Vector3 & Pos, const my::Quaternion & Rot, const Component * Exclusion);

	virtual my::AABB CalculateAABB(void) const;

	void UpdateAABB(void);

	void UpdateWorld(void);

	void UpdateOctNode(void);

	int CalculateLod(float dist) const;
	
	void SetLod(int Lod);

	int GetLod(void) const
	{
		return m_Lod;
	}

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void ClearRigidActor(void);

	void CreateRigidActor(physx::PxActorType::Enum ActorType);

	physx::PxActorType::Enum GetRigidActorType(void) const;

	void SetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag, bool Value);

	bool GetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag) const;

	void SetMass(float mass);

	float GetMass(void) const;

	void SetCMassLocalPose(const my::Bone & pose);

	my::Bone GetCMassLocalPose(void) const;

	// ! ref: my::RigidBody::setInertialTensor, getInertialTensor
	void SetMassSpaceInertiaTensor(const my::Vector3 & m);

	my::Vector3 GetMassSpaceInertiaTensor(void);

	void UpdateMassAndInertia(float density);

	void SetLinearVelocity(const my::Vector3& LinearVelocity);

	my::Vector3 GetLinearVelocity(void) const;

	void SetAngularVelocity(const my::Vector3 & angVel);

	my::Vector3 GetAngularVelocity(void) const;

	void AddForce(const my::Vector3& force, physx::PxForceMode::Enum mode, bool autowake);

	bool IsSleeping(void) const;

	void WakeUp(void);

	void InsertComponent(ComponentPtr cmp);

	void InsertComponent(unsigned int i, ComponentPtr cmp);

	void RemoveComponent(unsigned int i);

	unsigned int GetComponentNum(void) const;

	void ClearAllComponent(void);

	void Attach(Actor * other, int BoneId);

	void Detach(Actor * other);

	unsigned int GetAttachNum(void) const;

	my::Bone GetAttachPose(int BoneId, const my::Vector3 & LocalPosition, const my::Quaternion & LocalRotation) const;

	void ClearAllAttach(void);

	physx::PxRevoluteJoint * AddRevoluteJoint(Actor * actor0, const my::Bone & localFrame0, Actor * actor1, const my::Bone & localFrame1);

	physx::PxD6Joint * AddD6Joint(Actor * actor0, const my::Bone & localFrame0, Actor * actor1, const my::Bone & localFrame1);

	void CreateAggregate(bool enableSelfCollision);

	boost::shared_ptr<ActionInst> PlayAction(Action * action);

	ActionInstPtrList::iterator StopActionInstIter(ActionInstPtrList::iterator action_inst_iter);

	void StopActionInst(boost::shared_ptr<ActionInst> action_inst);

	void StopAllActionInst(void);

	Component * GetFirstComponent(DWORD Type, unsigned int startid);

	const Component * GetFirstComponent(DWORD Type, unsigned int startid) const;

	template <typename ComponentType>
	ComponentType * GetFirstComponent(void)
	{
		return dynamic_cast<ComponentType*>(GetFirstComponent(ComponentType::TypeID, 0));
	}

	template <typename ComponentType>
	const ComponentType * GetFirstComponent(void) const
	{
		return dynamic_cast<const ComponentType*>(GetFirstComponent(ComponentType::TypeID, 0));
	}
};
