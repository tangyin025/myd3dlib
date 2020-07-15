#pragma once

#include "myOctree.h"
#include "Component.h"

class AnimationRoot;

class Actor;

typedef boost::shared_ptr<Actor> ActorPtr;

class Action;

class ActionInst;

struct ActorEventArg : public my::EventArg
{
public:
	Actor * self;

	ActorEventArg(Actor * _self)
		: self(_self)
	{
	}
};

struct TriggerEventArg : public ActorEventArg
{
public:
	Actor * other;

	TriggerEventArg(Actor * _self, Actor * _other)
		: ActorEventArg(_self)
		, other(_other)
	{
	}
};

class Actor
	: public my::NamedObject
	, public my::OctEntity
	, public boost::enable_shared_from_this<Actor>
{
public:
	static const float MinBlock;

	static const float Threshold;

	boost::shared_ptr<unsigned char> m_SerializeBuff;

	my::AABB m_aabb;

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Vector3 m_Scale;

	my::Matrix4 m_World;

	bool m_Requested;

	bool m_EnteredPhysx;

	bool m_ViewNotified;

	unsigned int m_Lod;

	float m_LodDist;

	float m_LodFactor;

	boost::shared_ptr<AnimationRoot> m_Animation;

	typedef std::vector<ComponentPtr> ComponentPtrList;

	ComponentPtrList m_Cmps;

	boost::shared_ptr<physx::PxRigidActor> m_PxActor;

	Actor * m_Base;

	typedef std::pair<Actor *, int> AttachPair;

	typedef std::vector<AttachPair> AttachPairList;

	AttachPairList m_Attaches;

	typedef std::vector<boost::shared_ptr<ActionInst> > ActionInstPtrList;

	ActionInstPtrList m_ActionInstList;

	int m_ActionTrackPoseInstRef;

	my::EventFunction m_EventEnterView;

	my::EventFunction m_EventLeaveView;

	my::EventFunction m_EventEnterTrigger;

	my::EventFunction m_EventLeaveTrigger;

protected:
	Actor(void)
		: m_aabb(my::AABB::Invalid())
		, m_Position(0, 0, 0)
		, m_Rotation(my::Quaternion::Identity())
		, m_Scale(1, 1, 1)
		, m_World(my::Matrix4::Identity())
		, m_Requested(false)
		, m_EnteredPhysx(false)
		, m_ViewNotified(false)
		, m_Lod(Component::LOD_INFINITE)
		, m_LodDist(33.0f)
		, m_LodFactor(2.0f)
		, m_Base(NULL)
		, m_ActionTrackPoseInstRef(0)
	{
	}

public:
	Actor(const char * Name, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb)
		: m_aabb(aabb)
		, NamedObject(Name)
		, m_Position(Position)
		, m_Rotation(Rotation)
		, m_Scale(Scale)
		, m_World(my::Matrix4::Identity())
		, m_Requested(false)
		, m_EnteredPhysx(false)
		, m_ViewNotified(false)
		, m_Lod(Component::LOD_INFINITE)
		, m_LodDist(33.0f)
		, m_LodFactor(2.0f)
		, m_Base(NULL)
		, m_ActionTrackPoseInstRef(0)
	{
	}

	virtual ~Actor(void);

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

	bool operator ==(const Actor & rhs) const
	{
		return this == &rhs;
	}

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	bool IsEnteredPhysx(void) const
	{
		return m_EnteredPhysx;
	}

	bool IsViewNotified(void) const
	{
		return m_ViewNotified;
	}

	void CopyFrom(const Actor & rhs);

	virtual ActorPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void EnterPhysxScene(PhysxSceneContext * scene);

	virtual void LeavePhysxScene(PhysxSceneContext * scene);

	virtual void NotifyEnterView(void);

	virtual void NotifyLeaveView(void);

	virtual void OnPxTransformChanged(const physx::PxTransform & trans);

	virtual void Update(float fElapsedTime);

	virtual void SetPose(const my::Vector3 & Pos, const my::Quaternion & Rot);

	virtual my::AABB CalculateAABB(void) const;

	void UpdateAABB(void);

	void UpdateWorld(void);

	void UpdateOctNode(void);

	virtual bool AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	Component::LODMask CalculateLod(const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void SetLod(unsigned int lod);

	void ClearRigidActor(void);

	void CreateRigidActor(physx::PxActorType::Enum ActorType);

	void SetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag, bool Value);

	bool GetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag) const;

	void AddComponent(ComponentPtr cmp);

	void RemoveComponent(ComponentPtr cmp);

	void ClearAllComponent(void);

	static ActorPtr LoadFromFile(const char * path);

	void SaveToFile(const char * path) const;

	void Attach(Actor * other, int BoneId);

	void Detach(Actor * other);

	void ClearAllAttacher(void);

	void PlayAction(Action * action);

	void StopAllAction(void);
};
