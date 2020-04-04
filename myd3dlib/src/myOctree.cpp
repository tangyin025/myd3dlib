#include "myOctree.h"
#pragma warning(disable:4308)
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/algorithm/cxx11/none_of.hpp>
#include <boost/lambda/lambda.hpp>

using namespace my;

BOOST_CLASS_EXPORT(OctEntity)

const AABB & OctEntity::GetOctAABB(void) const
{
	_ASSERT(m_Node);

	OctNode::OctEntityMap::const_iterator entity_iter = m_Node->m_Entities.find(boost::const_pointer_cast<OctEntity>(shared_from_this()));

	_ASSERT(entity_iter != m_Node->m_Entities.end());

	return entity_iter->second;
}

BOOST_CLASS_EXPORT(OctNode)

const float OctNode::THRESHOLD = 0.1f;

const float OctNode::MIN_BLOCK = 1.0f;

template<class Archive>
void OctNode::save(Archive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_aabb);
	ar << BOOST_SERIALIZATION_NVP(m_Half);
	//ar << BOOST_SERIALIZATION_NVP(m_Entities);
	ar << BOOST_SERIALIZATION_NVP(m_Childs);
}

template<class Archive>
void OctNode::load(Archive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
	ar >> BOOST_SERIALIZATION_NVP(m_Half);
	//ar >> BOOST_SERIALIZATION_NVP(m_Entities);
	//OctEntityMap::iterator entity_iter = m_Entities.begin();
	//for (; entity_iter != m_Entities.end(); entity_iter++)
	//{
	//	entity_iter->first->m_Node = this;
	//}
	ar >> BOOST_SERIALIZATION_NVP(m_Childs);
	for (unsigned int i = 0; i < m_Childs.size(); i++)
	{
		if (m_Childs[i])
		{
			m_Childs[i]->m_Parent = this;
		}
	}
}

template
void OctNode::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void OctNode::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void OctNode::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void OctNode::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void OctNode::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void OctNode::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void OctNode::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void OctNode::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);

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
	if (!m_Parent)
	{
		return this;
	}
	return m_Parent->GetTopNode();
}

OctNode * OctNode::GetTopNode(void)
{
	if (!m_Parent)
	{
		return this;
	}
	return m_Parent->GetTopNode();
}

void OctNode::AddEntity(OctEntityPtr entity, const AABB & aabb)
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
	m_Entities.insert(std::make_pair(entity, aabb));
	entity->m_Node = this;
}

void OctNode::AddToChild(ChildArray::reference & child, const AABB & child_aabb, OctEntityPtr entity, const AABB & aabb)
{
	if (!child)
	{
		child.reset(new OctNode(this, child_aabb));
	}
	child->AddEntity(entity, aabb);
}

void OctNode::QueryEntity(const Ray & ray, QueryCallback * callback) const
{
	OctEntityMap::const_iterator entity_iter = m_Entities.begin();
	for (; entity_iter != m_Entities.end(); entity_iter++)
	{
		if (IntersectionTests::rayAndAABB(ray.p, ray.d, entity_iter->second).first)
		{
			callback->OnQueryEntity(entity_iter->first.get(), entity_iter->second, IntersectionTests::IntersectionTypeRay);
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
			callback->OnQueryEntity(entity_iter->first.get(), entity_iter->second, intersect_type);
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
			callback->OnQueryEntity(entity_iter->first.get(), entity_iter->second, intersect_type);
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
		callback->OnQueryEntity(entity_iter->first.get(), entity_iter->second, IntersectionTests::IntersectionTypeInside);
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

bool OctNode::RemoveEntity(OctEntityPtr entity)
{
	if (entity->m_Node)
	{
		_ASSERT(HaveNode(entity->m_Node));
		OctEntityMap::iterator entity_iter = entity->m_Node->m_Entities.find(entity);
		if (entity_iter != entity->m_Node->m_Entities.end())
		{
			entity->m_Node->m_Entities.erase(entity_iter);
			entity->m_Node = NULL;
			return true;
		}
	}
	return false;
}

void OctNode::ClearAllEntityInCurrentNode(void)
{
	OctEntityMap::iterator entity_iter = m_Entities.begin();
	for (; entity_iter != m_Entities.end(); entity_iter++)
	{
		_ASSERT(entity_iter->first->m_Node == this);
		entity_iter->first->m_Node = NULL;
	}
	m_Entities.clear();
}

void OctNode::ClearAllEntity(void)
{
	ClearAllEntityInCurrentNode();

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

BOOST_CLASS_EXPORT(OctRoot)

template<class Archive>
void OctRoot::save(Archive & ar, const unsigned int version) const
{
	class Callback: public QueryCallback
	{
	public:
		std::vector<std::pair<OctEntityPtr, AABB> > entity_list;

		virtual void OnQueryEntity(my::OctEntity * oct_entity, const my::AABB & aabb, my::IntersectionTests::IntersectionType)
		{
			_ASSERT(oct_entity);
			entity_list.push_back(std::make_pair(oct_entity->shared_from_this(), aabb));
		}
	};

	Callback cb;
	QueryEntityAll(&cb);
	ar << BOOST_SERIALIZATION_NVP(m_aabb);
	ar << BOOST_SERIALIZATION_NVP(m_Half);
	ar << BOOST_SERIALIZATION_NVP(cb.entity_list);
}

template<class Archive>
void OctRoot::load(Archive & ar, const unsigned int version)
{
	class Callback
	{
	public:
		std::vector<std::pair<OctEntityPtr, AABB> > entity_list;
	};

	Callback cb;
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
	ar >> BOOST_SERIALIZATION_NVP(m_Half);
	ar >> BOOST_SERIALIZATION_NVP(cb.entity_list);
	std::vector<std::pair<OctEntityPtr, AABB> >::iterator entity_iter = cb.entity_list.begin();
	for (; entity_iter != cb.entity_list.end(); entity_iter++)
	{
		AddEntity(entity_iter->first, entity_iter->second);
	}
}

template
void OctRoot::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void OctRoot::save<boost::archive::text_oarchive>(boost::archive::text_oarchive & ar, const unsigned int version) const;

template
void OctRoot::save<boost::archive::binary_oarchive>(boost::archive::binary_oarchive & ar, const unsigned int version) const;

template
void OctRoot::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const;

template
void OctRoot::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version);

template
void OctRoot::load<boost::archive::text_iarchive>(boost::archive::text_iarchive & ar, const unsigned int version);

template
void OctRoot::load<boost::archive::binary_iarchive>(boost::archive::binary_iarchive & ar, const unsigned int version);

template
void OctRoot::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version);
