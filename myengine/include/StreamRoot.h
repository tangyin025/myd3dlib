#pragma once

#include "myOctree.h"

class Actor;

class PhysXSceneContext;

class StreamRoot : public my::OctNode
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
