// Copyright (c) 2011-2024 tangyin025
// License: MIT
#include "myOctree.h"
#include "myCollision.h"
#include <boost/serialization/export.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/cxx11/none_of.hpp>
#include <boost/scope_exit.hpp>

using namespace my;

BOOST_CLASS_EXPORT(OctRoot)

OctEntity::~OctEntity(void)
{
	if (m_Node)
	{
		_ASSERT(false);
		
		m_Node->OctNode::RemoveEntity(this);
	}

	_ASSERT(!m_OctAabb);
}

bool OctNode::HaveNode(const OctNode * node) const
{
	for (; node; node = node->m_Parent)
	{
		if (this == node)
		{
			return true;
		}
	}
	return false;
}

OctNode * OctNode::GetTopNode(void)
{
	if (m_Parent)
	{
		return m_Parent->GetTopNode();
	}
	return this;
}

void OctNode::AddEntity(OctEntity * entity, const AABB & aabb, float minblock, float threshold)
{
	//_ASSERT(!GetTopNode()->m_QueryEntityMuted);

	_ASSERT(!entity->m_Node);

	_ASSERT(aabb.IsValid());

	if (m_max.x - m_min.x > minblock + threshold || m_max.y - m_min.y > minblock + threshold || m_max.z - m_min.z > minblock + threshold)
	{
		if (aabb.m_min.x > m_Half.x - threshold && aabb.m_max.x < m_max.x + threshold)
		{
			if (aabb.m_min.y > m_Half.y - threshold && aabb.m_max.y < m_max.y + threshold)
			{
				if (aabb.m_min.z > m_Half.z - threshold && aabb.m_max.z < m_max.z + threshold)
				{
					AddToChild(m_Childs[QuadrantPxPyPz], Slice<QuadrantPxPyPz>(m_Half), entity, aabb, minblock, threshold);
					return;
				}
				else if (aabb.m_max.z < m_Half.z + threshold && aabb.m_min.z > m_min.z - threshold)
				{
					AddToChild(m_Childs[QuadrantPxPyNz], Slice<QuadrantPxPyNz>(m_Half), entity, aabb, minblock, threshold);
					return;
				}
			}
			else if (aabb.m_max.y < m_Half.y + threshold && aabb.m_min.y > m_min.y - threshold)
			{
				if (aabb.m_min.z > m_Half.z - threshold && aabb.m_max.z < m_max.z + threshold)
				{
					AddToChild(m_Childs[QuadrantPxNyPz], Slice<QuadrantPxNyPz>(m_Half), entity, aabb, minblock, threshold);
					return;
				}
				else if (aabb.m_max.z < m_Half.z + threshold && aabb.m_min.z > m_min.z - threshold)
				{
					AddToChild(m_Childs[QuadrantPxNyNz], Slice<QuadrantPxNyNz>(m_Half), entity, aabb, minblock, threshold);
					return;
				}
			}
		}
		else if (aabb.m_max.x < m_Half.x + threshold && aabb.m_min.x > m_min.x - threshold)
		{
			if (aabb.m_min.y > m_Half.y - threshold && aabb.m_max.y < m_max.y + threshold)
			{
				if (aabb.m_min.z > m_Half.z - threshold && aabb.m_max.z < m_max.z + threshold)
				{
					AddToChild(m_Childs[QuadrantNxPyPz], Slice<QuadrantNxPyPz>(m_Half), entity, aabb, minblock, threshold);
					return;
				}
				else if (aabb.m_max.z < m_Half.z + threshold && aabb.m_min.z > m_min.z - threshold)
				{
					AddToChild(m_Childs[QuadrantNxPyNz], Slice<QuadrantNxPyNz>(m_Half), entity, aabb, minblock, threshold);
					return;
				}
			}
			else if (aabb.m_max.y < m_Half.y + threshold && aabb.m_min.y > m_min.y - threshold)
			{
				if (aabb.m_min.z > m_Half.z - threshold && aabb.m_max.z < m_max.z + threshold)
				{
					AddToChild(m_Childs[QuadrantNxNyPz], Slice<QuadrantNxNyPz>(m_Half), entity, aabb, minblock, threshold);
					return;
				}
				else if (aabb.m_max.z < m_Half.z + threshold && aabb.m_min.z > m_min.z - threshold)
				{
					AddToChild(m_Childs[QuadrantNxNyNz], Slice<QuadrantNxNyNz>(m_Half), entity, aabb, minblock, threshold);
					return;
				}
			}
		}
	}

	_ASSERT(m_Entities.end() == boost::find_if(m_Entities,
		boost::bind(std::equal_to<OctEntity*>(), boost::bind(&std::pair<OctEntity*, AABB>::first, boost::placeholders::_1), entity)));
	m_Entities.push_back(std::make_pair(entity, aabb));
	entity->m_Node = this;
	entity->m_OctAabb = &m_Entities.rbegin()->second;
}

