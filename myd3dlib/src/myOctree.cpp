#include "StdAfx.h"
#include "myOctree.h"

using namespace my;

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
		OctComponentSet::iterator cmp_iter = m_Components.begin();
		for(; cmp_iter != m_Components.end(); cmp_iter++)
		{
			if (IntersectionTests::rayAndAABB(ray.p, ray.d, cmp_iter->second).first)
			{
				(*callback)(cmp_iter->first, IntersectionTests::IntersectionTypeRay);
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
		(*callback)(cmp_iter->first, IntersectionTests::IntersectionTypeInside);
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
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndAABB(cmp_iter->second, aabb);
		switch(intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(cmp_iter->first, intersect_type);
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
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndFrustum(cmp_iter->second, frustum);
		switch(intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(cmp_iter->first, intersect_type);
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
	if (cmp->m_OctNode)
	{
		_ASSERT(HaveNode(cmp->m_OctNode));
		OctComponentSet::iterator cmp_iter = cmp->m_OctNode->m_Components.find(cmp);
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
	OctComponentSet::iterator cmp_iter = m_Components.begin();
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
