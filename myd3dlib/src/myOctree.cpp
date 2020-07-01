#include "myOctree.h"
#include <boost/serialization/export.hpp>
#include <boost/algorithm/cxx11/none_of.hpp>
#include <boost/lambda/lambda.hpp>

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

void OctNode::AddEntity(OctEntity * entity, const AABB & aabb)
{
	_ASSERT(!entity->m_Node);
	if (m_aabb.m_max.x - m_aabb.m_min.x > MIN_BLOCK + THRESHOLD || m_aabb.m_max.y - m_aabb.m_min.y > MIN_BLOCK + THRESHOLD || m_aabb.m_max.z - m_aabb.m_min.z > MIN_BLOCK + THRESHOLD)
	{
		if (aabb.m_min.x > m_Half.x - THRESHOLD && aabb.m_max.x < m_aabb.m_max.x + THRESHOLD)
		{
			if (aabb.m_min.y > m_Half.y - THRESHOLD && aabb.m_max.y < m_aabb.m_max.y + THRESHOLD)
			{
				if (aabb.m_min.z > m_Half.z - THRESHOLD && aabb.m_max.z < m_aabb.m_max.z + THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantPxPyPz], m_aabb.Slice<AABB::QuadrantPxPyPz>(m_Half), entity, aabb);
					return;
				}
				else if (aabb.m_max.z < m_Half.z + THRESHOLD && aabb.m_min.z > m_aabb.m_min.z - THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantPxPyNz], m_aabb.Slice<AABB::QuadrantPxPyNz>(m_Half), entity, aabb);
					return;
				}
			}
			else if (aabb.m_max.y < m_Half.y + THRESHOLD && aabb.m_min.y > m_aabb.m_min.y - THRESHOLD)
			{
				if (aabb.m_min.z > m_Half.z - THRESHOLD && aabb.m_max.z < m_aabb.m_max.z + THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantPxNyPz], m_aabb.Slice<AABB::QuadrantPxNyPz>(m_Half), entity, aabb);
					return;
				}
				else if (aabb.m_max.z < m_Half.z + THRESHOLD && aabb.m_min.z > m_aabb.m_min.z - THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantPxNyNz], m_aabb.Slice<AABB::QuadrantPxNyNz>(m_Half), entity, aabb);
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
					AddToChild(m_Childs[AABB::QuadrantNxPyPz], m_aabb.Slice<AABB::QuadrantNxPyPz>(m_Half), entity, aabb);
					return;
				}
				else if (aabb.m_max.z < m_Half.z + THRESHOLD && aabb.m_min.z > m_aabb.m_min.z - THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantNxPyNz], m_aabb.Slice<AABB::QuadrantNxPyNz>(m_Half), entity, aabb);
					return;
				}
			}
			else if (aabb.m_max.y < m_Half.y + THRESHOLD && aabb.m_min.y > m_aabb.m_min.y - THRESHOLD)
			{
				if (aabb.m_min.z > m_Half.z - THRESHOLD && aabb.m_max.z < m_aabb.m_max.z + THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantNxNyPz], m_aabb.Slice<AABB::QuadrantNxNyPz>(m_Half), entity, aabb);
					return;
				}
				else if (aabb.m_max.z < m_Half.z + THRESHOLD && aabb.m_min.z > m_aabb.m_min.z - THRESHOLD)
				{
					AddToChild(m_Childs[AABB::QuadrantNxNyNz], m_aabb.Slice<AABB::QuadrantNxNyNz>(m_Half), entity, aabb);
					return;
				}
			}
		}
	}
	std::pair<OctEntityMap::iterator, bool> result = m_Entities.insert(std::make_pair(entity, aabb));
	_ASSERT(result.second);
	entity->m_Node = this;
	entity->m_OctAabb = &result.first->second;
}

void OctNode::AddToChild(ChildArray::reference & child, const AABB & child_aabb, OctEntity * entity, const AABB & aabb)
{
	if (!child)
	{
		child.reset(new OctNode(this, child_aabb));
	}
	child->OctNode::AddEntity(entity, aabb);
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
			if (IntersectionTests::rayAndAABB(ray.p, ray.d, (*node_iter)->m_aabb).first)
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
			switch (IntersectionTests::IntersectAABBAndAABB((*node_iter)->m_aabb, aabb))
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
			switch (IntersectionTests::IntersectAABBAndFrustum((*node_iter)->m_aabb, frustum))
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
		OctEntityMap::iterator entity_iter = entity->m_Node->m_Entities.find(entity);
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
