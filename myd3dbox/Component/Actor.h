#pragma once

#include "myOctree.h"
#include "Component.h"

class Actor
	: public Component
{
public:
	my::AABB m_aabb;

	bool m_Requested;

public:
	Actor(const my::Vector3 & Position, const my::Quaternion & Rotation, const my::Vector3 & Scale, const my::AABB & aabb)
		: Component(ComponentTypeActor, Position, Rotation, Scale)
		, m_aabb(aabb)
		, m_Requested(false)
	{
	}

	Actor(void)
		: Component(ComponentTypeActor, my::Vector3(0,0,0), my::Quaternion::Identity(), my::Vector3(1,1,1))
		, m_aabb(-1,1)
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

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	virtual void Update(float fElapsedTime);

	void UpdateAABB(void);

	virtual void AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask);

	virtual void UpdateLod(const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos);
};

typedef boost::shared_ptr<Actor> ActorPtr;
