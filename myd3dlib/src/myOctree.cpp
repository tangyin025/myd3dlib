#include "StdAfx.h"
#include "myOctree.h"
#pragma warning(disable:4308)
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/algorithm/cxx11/none_of.hpp>
#include <boost/lambda/lambda.hpp>

using namespace my;

BOOST_CLASS_EXPORT(OctActor)

bool OctNodeBase::HaveNode(const OctNodeBase * node) const
{
	if (this == node)
	{
		return true;
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for(; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter && (*node_iter)->HaveNode(node))
		{
			return true;
		}
	}
	return false;
}

const OctNodeBase * OctNodeBase::GetTopNode(void) const
{
	if (!m_Parent)
	{
		return this;
	}
	return m_Parent->GetTopNode();
}

OctNodeBase * OctNodeBase::GetTopNode(void)
{
	if (!m_Parent)
	{
		return this;
	}
	return m_Parent->GetTopNode();
}

void OctNodeBase::QueryActor(const Ray & ray, QueryCallback * callback) const
{
	if (IntersectionTests::rayAndAABB(ray.p, ray.d, m_aabb).first)
	{
		OctActorMap::const_iterator actor_iter = m_Actors.begin();
		for(; actor_iter != m_Actors.end(); actor_iter++)
		{
			if (IntersectionTests::rayAndAABB(ray.p, ray.d, actor_iter->second).first)
			{
				(*callback)(actor_iter->first.get(), actor_iter->second, IntersectionTests::IntersectionTypeRay);
			}
		}

		ChildArray::const_iterator node_iter = m_Childs.begin();
		for(; node_iter != m_Childs.end(); node_iter++)
		{
			if (*node_iter)
			{
				(*node_iter)->QueryActor(ray, callback);
			}
		}
	}
}

void OctNodeBase::QueryActor(const AABB & aabb, QueryCallback * callback) const
{
	switch (IntersectionTests::IntersectAABBAndAABB(m_aabb, aabb))
	{
	case IntersectionTests::IntersectionTypeInside:
		QueryActorAll(callback);
		break;

	case IntersectionTests::IntersectionTypeIntersect:
		QueryActorIntersected(aabb, callback);
		break;
	}
}

void OctNodeBase::QueryActor(const Frustum & frustum, QueryCallback * callback) const
{
	switch(IntersectionTests::IntersectAABBAndFrustum(m_aabb, frustum))
	{
	case IntersectionTests::IntersectionTypeInside:
		QueryActorAll(callback);
		break;

	case IntersectionTests::IntersectionTypeIntersect:
		QueryActorIntersected(frustum, callback);
		break;
	}
}

void OctNodeBase::QueryActorAll(QueryCallback * callback) const
{
	OctActorMap::const_iterator actor_iter = m_Actors.begin();
	for(; actor_iter != m_Actors.end(); actor_iter++)
	{
		(*callback)(actor_iter->first.get(), actor_iter->second, IntersectionTests::IntersectionTypeInside);
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for(; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			(*node_iter)->QueryActorAll(callback);
		}
	}
}

void OctNodeBase::QueryActorIntersected(const AABB & aabb, QueryCallback * callback) const
{
	OctActorMap::const_iterator actor_iter = m_Actors.begin();
	for(; actor_iter != m_Actors.end(); actor_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndAABB(actor_iter->second, aabb);
		switch(intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(actor_iter->first.get(), actor_iter->second, intersect_type);
			break;
		}
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for(; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			(*node_iter)->QueryActor(aabb, callback);
		}
	}
}

void OctNodeBase::QueryActorIntersected(const Frustum & frustum, QueryCallback * callback) const
{
	OctActorMap::const_iterator actor_iter = m_Actors.begin();
	for(; actor_iter != m_Actors.end(); actor_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndFrustum(actor_iter->second, frustum);
		switch(intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(actor_iter->first.get(), actor_iter->second, intersect_type);
			break;
		}
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for(; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			(*node_iter)->QueryActor(frustum, callback);
		}
	}
}

bool OctNodeBase::RemoveActor(OctActorPtr actor)
{
	if (actor->m_Node)
	{
		_ASSERT(HaveNode(actor->m_Node));
		OctActorMap::iterator actor_iter = actor->m_Node->m_Actors.find(actor);
		if (actor_iter != actor->m_Node->m_Actors.end())
		{
			actor->m_Node->m_Actors.erase(actor_iter);
			actor->m_Node = NULL;
			return true;
		}
	}
	return false;
}

void OctNodeBase::ClearAllActor(void)
{
	OctActorMap::iterator actor_iter = m_Actors.begin();
	for (; actor_iter != m_Actors.end(); actor_iter++)
	{
		_ASSERT(actor_iter->first->m_Node == this);
		actor_iter->first->m_Node = NULL;
	}
	m_Actors.clear();

	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			m_Childs[i]->ClearAllActor();
			m_Childs[i].reset();
		}
	}
}

void OctNodeBase::Flush(void)
{
	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			m_Childs[i]->Flush();
			if (m_Childs[i]->m_Actors.empty()
				&& boost::algorithm::none_of(m_Childs[i]->m_Childs, boost::lambda::_1))
			{
				m_Childs[i].reset();
			}
		}
	}
}

template<>
void OctRoot::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	class Callback: public QueryCallback
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
	ar << boost::serialization::make_nvp("OctNode_0", boost::serialization::base_object< my::OctNode<0> >(*this));
	ar << BOOST_SERIALIZATION_NVP(cb.actor_list);
}

template<>
void OctRoot::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	class Callback
	{
	public:
		std::vector<std::pair<OctActorPtr, AABB> > actor_list;
	};

	Callback cb;
	ar >> boost::serialization::make_nvp("OctNode_0", boost::serialization::base_object< my::OctNode<0> >(*this));
	ar >> BOOST_SERIALIZATION_NVP(cb.actor_list);
	std::vector<std::pair<OctActorPtr, AABB> >::iterator actor_iter = cb.actor_list.begin();
	for (; actor_iter != cb.actor_list.end(); actor_iter++)
	{
		AddActor(actor_iter->first, actor_iter->second, 0.1f);
	}
}
