#pragma once

#include "myOctree.h"
#include "Component.h"
#include <deque>
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

	ActorEventArg(Actor* _self)
		: self(_self)
	{
	}
};

struct TriggerEventArg : public ActorEventArg
{
public:
	Component* self_cmp;

	Actor* other;

	Component* other_cmp;

	TriggerEventArg(Actor* _self, Component* _self_cmp, Actor* _other, Component* _other_cmp)
		: ActorEventArg(_self)
		, self_cmp(_self_cmp)
		, other(_other)
		, other_cmp(_other_cmp)
	{
	}
};

struct ControllerEventArg : public ActorEventArg
{
public:
	Component* self_cmp;
	my::Vector3 worldPos;		//!< Contact position in world space
	my::Vector3 worldNormal;	//!< Contact normal in world space
	my::Vector3 dir;			//!< Motion direction
	float length;				//!< Motion length

	ControllerEventArg::ControllerEventArg(Actor* _self, Component* _self_cmp)
		: ActorEventArg(_self)
		, self_cmp(_self_cmp)
		, worldPos(0, 0, 0)
		, worldNormal(1, 0, 0)
		, dir(1, 0, 0)
		, length(0)
	{
	}
};

struct ShapeHitEventArg : public ControllerEventArg
{
public:
	Actor* other;				//!< Touched actor
	Component* other_cmp;		//!< Touched shape
	unsigned int triangleIndex;	//!< touched triangle index (only for meshes/heightfields)

	ShapeHitEventArg::ShapeHitEventArg(Actor* _self, Component* _self_cmp, Actor* _other, Component* _other_cmp)
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

	ControllerHitEventArg::ControllerHitEventArg(Actor* _self, Component* _self_cmp, Actor* _other, Component* _other_cmp)
		: ControllerEventArg(_self, _self_cmp)
		, other(_other)
		, other_cmp(_other_cmp)
	{
	}
};

struct ObstacleHitEventArg : public ControllerEventArg
{
public:
	ObstacleHitEventArg::ObstacleHitEventArg(Actor* _self, Component* _self_cmp)
		: ControllerEventArg(_self, _self_cmp)
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
	static const float MinBlock;

	static const float Threshold;

	my::AABB m_aabb;

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Vector3 m_Scale;

	my::Matrix4 m_World;

	bool m_Requested;

	int m_Lod;

	float m_LodDist;

	float m_LodFactor;

	typedef std::vector<ComponentPtr> ComponentPtrList;

	ComponentPtrList m_Cmps;

	boost::shared_ptr<unsigned char> m_SerializeBuff;

	boost::shared_ptr<physx::PxRigidActor> m_PxActor;

	Actor * m_Base;

	int m_BaseBoneId;

	typedef std::set<Actor *> ActorList;

	ActorList m_Attaches;

	typedef std::vector<std::pair<boost::shared_ptr<ActionInst>, float> > ActionInstPtrList;

	ActionInstPtrList m_ActionInstList;

	int m_ActionTrackPoseInstRef;

	my::EventSignal m_EventEnterTrigger;

	my::EventSignal m_EventLeaveTrigger;

	my::EventSignal m_EventPxThreadShapeHit;

	my::EventSignal m_EventPxThreadControllerHit;

	my::EventSignal m_EventPxThreadObstacleHit;

protected:
	Actor(void)
		: m_aabb(my::AABB::Invalid())
		, m_Position(0, 0, 0)
		, m_Rotation(my::Quaternion::Identity())
		, m_Scale(1, 1, 1)
		, m_World(my::Matrix4::Identity())
		, m_Requested(false)
		, m_Lod(Component::LOD_INFINITE)
		, m_LodDist(33.0f)
		, m_LodFactor(2.0f)
		, m_Base(NULL)
		, m_BaseBoneId(-1)
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
		, m_Lod(Component::LOD_INFINITE)
		, m_LodDist(33.0f)
		, m_LodFactor(2.0f)
		, m_Base(NULL)
		, m_BaseBoneId(-1)
		, m_ActionTrackPoseInstRef(0)
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

	void SetPose(const my::Vector3 & Pos, const my::Quaternion & Rot, const my::Vector3 & Scale);

	void SetPxPoseOrbyPxThread(const my::Vector3 & Pos);

	void SetPxPoseOrbyPxThread(const my::Vector3 & Pos, const my::Quaternion & Rot, const Component * Exclusion);

	virtual my::AABB CalculateAABB(void) const;

	void UpdateAABB(void);

	void UpdateWorld(void);

	void UpdateOctNode(void);

	int CalculateLod(float DistanceSq, float Scale) const;

	int CalculateLod(const my::Vector3 & Center, const my::Vector3 & ViewPos, float Scale) const;

	int CalculateLod2D(const my::Vector3 & Center, const my::Vector3 & ViewPos, float Scale) const;
	
	void UpdateLod(const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void ClearRigidActor(void);

	void CreateRigidActor(physx::PxActorType::Enum ActorType);

	void SetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag, bool Value);

	bool GetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag) const;

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

	void PlayAction(Action * action, float Length);

	void StopAllAction(void);

	Component * GetFirstComponent(DWORD Type);

	const Component * GetFirstComponent(DWORD Type) const;

	template <typename ComponentType>
	ComponentType * GetFirstComponent(void)
	{
		return dynamic_cast<ComponentType*>(GetFirstComponent(ComponentType::TypeID));
	}

	template <typename ComponentType>
	const ComponentType * GetFirstComponent(void) const
	{
		return dynamic_cast<const ComponentType*>(GetFirstComponent(ComponentType::TypeID));
	}
};
