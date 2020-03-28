#pragma once

#include "myOctree.h"
#include "Component.h"

class Animator;

class Controller;

class Actor;

typedef boost::shared_ptr<Actor> ActorPtr;

class Attacher;

typedef boost::shared_ptr<Attacher> AttacherPtr;

class Actor
	: public my::OctEntity
{
public:
	boost::shared_ptr<unsigned char> m_SerializeBuff;

	my::AABB m_aabb;

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Vector3 m_Scale;

	my::Matrix4 m_World;

	bool m_Requested;

	Component::LODMask m_Lod;

	float m_LodDist;

	float m_LodFactor;

	boost::shared_ptr<Animator> m_Animator;

	boost::shared_ptr<Controller> m_Controller;

	typedef std::vector<ComponentPtr> ComponentPtrList;

	ComponentPtrList m_Cmps;

	boost::shared_ptr<physx::PxRigidActor> m_PxActor;

	Attacher * m_Base;

	typedef std::set<AttacherPtr> AttacherPtrList;

	AttacherPtrList m_Attaches;

protected:
	Actor(void)
		: m_aabb(my::AABB::Invalid())
		, m_Position(0, 0, 0)
		, m_Rotation(my::Quaternion::Identity())
		, m_Scale(1, 1, 1)
		, m_World(my::Matrix4::Identity())
		, m_Requested(false)
		, m_Lod(Component::LOD0)
		, m_LodDist(33.0f)
		, m_LodFactor(2.0f)
		, m_Base(NULL)
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
		, m_Lod(Component::LOD0)
		, m_LodDist(33.0f)
		, m_LodFactor(2.0f)
		, m_Base(NULL)
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

	virtual void OnPxTransformChanged(const physx::PxTransform & trans);

	virtual void OnShaderChanged(void);

	virtual void Update(float fElapsedTime);

	void UpdatePose(const my::Vector3 & Pos, const my::Quaternion & Rot);

	virtual my::AABB CalculateAABB(void) const;

	void UpdateAABB(void);

	void UpdateWorld(void);

	void OnWorldUpdated(void);

	void UpdateOctNode(void);

	//void UpdatePxTransform(void);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void UpdateLod(const my::Vector3 & ViewPos, const my::Vector3 & TargetPos);

	void ClearRigidActor(void);

	void CreateRigidActor(physx::PxActorType::Enum ActorType);

	void SetRigidBodyFlag(physx::PxRigidBodyFlag::Enum Flag, bool Value);

	void AddComponent(ComponentPtr cmp);

	void RemoveComponent(ComponentPtr cmp);

	void ClearAllComponent(ComponentPtr cmp);

	static ActorPtr LoadFromFile(const char * path);

	void SaveToFile(const char * path) const;

	void Attach(ActorPtr other, int BoneId);

	void Dettach(ActorPtr other);
};

class Attacher : public boost::enable_shared_from_this<Attacher>
{
public:
	Actor * m_Owner;

	Actor * m_Suber;

protected:
	Attacher(Actor * Owner, Actor * Suber)
		: m_Owner(Owner)
		, m_Suber(Suber)
	{
		if (m_Suber)
		{
			_ASSERT(m_Suber->m_Base == NULL);
			m_Suber->m_Base = this;
		}
	}

public:
	virtual ~Attacher(void)
	{
		if (m_Suber)
		{
			_ASSERT(m_Suber->m_Base == this);
			m_Suber->m_Base = NULL;
		}
	}

	virtual void Update(float fElapsedTime)
	{
		if (m_Suber)
		{
			m_Suber->Update(fElapsedTime);
		}
	}
};

class BoneAttacher : public Attacher
{
protected:
	int m_BoneId;

public:
	BoneAttacher(Actor * Owner, Actor * Suber, int BoneId)
		: Attacher(Owner, Suber)
		, m_BoneId(BoneId)
	{
	}

	virtual void Update(float fElapsedTime);
};
