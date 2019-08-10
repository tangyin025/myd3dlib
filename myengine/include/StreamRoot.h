#pragma once

#include "myOctree.h"

class Actor;

class PhysXSceneContext;

class StreamNode : public my::OctNode
{
public:
	StreamNode(void)
	{
	}

	StreamNode(my::OctNode * Parent, const my::AABB & aabb)
		: OctNode(Parent, aabb)
	{
	}

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const;

	template<class Archive>
	void load(Archive & ar, const unsigned int version);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		boost::serialization::split_member(ar, *this, version);
	}

	virtual void AddToChild(ChildArray::reference & child, const my::AABB & child_aabb, my::OctActorPtr actor, const my::AABB & aabb);
};

class StreamRoot : public StreamNode
{
public:
	typedef std::map<Actor *, boost::weak_ptr<Actor> > WeakActorMap;

	WeakActorMap m_ViewedActors;

protected:
	StreamRoot(void);

public:
	StreamRoot(const my::AABB & aabb);

	virtual ~StreamRoot();

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

	void CheckViewedActor(PhysXSceneContext * scene, const my::AABB & In, const my::AABB & Out);
};