void OctNode::AddToChild(ChildArray::reference & child, const AABB & child_aabb, OctEntity * entity, const AABB & aabb, float minblock, float threshold)
{
	if (!child)
	{
		child.reset(new OctNode(this, child_aabb.m_min, child_aabb.m_max));
	}
	child->OctNode::AddEntity(entity, aabb, minblock, threshold);
}

bool OctNode::QueryEntity(const Ray & ray, QueryCallback * callback) const
{
#ifdef _DEBUG
	const_cast<bool&>(m_QueryEntityMuted) = true;
	BOOST_SCOPE_EXIT(&m_QueryEntityMuted)
	{
		const_cast<bool&>(m_QueryEntityMuted) = false;
	}
	BOOST_SCOPE_EXIT_END
#endif

	OctEntityMap::const_iterator entity_iter = m_Entities.begin();
	for (; entity_iter != m_Entities.end(); entity_iter++)
	{
		if (IntersectionTests::rayAndAABB(ray.p, ray.d, entity_iter->second).first)
		{
			if (!callback->OnQueryEntity(entity_iter->first, entity_iter->second))
			{
				return false;
			}
		}
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for (; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			if (IntersectionTests::rayAndAABB(ray.p, ray.d, *(*node_iter)).first)
			{
				if (!(*node_iter)->QueryEntity(ray, callback))
				{
					return false;
				}
			}
		}
	}
	return true;
}

bool OctNode::QueryEntity(const AABB & aabb, QueryCallback * callback) const
{
#ifdef _DEBUG
	const_cast<bool&>(m_QueryEntityMuted) = true;
	BOOST_SCOPE_EXIT(&m_QueryEntityMuted)
	{
		const_cast<bool&>(m_QueryEntityMuted) = false;
	}
	BOOST_SCOPE_EXIT_END
#endif
		
	OctEntityMap::const_iterator entity_iter = m_Entities.begin();
	for (; entity_iter != m_Entities.end(); entity_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndAABB(entity_iter->second, aabb);
		switch (intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			if (!callback->OnQueryEntity(entity_iter->first, entity_iter->second))
			{
				return false;
			}
			break;
		}
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for (; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			switch (IntersectionTests::IntersectAABBAndAABB(*(*node_iter), aabb))
			{
			case IntersectionTests::IntersectionTypeInside:
				if (!(*node_iter)->QueryAllEntity(callback))
				{
					return false;
				}
				break;

			case IntersectionTests::IntersectionTypeIntersect:
				if (!(*node_iter)->QueryEntity(aabb, callback))
				{
					return false;
				}
				break;
			}
		}
	}
	return true;
}

bool OctNode::QueryEntity(const Frustum & frustum, QueryCallback * callback) const
{
#ifdef _DEBUG
	const_cast<bool&>(m_QueryEntityMuted) = true;
	BOOST_SCOPE_EXIT(&m_QueryEntityMuted)
	{
		const_cast<bool&>(m_QueryEntityMuted) = false;
	}
	BOOST_SCOPE_EXIT_END
#endif

	OctEntityMap::const_iterator entity_iter = m_Entities.begin();
	for (; entity_iter != m_Entities.end(); entity_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndFrustum(entity_iter->second, frustum);
		switch (intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			if (!callback->OnQueryEntity(entity_iter->first, entity_iter->second))
			{
				return false;
			}
			break;
		}
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for (; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			switch (IntersectionTests::IntersectAABBAndFrustum(*(*node_iter), frustum))
			{
			case IntersectionTests::IntersectionTypeInside:
				if (!(*node_iter)->QueryAllEntity(callback))
				{
					return false;
				}
				break;

			case IntersectionTests::IntersectionTypeIntersect:
				if (!(*node_iter)->QueryEntity(frustum, callback))
				{
					return false;
				}
				break;
			}
		}
	}
	return true;
}

bool OctNode::QueryAllEntity(QueryCallback * callback) const
{
#ifdef _DEBUG
	const_cast<bool&>(m_QueryEntityMuted) = true;
	BOOST_SCOPE_EXIT(&m_QueryEntityMuted)
	{
		const_cast<bool&>(m_QueryEntityMuted) = false;
	}
	BOOST_SCOPE_EXIT_END
#endif
		
	OctEntityMap::const_iterator entity_iter = m_Entities.begin();
	for(; entity_iter != m_Entities.end(); entity_iter++)
	{
		if (!callback->OnQueryEntity(entity_iter->first, entity_iter->second))
		{
			return false;
		}
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for(; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			if (!(*node_iter)->QueryAllEntity(callback))
			{
				return false;
			}
		}
	}
	return true;
}

