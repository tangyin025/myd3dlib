#include "StdAfx.h"
#include "myOctree.h"

using namespace my;

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
	AABBComponentPtrList::iterator comp_iter = m_ComponentList.begin();
	for(; comp_iter != m_ComponentList.end(); comp_iter++)
	{
		(*callback)(comp_iter->get(), IntersectionTests::IntersectionTypeInside);
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
	AABBComponentPtrList::iterator comp_iter = m_ComponentList.begin();
	for(; comp_iter != m_ComponentList.end(); comp_iter++)
	{
		// ! performance lost when node have many pieces of object
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndFrustum((*comp_iter)->m_aabb, frustum);
		switch(intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			(*callback)(comp_iter->get(), intersect_type);
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

bool OctNodeBase::RemoveComponent(AABBComponentPtr comp)
{
	AABBComponentPtrList::iterator comp_iter = std::find(m_ComponentList.begin(), m_ComponentList.end(), comp);
	if (comp_iter != m_ComponentList.end())
	{
		m_ComponentList.erase(comp_iter);
		return true;
	}
	else
	{
		for (unsigned int i = 0; i < m_Childs.size(); i++)
		{
			if (m_Childs[i] && m_Childs[i]->RemoveComponent(comp))
			{
				if (!m_Childs[i]->HaveChildNodes() && m_Childs[i]->m_ComponentList.empty())
				{
					m_Childs[i].reset();
				}
				return true;
			}
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
