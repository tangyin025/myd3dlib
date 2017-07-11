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

BOOST_CLASS_EXPORT(OctNodeBase)

BOOST_CLASS_EXPORT_GUID(OctNode<0>, "OctNode0")

BOOST_CLASS_EXPORT_GUID(OctNode<1>, "OctNode1")

BOOST_CLASS_EXPORT_GUID(OctNode<2>, "OctNode2")

template<>
void OctNodeBase::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_aabb);
	ar << BOOST_SERIALIZATION_NVP(m_Actors);
	ar << BOOST_SERIALIZATION_NVP(m_Childs);
}

template<>
void OctNodeBase::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
	ar >> BOOST_SERIALIZATION_NVP(m_Actors);
	ar >> BOOST_SERIALIZATION_NVP(m_Childs);
	OctActorMap::iterator cmp_iter = m_Actors.begin();
	for (; cmp_iter != m_Actors.end(); cmp_iter++)
	{
		cmp_iter->first->m_Node = this;
	}
	for (unsigned int i = 0; i < ChildArray::static_size; i++)
	{
		if (m_Childs[i])
		{
			m_Childs[i]->m_Parent = this;
		}
	}
}

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

OctNodeBase * OctNodeBase::GetTopNode(void)
{
	if (!m_Parent)
	{
		return this;
	}
	return m_Parent->GetTopNode();
}

void OctNodeBase::QueryActor(const Ray & ray, QueryCallback * callback)
{
	if (IntersectionTests::rayAndAABB(ray.p, ray.d, m_aabb).first)
	{
		OctActorMap::iterator cmp_iter = m_Actors.begin();
		for(; cmp_iter != m_Actors.end(); cmp_iter++)
		{
			if (IntersectionTests::rayAndAABB(ray.p, ray.d, cmp_iter->second).first)
			{
				(*callback)(cmp_iter->first.get(), IntersectionTests::IntersectionTypeRay);
			}
		}

		ChildArray::iterator node_iter = m_Childs.begin();
		for(; node_iter != m_Childs.end(); node_iter++)
		{
			if (*node_iter)
			{
				(*node_iter)->QueryActor(ray, callback);
			}
		}
	}
}

void OctNodeBase::QueryActor(const AABB & aabb, QueryCallback * callback)
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

void OctNodeBase::QueryActor(const Frustum & frustum, QueryCallback * callback)
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

void OctNodeBase::QueryActorAll(QueryCallback * callback)
{
	OctActorMap::iterator cmp_iter = m_Actors.begin();
	for(; cmp_iter != m_Actors.end(); cmp_iter++)
	{
		(*callback)(cmp_iter->first.get(), IntersectionTests::IntersectionTypeInside);
	}

	ChildArray::iterator node_iter = m_Childs.begin();
	for(; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			(*node_iter)->QueryActorAll(callback);
		}
	}
}

void OctNodeBase::QueryActorIntersected(const AABB & aabb, QueryCallback * callback)
{
	OctActorMap::iterator cmp_iter = m_Actors.begin();
	for(; cmp_iter != m_Actors.end(); cmp_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndAABB(cmp_iter->second, aabb);
		switch(intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(cmp_iter->first.get(), intersect_type);
			break;
		}
	}

	ChildArray::iterator node_iter = m_Childs.begin();
	for(; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			(*node_iter)->QueryActor(aabb, callback);
		}
	}
}

void OctNodeBase::QueryActorIntersected(const Frustum & frustum, QueryCallback * callback)
{
	OctActorMap::iterator cmp_iter = m_Actors.begin();
	for(; cmp_iter != m_Actors.end(); cmp_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndFrustum(cmp_iter->second, frustum);
		switch(intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(cmp_iter->first.get(), intersect_type);
			break;
		}
	}

	ChildArray::iterator node_iter = m_Childs.begin();
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
		OctActorMap::iterator cmp_iter = actor->m_Node->m_Actors.find(actor);
		if (cmp_iter != actor->m_Node->m_Actors.end())
		{
			actor->m_Node->m_Actors.erase(cmp_iter);
			actor->m_Node = NULL;
			return true;
		}
	}
	return false;
}

void OctNodeBase::ClearAllActor(void)
{
	OctActorMap::iterator cmp_iter = m_Actors.begin();
	for (; cmp_iter != m_Actors.end(); cmp_iter++)
	{
		_ASSERT(cmp_iter->first->m_Node == this);
		cmp_iter->first->m_Node = NULL;
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
