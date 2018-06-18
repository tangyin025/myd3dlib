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

const float OctNode::THRESHOLD = 0.1f;

const float OctNode::MIN_BLOCK = 1.0f;

bool OctNode::HaveNode(const OctNode * node) const
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

BOOST_CLASS_EXPORT(OctNode)

const OctNode * OctNode::GetTopNode(void) const
{
	if (!m_Parent)
	{
		return this;
	}
	return m_Parent->GetTopNode();
}

void OctNode::AddActor(OctActorPtr actor, const AABB & aabb)
{
	_ASSERT(!actor->m_Node);
	if (m_aabb.m_max.x - m_aabb.m_min.x > MIN_BLOCK + THRESHOLD || m_aabb.m_max.y - m_aabb.m_min.y > MIN_BLOCK + THRESHOLD || m_aabb.m_max.z - m_aabb.m_min.z > MIN_BLOCK + THRESHOLD)
	{
		if (aabb.m_min.x > m_Half.x - THRESHOLD && aabb.m_max.x < m_aabb.m_max.x + THRESHOLD)
		{
			if (aabb.m_min.y > m_Half.y - THRESHOLD && aabb.m_max.y < m_aabb.m_max.y + THRESHOLD)
			{
				if (aabb.m_min.z > m_Half.z - THRESHOLD && aabb.m_max.z < m_aabb.m_max.z + THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantPxPyPz], m_aabb.Slice<AABB::QuadrantPxPyPz>(m_Half), actor, aabb);
					return;
				}
				else if (aabb.m_max.z < m_Half.z + THRESHOLD && aabb.m_min.z > m_aabb.m_min.z - THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantPxPyNz], m_aabb.Slice<AABB::QuadrantPxPyNz>(m_Half), actor, aabb);
					return;
				}
			}
			else if (aabb.m_max.y < m_Half.y + THRESHOLD && aabb.m_min.y > m_aabb.m_min.y - THRESHOLD)
			{
				if (aabb.m_min.z > m_Half.z - THRESHOLD && aabb.m_max.z < m_aabb.m_max.z + THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantPxNyPz], m_aabb.Slice<AABB::QuadrantPxNyPz>(m_Half), actor, aabb);
					return;
				}
				else if (aabb.m_max.z < m_Half.z + THRESHOLD && aabb.m_min.z > m_aabb.m_min.z - THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantPxNyNz], m_aabb.Slice<AABB::QuadrantPxNyNz>(m_Half), actor, aabb);
					return;
				}
			}
		}
		else if (aabb.m_max.x < m_Half.x + THRESHOLD && aabb.m_min.x > m_aabb.m_min.x - THRESHOLD)
		{
			if (aabb.m_min.y > m_Half.y - THRESHOLD && aabb.m_max.y < m_aabb.m_max.y + THRESHOLD)
			{
				if (aabb.m_min.z > m_Half.z - THRESHOLD && aabb.m_max.z < m_aabb.m_max.z + THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantNxPyPz], m_aabb.Slice<AABB::QuadrantNxPyPz>(m_Half), actor, aabb);
					return;
				}
				else if (aabb.m_max.z < m_Half.z + THRESHOLD && aabb.m_min.z > m_aabb.m_min.z - THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantNxPyNz], m_aabb.Slice<AABB::QuadrantNxPyNz>(m_Half), actor, aabb);
					return;
				}
			}
			else if (aabb.m_max.y < m_Half.y + THRESHOLD && aabb.m_min.y > m_aabb.m_min.y - THRESHOLD)
			{
				if (aabb.m_min.z > m_Half.z - THRESHOLD && aabb.m_max.z < m_aabb.m_max.z + THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantNxNyPz], m_aabb.Slice<AABB::QuadrantNxNyPz>(m_Half), actor, aabb);
					return;
				}
				else if (aabb.m_max.z < m_Half.z + THRESHOLD && aabb.m_min.z > m_aabb.m_min.z - THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantNxNyNz], m_aabb.Slice<AABB::QuadrantNxNyNz>(m_Half), actor, aabb);
					return;
				}
			}
		}
	}
	m_Actors.insert(std::make_pair(actor, aabb));
	actor->m_Node = this;
}

void OctNode::AddToChild(ChildArray::reference & child, const AABB & child_aabb, OctActorPtr actor, const AABB & aabb)
{
	if (!child)
	{
		child.reset(new OctNode(this, child_aabb));
	}
	child->AddActor(actor, aabb);
}

OctNode * OctNode::GetTopNode(void)
{
	if (!m_Parent)
	{
		return this;
	}
	return m_Parent->GetTopNode();
}

void OctNode::QueryActor(const Ray & ray, QueryCallback * callback) const
{
	OctActorMap::const_iterator actor_iter = m_Actors.begin();
	for (; actor_iter != m_Actors.end(); actor_iter++)
	{
		if (IntersectionTests::rayAndAABB(ray.p, ray.d, actor_iter->second).first)
		{
			(*callback)(actor_iter->first.get(), actor_iter->second, IntersectionTests::IntersectionTypeRay);
		}
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for (; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			if (IntersectionTests::rayAndAABB(ray.p, ray.d, (*node_iter)->m_aabb).first)
			{
				(*node_iter)->QueryActor(ray, callback);
			}
		}
	}
}

void OctNode::QueryActor(const AABB & aabb, QueryCallback * callback) const
{
	OctActorMap::const_iterator actor_iter = m_Actors.begin();
	for (; actor_iter != m_Actors.end(); actor_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndAABB(actor_iter->second, aabb);
		switch (intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(actor_iter->first.get(), actor_iter->second, intersect_type);
			break;
		}
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for (; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			switch (IntersectionTests::IntersectAABBAndAABB((*node_iter)->m_aabb, aabb))
			{
			case IntersectionTests::IntersectionTypeInside:
				(*node_iter)->QueryActorAll(callback);
				break;

			case IntersectionTests::IntersectionTypeIntersect:
				(*node_iter)->QueryActor(aabb, callback);
				break;
			}
		}
	}
}

void OctNode::QueryActor(const Frustum & frustum, QueryCallback * callback) const
{
	OctActorMap::const_iterator actor_iter = m_Actors.begin();
	for (; actor_iter != m_Actors.end(); actor_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndFrustum(actor_iter->second, frustum);
		switch (intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(actor_iter->first.get(), actor_iter->second, intersect_type);
			break;
		}
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for (; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			switch (IntersectionTests::IntersectAABBAndFrustum((*node_iter)->m_aabb, frustum))
			{
			case IntersectionTests::IntersectionTypeInside:
				(*node_iter)->QueryActorAll(callback);
				break;

			case IntersectionTests::IntersectionTypeIntersect:
				(*node_iter)->QueryActor(frustum, callback);
				break;
			}
		}
	}
}

void OctNode::QueryActorAll(QueryCallback * callback) const
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

bool OctNode::RemoveActor(OctActorPtr actor)
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

void OctNode::ClearAllActor(void)
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

void OctNode::Flush(void)
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

BOOST_CLASS_EXPORT(OctRoot)

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
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctNode);
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
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctNode);
	ar >> BOOST_SERIALIZATION_NVP(cb.actor_list);
	std::vector<std::pair<OctActorPtr, AABB> >::iterator actor_iter = cb.actor_list.begin();
	for (; actor_iter != cb.actor_list.end(); actor_iter++)
	{
		AddActor(actor_iter->first, actor_iter->second);
	}
}
