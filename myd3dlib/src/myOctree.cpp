#include "StdAfx.h"
#include "myOctree.h"

using namespace my;

void OctNodeBase::QueryComponent(const Ray & ray, IQueryCallback * callback)
{
	if (IntersectionTests::rayAndAABB(ray.p, ray.d, m_aabb).first)
	{
		AABBComponentPtrPairList::iterator cmp_iter = m_ComponentList.begin();
		for(; cmp_iter != m_ComponentList.end(); cmp_iter++)
		{
			if (IntersectionTests::rayAndAABB(ray.p, ray.d, cmp_iter->first).first)
			{
				(*callback)(cmp_iter->second, IntersectionTests::IntersectionTypeRay);
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
	AABBComponentPtrPairList::iterator cmp_iter = m_ComponentList.begin();
	for(; cmp_iter != m_ComponentList.end(); cmp_iter++)
	{
		(*callback)(cmp_iter->second, IntersectionTests::IntersectionTypeInside);
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

void OctNodeBase::QueryComponentIntersected(const Frustum & frustum, IQueryCallback * callback)
{
	AABBComponentPtrPairList::iterator cmp_iter = m_ComponentList.begin();
	for(; cmp_iter != m_ComponentList.end(); cmp_iter++)
	{
		// ! performance lost when node have many pieces of object
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndFrustum(cmp_iter->first, frustum);
		switch(intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(cmp_iter->second, intersect_type);
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

bool OctNodeBase::HaveChildNodes(void)
{
	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			return true;
		}
	}
	return false;
}

bool OctNodeBase::RemoveComponent(AABBComponentPtr cmp)
{
	AABBComponentPtrPairList::iterator cmp_iter = m_ComponentList.begin();
	for (; cmp_iter != m_ComponentList.end(); cmp_iter++)
	{
		if (cmp_iter->second == cmp)
		{
			m_ComponentList.erase(cmp_iter);
			return true;
		}
	}

	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i] && m_Childs[i]->RemoveComponent(cmp))
		{
			if (!m_Childs[i]->HaveChildNodes() && m_Childs[i]->m_ComponentList.empty())
			{
				m_Childs[i].reset();
			}
			return true;
		}
	}
	return false;
}

void OctRoot::ClearComponents(void)
{
	m_ComponentList.clear();

	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		m_Childs[i].reset();
	}
}