int OctNode::GetAllEntityNum(void) const
{
	int ret = m_Entities.size();

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for (; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			ret += (*node_iter)->GetAllEntityNum();
		}
	}
	return ret;
}

AABB OctNode::GetAllEntityAABB(const AABB & root_aabb) const
{
	AABB ret(root_aabb);
	OctEntityMap::const_iterator entity_iter = m_Entities.begin();
	for (; entity_iter != m_Entities.end(); entity_iter++)
	{
		ret.unionSelf(entity_iter->second);
	}

	for (int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			switch (i)
			{
			case QuadrantPxPyPz:
				if (!ret.Contains(Slice<QuadrantPxPyPz>(m_Half)))
				{
					ret.unionSelf(m_Childs[i]->GetAllEntityAABB(ret));
				}
				break;
			case QuadrantPxPyNz:
				if (!ret.Contains(Slice<QuadrantPxPyNz>(m_Half)))
				{
					ret.unionSelf(m_Childs[i]->GetAllEntityAABB(ret));
				}
				break;
			case QuadrantPxNyPz:
				if (!ret.Contains(Slice<QuadrantPxNyPz>(m_Half)))
				{
					ret.unionSelf(m_Childs[i]->GetAllEntityAABB(ret));
				}
				break;
			case QuadrantPxNyNz:
				if (!ret.Contains(Slice<QuadrantPxNyNz>(m_Half)))
				{
					ret.unionSelf(m_Childs[i]->GetAllEntityAABB(ret));
				}
				break;
			case QuadrantNxPyPz:
				if (!ret.Contains(Slice<QuadrantNxPyPz>(m_Half)))
				{
					ret.unionSelf(m_Childs[i]->GetAllEntityAABB(ret));
				}
				break;
			case QuadrantNxPyNz:
				if (!ret.Contains(Slice<QuadrantNxPyNz>(m_Half)))
				{
					ret.unionSelf(m_Childs[i]->GetAllEntityAABB(ret));
				}
				break;
			case QuadrantNxNyPz:
				if (!ret.Contains(Slice<QuadrantNxNyPz>(m_Half)))
				{
					ret.unionSelf(m_Childs[i]->GetAllEntityAABB(ret));
				}
				break;
			case QuadrantNxNyNz:
				if (!ret.Contains(Slice<QuadrantNxNyNz>(m_Half)))
				{
					ret.unionSelf(m_Childs[i]->GetAllEntityAABB(ret));
				}
				break;
			}
		}
	}
	return ret;
}

void OctNode::RemoveEntity(OctEntity * entity)
{
	_ASSERT(!GetTopNode()->m_QueryEntityMuted);

	if (entity->m_Node)
	{
		_ASSERT(HaveNode(entity->m_Node));
		OctEntityMap::iterator entity_iter = boost::find_if(entity->m_Node->m_Entities, boost::bind(std::equal_to<OctEntity*>(), boost::bind(&std::pair<OctEntity*, AABB>::first, boost::placeholders::_1), entity));
		if (entity_iter != entity->m_Node->m_Entities.end())
		{
			entity->m_Node->m_Entities.erase(entity_iter);
			entity->m_Node = NULL;
			entity->m_OctAabb = NULL;
		}
	}
}

void OctNode::ClearAllEntity(void)
{
	_ASSERT(!GetTopNode()->m_QueryEntityMuted);

	OctNode * Root = GetTopNode();
	OctEntityMap::iterator entity_iter = m_Entities.begin();
	for (; entity_iter != m_Entities.end(); entity_iter = m_Entities.begin())
	{
		_ASSERT(entity_iter->first->m_Node == this);
		Root->RemoveEntity(entity_iter->first);
	}

	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			m_Childs[i]->ClearAllEntity();
			m_Childs[i].reset();
		}
	}
}

void OctNode::Flush(void)
{
	_ASSERT(!GetTopNode()->m_QueryEntityMuted);

	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			m_Childs[i]->Flush();
			if (m_Childs[i]->m_Entities.empty() && boost::algorithm::none_of(
				m_Childs[i]->m_Childs, boost::bind(&boost::shared_ptr<OctNode>::operator bool, boost::placeholders::_1)))
			{
				m_Childs[i].reset();
			}
		}
	}
}
