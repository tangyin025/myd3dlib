#pragma once

#include "myOctree.h"
#include "Component.h"

class Actor
	: public my::OctActor
{
public:
	my::AABB m_aabb;

	my::Vector3 m_Position;

	my::Quaternion m_Rotation;

	my::Vector3 m_Scale;

	my::Matrix4 m_World;

	typedef std::vector<ComponentPtr> ComponentPtrList;

	ComponentPtrList m_Cmps;

	bool m_Requested;

public:
	Actor(const my::AABB & aabb, const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale)
		: m_aabb(aabb)
		, m_Position(Position)
		, m_Rotation(Rotation)
		, m_Scale(Scale)
		, m_World(my::Matrix4::Identity())
	{
	}

	Actor(void)
		: m_aabb(-1,1)
		, m_Position(0,0,0)
		, m_Rotation(my::Quaternion::Identity())
		, m_Scale(1,1,1)
		, m_World(my::Matrix4::Identity())
	{
	}

	virtual ~Actor(void)
	{
		//if (m_OctNode)
		//{
		//	m_OctNode->RemoveActor(this);
		//}
		// ! Derived class must ReleaseResource menually
		_ASSERT(!IsRequested());
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctActor);
		ar & BOOST_SERIALIZATION_NVP(m_aabb);
		ar & BOOST_SERIALIZATION_NVP(m_Position);
		ar & BOOST_SERIALIZATION_NVP(m_Rotation);
		ar & BOOST_SERIALIZATION_NVP(m_Scale);
		ar & BOOST_SERIALIZATION_NVP(m_Cmps);
	}

	bool IsRequested(void) const
	{
		return m_Requested;
	}

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	virtual void QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	virtual void UpdateLod(const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos);
};

typedef boost::shared_ptr<Actor> ActorPtr;
