#pragma once

#include "myOctree.h"
#include "Component.h"

class Actor
	: public my::OctActor
{
public:
	my::AABB m_aabb;

	my::Matrix4 m_Local;

	my::Matrix4 m_World;

	typedef std::vector<ComponentPtr> ComponentPtrList;

	ComponentPtrList m_Cmps;

	bool m_Requested;

public:
	Actor(const my::AABB & aabb)
		: m_aabb(aabb)
	{
	}

	Actor(void)
		: m_aabb(my::AABB::Invalid())
	{
	}

	virtual ~Actor(void)
	{
		//if (m_OctNode)
		//{
		//	m_OctNode->RemoveComponent(this);
		//}
		// ! Derived class must ReleaseResource menually
		_ASSERT(!IsRequested());
	}

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctActor);
		ar & BOOST_SERIALIZATION_NVP(m_aabb);
		ar & BOOST_SERIALIZATION_NVP(m_Local);
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
