#include "StdAfx.h"
#include "myOctree.h"
#pragma warning(disable:4308)
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(OctComponent)

BOOST_CLASS_EXPORT(OctNodeBase)

BOOST_CLASS_EXPORT_GUID(OctNode<0>, "OctNode0")

BOOST_CLASS_EXPORT_GUID(OctNode<1>, "OctNode1")

BOOST_CLASS_EXPORT_GUID(OctNode<2>, "OctNode2")

BOOST_CLASS_EXPORT(OctTree)

template<>
void OctNodeBase::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_aabb);
	ar << BOOST_SERIALIZATION_NVP(m_Components);
	ar << BOOST_SERIALIZATION_NVP(m_Childs);
}

template<>
void OctNodeBase::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
	ar >> BOOST_SERIALIZATION_NVP(m_Components);
	ar >> BOOST_SERIALIZATION_NVP(m_Childs);
	OctComponentList::iterator cmp_iter = m_Components.begin();
	for(; cmp_iter != m_Components.end(); cmp_iter++)
	{
		(*cmp_iter)->m_OctNode = this;
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

void OctNodeBase::QueryComponent(const Ray & ray, IQueryCallback * callback)
{
	if (IntersectionTests::rayAndAABB(ray.p, ray.d, m_aabb).first)
	{
		OctComponentList::iterator cmp_iter = m_Components.begin();
		for(; cmp_iter != m_Components.end(); cmp_iter++)
		{
			if (IntersectionTests::rayAndAABB(ray.p, ray.d, (*cmp_iter)->m_aabb).first)
			{
				(*callback)(cmp_iter->get(), IntersectionTests::IntersectionTypeRay);
			}
		}

		ChildArray::iterator node_iter = m_Childs.begin();
		for(; node_iter != m_Childs.end(); node_iter++)
		{
			if (*node_iter)
			{
				(*node_iter)->QueryComponent(ray, callback);
			}
		}
	}
}

void OctNodeBase::QueryComponent(const AABB & aabb, IQueryCallback * callback)
{
	switch (IntersectionTests::IntersectAABBAndAABB(m_aabb, aabb))
	{
	case IntersectionTests::IntersectionTypeInside:
		QueryComponentAll(callback);
		break;

	case IntersectionTests::IntersectionTypeIntersect:
		QueryComponentIntersected(aabb, callback);
		break;
	}
}

void OctNodeBase::QueryComponent(const Frustum & frustum, IQueryCallback * callback)
{
	switch(IntersectionTests::IntersectAABBAndFrustum(m_aabb, frustum))
	{
	case IntersectionTests::IntersectionTypeInside:
		QueryComponentAll(callback);
		break;

	case IntersectionTests::IntersectionTypeIntersect:
		QueryComponentIntersected(frustum, callback);
		break;
	}
}

void OctNodeBase::QueryComponentAll(IQueryCallback * callback)
{
	OctComponentList::iterator cmp_iter = m_Components.begin();
	for(; cmp_iter != m_Components.end(); cmp_iter++)
	{
		(*callback)(cmp_iter->get(), IntersectionTests::IntersectionTypeInside);
	}

	ChildArray::iterator node_iter = m_Childs.begin();
	for(; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			(*node_iter)->QueryComponentAll(callback);
		}
	}
}

void OctNodeBase::QueryComponentIntersected(const AABB & aabb, IQueryCallback * callback)
{
	OctComponentList::iterator cmp_iter = m_Components.begin();
	for(; cmp_iter != m_Components.end(); cmp_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndAABB((*cmp_iter)->m_aabb, aabb);
		switch(intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(cmp_iter->get(), intersect_type);
			break;
		}
	}

	ChildArray::iterator node_iter = m_Childs.begin();
	for(; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			(*node_iter)->QueryComponent(aabb, callback);
		}
	}
}

void OctNodeBase::QueryComponentIntersected(const Frustum & frustum, IQueryCallback * callback)
{
	OctComponentList::iterator cmp_iter = m_Components.begin();
	for(; cmp_iter != m_Components.end(); cmp_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndFrustum((*cmp_iter)->m_aabb, frustum);
		switch(intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(cmp_iter->get(), intersect_type);
			break;
		}
	}

	ChildArray::iterator node_iter = m_Childs.begin();
	for(; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			(*node_iter)->QueryComponent(frustum, callback);
		}
	}
}

bool OctNodeBase::RemoveComponent(OctComponentPtr cmp)
{
	if (cmp->m_OctNode)
	{
		_ASSERT(HaveNode(cmp->m_OctNode));
		OctComponentList::iterator cmp_iter =
			std::find(cmp->m_OctNode->m_Components.begin(), cmp->m_OctNode->m_Components.end(), cmp);
		if (cmp_iter != cmp->m_OctNode->m_Components.end())
		{
			cmp->m_OctNode->m_Components.erase(cmp_iter);
			cmp->m_OctNode = NULL;
			return true;
		}
	}
	return false;
}

void OctNodeBase::ClearAllComponents(void)
{
	OctComponentList::iterator cmp_iter = m_Components.begin();
	for (; cmp_iter != m_Components.end(); cmp_iter++)
	{
		_ASSERT((*cmp_iter)->m_OctNode == this);
		(*cmp_iter)->m_OctNode = NULL;
	}
	m_Components.clear();

	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			m_Childs[i]->ClearAllComponents();
			m_Childs[i].reset();
		}
	}
}
