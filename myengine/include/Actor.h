#pragma once

#include "myOctree.h"
#include "Component.h"

class Animator;

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

	boost::shared_ptr<physx::PxCollection> m_Collection;

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
	Actor* other;

	TriggerEventArg(Actor* _self, Actor* _other)
		: ActorEventArg(_self)
		, other(_other)
	{
	}
};

struct ShapeHitEventArg : public ActorEventArg
{
public:
	Component* self_cmp;
	my::Vector3 worldPos;		//!< Contact position in world space
	my::Vector3 worldNormal;	//!< Contact normal in world space
	my::Vector3 dir;			//!< Motion direction
	float length;				//!< Motion length
	Actor* other;				//!< Touched actor
	Component* other_cmp;			//!< Touched shape
	unsigned int triangleIndex;	//!< touched triangle index (only for meshes/heightfields)

	ShapeHitEventArg::ShapeHitEventArg(Actor* _self, Component* _self_cmp, Actor* _other, Component* _other_cmp)
		: ActorEventArg(_self)
		, self_cmp(_self_cmp)
		, worldPos(0, 0, 0)
		, worldNormal(1, 0, 0)
		, dir(1, 0, 0)
		, length(0)
		, other(_other)
		, other_cmp(_other_cmp)
		, triangleIndex(0)
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

	my::AABB m_aabb;

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Vector3 m_Scale;

	my::Matrix4 m_World;

	bool m_Requested;

	int m_Lod;

	float m_LodDist;

	float m_LodFactor;

	float m_CullingDist;

	typedef std::vector<ComponentPtr> ComponentPtrList;

	ComponentPtrList m_Cmps;

	boost::shared_ptr<unsigned char> m_SerializeBuff;

	boost::shared_ptr<physx::PxRigidActor> m_PxActor;

	Actor * m_Base;

	typedef std::pair<Actor *, int> AttachPair;

	typedef std::vector<AttachPair> AttachPairList;

	AttachPairList m_Attaches;

	typedef std::vector<boost::shared_ptr<ActionInst> > ActionInstPtrList;

	ActionInstPtrList m_ActionInstList;

	int m_ActionTrackPoseInstRef;

	my::EventFunction m_EventEnterTrigger;

	my::EventFunction m_EventLeaveTrigger;

	my::EventFunction m_EventShapeHit;

protected:
	Actor(void)
		: m_aabb(my::AABB::Invalid())
		, m_Position(0, 0, 0)
		, m_Rotation(my::Quaternion::Identity())
		, m_Scale(1, 1, 1)
		, m_World(my::Matrix4::Identity())
		, m_Requested(false)
		, m_Lod(Component::LOD_MAX)
		, m_LodDist(33.0f)
		, m_LodFactor(2.0f)
		, m_CullingDist(powf(m_LodDist* powf(m_LodFactor, 3), 2.0))
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
		, m_Lod(Component::LOD_MAX)
		, m_LodDist(33.0f)
		, m_LodFactor(2.0f)
		, m_CullingDist(powf(m_LodDist* powf(m_LodFactor, 3), 2.0))
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

	static boost::shared_ptr<boost::archive::polymorphic_iarchive> GetIArchive(std::istream & istr, const char* ext);

	static boost::shared_ptr<boost::archive::polymorphic_oarchive> GetOArchive(std::ostream & ostr, const char* ext);

	bool operator ==(const Actor & rhs) const
	{
		return this == &rhs;
	}

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	void CopyFrom(const Actor & rhs);

	virtual ActorPtr Clone(void) const;

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	void UpdateAttaches(float fElapsedTime);

	virtual void SetPose(const my::Vector3 & Pos, const my::Quaternion & Rot);

	virtual void SetPxPoseOrbyPxThread(const physx::PxTransform& pose);

	virtual void OnPxTransformChanged(const physx::PxTransform& trans);

	virtual my::AABB CalculateAABB(void) const;

	void UpdateAABB(void);

	void UpdateWorld(void);

	void UpdateOctNode(void);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	int CalculateLod(const my::AABB & Aabb, const my::Vector3 & ViewPos);

	void ClearRigidActor(void);

	void CreateRigidActor(physx::PxActorType::Enum ActorType);

	void SetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag, bool Value);

	bool GetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag) const;

	void AddComponent(ComponentPtr cmp);

	void RemoveComponent(ComponentPtr cmp);

	void ClearAllComponent(void);

	void Attach(Actor * other, int BoneId);

	void Detach(Actor * other);

	void ClearAllAttacher(void);

	void PlayAction(Action * action);

	void StopAllAction(void);

	Animator* GetAnimator(void);
};
