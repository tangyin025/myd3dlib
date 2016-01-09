#include "StdAfx.h"
#include "myOctree.h"

using namespace my;

void OctNodeBase::QueryComponent(const Ray & ray, IQueryCallback * callback)
{
	if (IntersectionTests::rayAndAABB(ray.p, ray.d, m_aabb).first)
	{
		OctComponentSet::iterator cmp_iter = m_Components.begin();
		for(; cmp_iter != m_Components.end(); cmp_iter++)
		{
			if (IntersectionTests::rayAndAABB(ray.p, ray.d, (*cmp_iter)->m_aabb).first)
			{
				(*callback)(*cmp_iter, IntersectionTests::IntersectionTypeRay);
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
	OctComponentSet::iterator cmp_iter = m_Components.begin();
	for(; cmp_iter != m_Components.end(); cmp_iter++)
	{
		(*callback)(*cmp_iter, IntersectionTests::IntersectionTypeInside);
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
	OctComponentSet::iterator cmp_iter = m_Components.begin();
	for(; cmp_iter != m_Components.end(); cmp_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndAABB((*cmp_iter)->m_aabb, aabb);
		switch(intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(*cmp_iter, intersect_type);
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
	OctComponentSet::iterator cmp_iter = m_Components.begin();
	for(; cmp_iter != m_Components.end(); cmp_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndFrustum((*cmp_iter)->m_aabb, frustum);
		switch(intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(*cmp_iter, intersect_type);
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

bool OctNodeBase::RemoveComponent(OctComponent * cmp)
{
	OctComponentSet::iterator cmp_iter = m_Components.find(cmp);
	if (cmp_iter != m_Components.end())
	{
		_ASSERT((*cmp_iter)->m_OctNode == this);
		(*cmp_iter)->m_OctNode = NULL;
		m_Components.erase(cmp_iter);
		return true;
	}

	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i] && m_Childs[i]->RemoveComponent(cmp))
		{
			return true;
		}
	}
	return false;
}

void OctNodeBase::ClearAllComponents(void)
{
	OctComponentSet::iterator cmp_iter = m_Components.begin();
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
