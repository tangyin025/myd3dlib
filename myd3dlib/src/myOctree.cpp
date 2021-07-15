#include "myOctree.h"
#include <boost/serialization/export.hpp>
#include <boost/algorithm/cxx11/none_of.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/range/algorithm/find_if.hpp>

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

const OctNode * OctNode::GetTopNode(void) const
{
	if (m_Parent)
	{
		return m_Parent->GetTopNode();
	}
	return this;
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
	_ASSERT(m_Entities.end() == boost::find_if(m_Entities, (&boost::lambda::_1)->* & std::pair<OctEntity*, AABB>::first == entity));
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

void OctNode::QueryEntity(const Ray & ray, QueryCallback * callback) const
{
	OctEntityMap::const_iterator entity_iter = m_Entities.begin();
	for (; entity_iter != m_Entities.end(); entity_iter++)
	{
		if (IntersectionTests::rayAndAABB(ray.p, ray.d, entity_iter->second).first)
		{
			callback->OnQueryEntity(entity_iter->first, entity_iter->second, IntersectionTests::IntersectionTypeRay);
		}
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for (; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			if (IntersectionTests::rayAndAABB(ray.p, ray.d, *(*node_iter)).first)
			{
				(*node_iter)->QueryEntity(ray, callback);
			}
		}
	}
}

void OctNode::QueryEntity(const AABB & aabb, QueryCallback * callback) const
{
	OctEntityMap::const_iterator entity_iter = m_Entities.begin();
	for (; entity_iter != m_Entities.end(); entity_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndAABB(entity_iter->second, aabb);
		switch (intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			callback->OnQueryEntity(entity_iter->first, entity_iter->second, intersect_type);
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
				(*node_iter)->QueryEntityAll(callback);
				break;

			case IntersectionTests::IntersectionTypeIntersect:
				(*node_iter)->QueryEntity(aabb, callback);
				break;
			}
		}
	}
}

void OctNode::QueryEntity(const Frustum & frustum, QueryCallback * callback) const
{
	OctEntityMap::const_iterator entity_iter = m_Entities.begin();
	for (; entity_iter != m_Entities.end(); entity_iter++)
	{
		IntersectionTests::IntersectionType intersect_type = IntersectionTests::IntersectAABBAndFrustum(entity_iter->second, frustum);
		switch (intersect_type)
		{
		case IntersectionTests::IntersectionTypeInside:
		case IntersectionTests::IntersectionTypeIntersect:
			callback->OnQueryEntity(entity_iter->first, entity_iter->second, intersect_type);
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
				(*node_iter)->QueryEntityAll(callback);
				break;

			case IntersectionTests::IntersectionTypeIntersect:
				(*node_iter)->QueryEntity(frustum, callback);
				break;
			}
		}
	}
}

void OctNode::QueryEntityAll(QueryCallback * callback) const
{
	OctEntityMap::const_iterator entity_iter = m_Entities.begin();
	for(; entity_iter != m_Entities.end(); entity_iter++)
	{
		callback->OnQueryEntity(entity_iter->first, entity_iter->second, IntersectionTests::IntersectionTypeInside);
	}

	ChildArray::const_iterator node_iter = m_Childs.begin();
	for(; node_iter != m_Childs.end(); node_iter++)
	{
		if (*node_iter)
		{
			(*node_iter)->QueryEntityAll(callback);
		}
	}
}

bool OctNode::RemoveEntity(OctEntity * entity)
{
	if (entity->m_Node)
	{
		_ASSERT(HaveNode(entity->m_Node));
		OctEntityMap::iterator entity_iter = boost::find_if(entity->m_Node->m_Entities, (&boost::lambda::_1)->* & std::pair<OctEntity*, AABB>::first == entity);
		if (entity_iter != entity->m_Node->m_Entities.end())
		{
			entity->m_Node->m_Entities.erase(entity_iter);
			entity->m_Node = NULL;
			entity->m_OctAabb = NULL;
			return true;
		}
	}
	return false;
}

void OctNode::ClearAllEntity(void)
{
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
	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			m_Childs[i]->Flush();
			if (m_Childs[i]->m_Entities.empty()
				&& boost::algorithm::none_of(m_Childs[i]->m_Childs, boost::lambda::_1))
			{
				m_Childs[i].reset();
			}
		}
	}
}
