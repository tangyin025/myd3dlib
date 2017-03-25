#include "stdafx.h"
#include "Actor.h"
#include "Animator.h"
#include <boost/archive/polymorphic_iarchive.hpp>
#include <boost/archive/polymorphic_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(Actor)

template<>
void Actor::save<boost::archive::polymorphic_oarchive>(boost::archive::polymorphic_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar << BOOST_SERIALIZATION_NVP(m_aabb);
}

template<>
void Actor::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
}

void Actor::RequestResource(void)
{
	_ASSERT(!m_Requested);
	m_Requested = true;
	Component::RequestResource();
}

void Actor::ReleaseResource(void)
{
	_ASSERT(m_Requested);
	m_Requested = false;
	Component::ReleaseResource();
}

void Actor::Update(float fElapsedTime)
{
	Component::Update(fElapsedTime);
}

void Actor::UpdateAABB(void)
{
	m_aabb = Component::CalculateAABB();
	if (!m_aabb.IsValid())
	{
		m_aabb.unionSelf(AABB(m_aabb.Center() - 1, m_aabb.Center() + 1));
	}
}

void Actor::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	Component::AddToPipeline(frustum, pipeline, PassMask);
}

void Actor::UpdateLod(const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos)
{
	Component::UpdateLod(ViewedPos, TargetPos);
}
