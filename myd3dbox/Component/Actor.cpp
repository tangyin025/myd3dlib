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
	ar << BOOST_SERIALIZATION_NVP(m_Animator);
	ar << BOOST_SERIALIZATION_NVP(m_Cmps);
}

template<>
void Actor::load<boost::archive::polymorphic_iarchive>(boost::archive::polymorphic_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(Component);
	ar >> BOOST_SERIALIZATION_NVP(m_aabb);
	ar >> BOOST_SERIALIZATION_NVP(m_Animator);
	if (m_Animator)
	{
		m_Animator->m_Cmp = this;
	}
	ar >> BOOST_SERIALIZATION_NVP(m_Cmps);
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for(; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->m_Actor = this;
	}
}

void Actor::RequestResource(void)
{
	_ASSERT(!m_Requested);

	Component::RequestResource();

	if (m_Animator)
	{
		m_Animator->RequestResource();
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->RequestResource();
	}
}

void Actor::ReleaseResource(void)
{
	_ASSERT(m_Requested);

	if (m_Animator)
	{
		m_Animator->ReleaseResource();
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->ReleaseResource();
	}

	Component::ReleaseResource();
}

void Actor::OnEnterPxScene(physx::PxScene * scene)
{
	Component::OnEnterPxScene(scene);

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->OnEnterPxScene(scene);
	}
}

void Actor::OnLeavePxScene(physx::PxScene * scene)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->OnLeavePxScene(scene);
	}

	Component::OnLeavePxScene(scene);
}

void Actor::Update(float fElapsedTime)
{
	Component::Update(fElapsedTime);

	if (m_Animator)
	{
		m_Animator->Update(fElapsedTime);
	}

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->Update(fElapsedTime);
	}
}

my::AABB Actor::CalculateAABB(void) const
{
	AABB ret = Component::CalculateAABB();
	ComponentPtrList::const_iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		ret.unionSelf((*cmp_iter)->CalculateAABB().transform((*cmp_iter)->CalculateLocal()));
	}
	return ret;
}

void Actor::UpdateAABB(void)
{
	m_aabb = CalculateAABB();
	if (!m_aabb.IsValid())
	{
		m_aabb.unionSelf(AABB(m_aabb.Center() - 1, m_aabb.Center() + 1));
	}
}

void Actor::UpdateWorld(const my::Matrix4 & World)
{
	Component::UpdateWorld(World);

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->UpdateWorld(m_World);
	}
}

void Actor::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	Component::AddToPipeline(frustum, pipeline, PassMask);

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->AddToPipeline(frustum, pipeline, PassMask);
	}
}

void Actor::UpdateLod(const my::Vector3 & ViewPos)
{
	Component::UpdateLod(ViewPos);

	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->UpdateLod(ViewPos);
	}
}

void Actor::AddComponent(ComponentPtr cmp)
{
	_ASSERT(!cmp->m_Actor);
	m_Cmps.push_back(cmp);
	cmp->m_Actor = this;
}

void Actor::RemoveComponent(ComponentPtr cmp)
{
	ComponentPtrList::iterator cmp_iter = std::find(m_Cmps.begin(), m_Cmps.end(), cmp);
	if (cmp_iter != m_Cmps.end())
	{
		_ASSERT((*cmp_iter)->m_Actor == this);
		m_Cmps.erase(cmp_iter);
		(*cmp_iter)->m_Actor = NULL;
	}
}

void Actor::ClearAllComponent(ComponentPtr cmp)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->m_Actor = NULL;
	}
	m_Cmps.clear();
}
