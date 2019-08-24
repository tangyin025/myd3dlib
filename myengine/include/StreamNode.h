#pragma once

#include "myOctree.h"
#include "mySingleton.h"

class Actor;

class PhysXSceneContext;

class StreamNode
	: public my::OctNode
	, public my::IResourceCallback
{
public:
	bool m_Ready;

public:
	StreamNode(void)
		: m_Ready(false)
	{
	}

	StreamNode(my::OctNode * Parent, const my::AABB & aabb)
		: OctNode(Parent, aabb)
		, m_Ready(true)
	{
	}

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctNode);
	}

	virtual void AddToChild(ChildArray::reference & child, const my::AABB & child_aabb, my::OctActorPtr actor, const my::AABB & aabb);

	virtual void RequestResource(void);

	virtual void ReleaseResource(void);

	std::string BuildPath(const char * RootPath);

	virtual void OnReady(my::IORequest * request);

	void SaveAllActor(const char * RootPath);
};

typedef boost::shared_ptr<StreamNode> StreamNodePtr;

class StreamRoot : public StreamNode
{
public:
	typedef std::set<StreamNode *> StreamNodeSet;

	StreamNodeSet m_ViewedNodes;

	typedef std::map<Actor *, boost::weak_ptr<Actor> > WeakActorMap;

	WeakActorMap m_ViewedActors;

	std::string m_Path;

protected:
	StreamRoot(void);

public:
	StreamRoot(const my::AABB & aabb);

	virtual ~StreamRoot();

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(StreamNode);
	}

	void ClearAllNode(void);

	bool CheckViewedActor(PhysXSceneContext * Scene, const my::AABB & In, const my::AABB & Out, bool IsEditor);
};
