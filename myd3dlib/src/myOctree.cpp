#include "StdAfx.h"
#include "myOctree.h"

using namespace my;

void OctNodeBase::QueryComponent(const Ray & ray, IQueryCallback * callback)
{
	if (IntersectionTests::rayAndAABB(ray.p, ray.d, m_aabb).first)
	{
		AABBComponentPtrMap::iterator cmp_iter = m_ComponentList.begin();
		for(; cmp_iter != m_ComponentList.end(); cmp_iter++)
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
	AABBComponentPtrMap::iterator cmp_iter = m_ComponentList.begin();
	for(; cmp_iter != m_ComponentList.end(); cmp_iter++)
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
	AABBComponentPtrMap::iterator cmp_iter = m_ComponentList.begin();
	for(; cmp_iter != m_ComponentList.end(); cmp_iter++)
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
	AABBComponentPtrMap::iterator cmp_iter = m_ComponentList.begin();
	for(; cmp_iter != m_ComponentList.end(); cmp_iter++)
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

bool OctNodeBase::RemoveComponent(AABBComponentPtr cmp)
{
	AABBComponentPtrMap::iterator cmp_iter = m_ComponentList.find(cmp);
	if (cmp_iter != m_ComponentList.end())
	{
		_ASSERT(cmp_iter->first->GetOctNode() == this);
		cmp_iter->first->SetOctNode(NULL);
		m_ComponentList.erase(cmp_iter);
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

void OctNodeBase::ClearComponents(void)
{
	AABBComponentPtrMap::iterator cmp_iter = m_ComponentList.begin();
	if (cmp_iter != m_ComponentList.end())
	{
		_ASSERT(cmp_iter->first->GetOctNode() == this);
		cmp_iter->first->SetOctNode(NULL);
	}
	m_ComponentList.clear();

	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			m_Childs[i]->ClearComponents();
			m_Childs[i].reset();
		}
	}
}
