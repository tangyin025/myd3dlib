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
#include <boost/foreach.hpp>
#include <boost/algorithm/cxx11/none_of.hpp>
#include <boost/lambda/lambda.hpp>

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
	OctComponentMap::iterator cmp_iter = m_Components.begin();
	for(; cmp_iter != m_Components.end(); cmp_iter++)
	{
		cmp_iter->first->m_OctNode = this;
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
		OctComponentMap::iterator cmp_iter = m_Components.begin();
		for(; cmp_iter != m_Components.end(); cmp_iter++)
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
	OctComponentMap::iterator cmp_iter = m_Components.begin();
	for(; cmp_iter != m_Components.end(); cmp_iter++)
	{
		(*callback)(cmp_iter->first.get(), IntersectionTests::IntersectionTypeInside);
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
	OctComponentMap::iterator cmp_iter = m_Components.begin();
	for(; cmp_iter != m_Components.end(); cmp_iter++)
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
			(*node_iter)->QueryComponent(aabb, callback);
		}
	}
}

void OctNodeBase::QueryComponentIntersected(const Frustum & frustum, IQueryCallback * callback)
{
	OctComponentMap::iterator cmp_iter = m_Components.begin();
	for(; cmp_iter != m_Components.end(); cmp_iter++)
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
			(*node_iter)->QueryComponent(frustum, callback);
		}
	}
}

bool OctNodeBase::RemoveComponent(OctComponentPtr cmp)
{
	if (cmp->m_OctNode)
	{
		_ASSERT(HaveNode(cmp->m_OctNode));
		OctComponentMap::iterator cmp_iter = cmp->m_OctNode->m_Components.find(cmp);
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
	OctComponentMap::iterator cmp_iter = m_Components.begin();
	for (; cmp_iter != m_Components.end(); cmp_iter++)
	{
		_ASSERT(cmp_iter->first->m_OctNode == this);
		cmp_iter->first->m_OctNode = NULL;
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

void OctNodeBase::Compress(void)
{
	BOOST_FOREACH(OctNodeBasePtr & child, m_Childs)
	{
		if (child)
		{
			child->Compress();
			if (child->m_Components.empty()
				&& boost::algorithm::none_of(child->m_Childs, boost::lambda::_1))
			{
				child.reset();
			}
		}
	}
}
