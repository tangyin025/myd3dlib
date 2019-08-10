#include "StreamRoot.h"
#include "Actor.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(StreamRoot)

StreamRoot::StreamRoot(void)
	: OctNode(NULL, AABB::Invalid())
{
}

StreamRoot::StreamRoot(const my::AABB & aabb)
	: OctNode(NULL, aabb)
{
}

StreamRoot::~StreamRoot()
{
}

template<>
void StreamRoot::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	class Callback : public QueryCallback
	{
	public:
		std::vector<std::pair<OctActorPtr, AABB> > actor_list;

		virtual void operator() (OctActor * oct_actor, const AABB & aabb, IntersectionTests::IntersectionType)
		{
			_ASSERT(oct_actor);
			actor_list.push_back(std::make_pair(oct_actor->shared_from_this(), aabb));
		}
	};

	Callback cb;
	QueryActorAll(&cb);
	ar << BOOST_SERIALIZATION_NVP(m_aabb);
	ar << BOOST_SERIALIZATION_NVP(m_Half);
	ar << BOOST_SERIALIZATION_NVP(cb.actor_list);
}

template<>
void StreamRoot::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	class Callback
	{
	public:
		std::vector<std::pair<OctActorPtr, AABB> > actor_list;
	};

	Callback cb;
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
	ar >> BOOST_SERIALIZATION_NVP(m_Half);
	ar >> BOOST_SERIALIZATION_NVP(cb.actor_list);
	std::vector<std::pair<OctActorPtr, AABB> >::iterator actor_iter = cb.actor_list.begin();
	for (; actor_iter != cb.actor_list.end(); actor_iter++)
	{
		AddActor(actor_iter->first, actor_iter->second);
	}
}

void StreamRoot::CheckViewedActor(PhysXSceneContext * scene, const my::AABB & In, const my::AABB & Out)
{
	struct Callback : public my::OctNode::QueryCallback
	{
		WeakActorMap & ViewedActors;
		PhysXSceneContext * scene;
		typedef std::vector<ActorPtr> ActorList;
		ActorList actor_list;
		Callback(WeakActorMap & _ViewedActors, PhysXSceneContext * _scene)
			: ViewedActors(_ViewedActors)
			, scene(_scene)
		{
		}
		void operator() (OctActor * oct_actor, const AABB & aabb, IntersectionTests::IntersectionType)
		{
			_ASSERT(dynamic_cast<Actor *>(oct_actor));
			Actor * actor = static_cast<Actor *>(oct_actor);
			WeakActorMap::const_iterator actor_iter = ViewedActors.find(actor);
			if (actor_iter != ViewedActors.end())
			{
				ViewedActors.erase(actor);
			}
			else if (!actor->IsRequested())
			{
				actor->RequestResource();
				actor->OnEnterPxScene(scene);
			}
			actor_list.push_back(boost::static_pointer_cast<Actor>(actor->shared_from_this()));
		}
	};

	Callback cb(m_ViewedActors, scene);
	QueryActor(In, &cb);

	WeakActorMap::iterator weak_actor_iter = m_ViewedActors.begin();
	for (; weak_actor_iter != m_ViewedActors.end(); weak_actor_iter++)
	{
		ActorPtr actor = weak_actor_iter->second.lock();
		if (actor && actor->IsRequested() && !actor->GetOctAABB().Intersect(Out).IsValid())
		{
			actor->OnLeavePxScene(scene);
			actor->ReleaseResource();
		}
	}

	m_ViewedActors.clear();
	Callback::ActorList::iterator actor_iter = cb.actor_list.begin();
	for (; actor_iter != cb.actor_list.end(); actor_iter++)
	{
		m_ViewedActors.insert(std::make_pair(actor_iter->get(), *actor_iter));
	}
}
