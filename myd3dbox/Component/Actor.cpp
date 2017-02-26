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

void Actor::RequestResource(void)
{
	_ASSERT(!m_Requested);
	m_Requested = true;
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->RequestResource();
	}
}

void Actor::ReleaseResource(void)
{
	_ASSERT(m_Requested);
	m_Requested = false;
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->ReleaseResource();
	}
}

void Actor::Update(float fElapsedTime)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->Update(fElapsedTime);
	}
}

void Actor::QueryRenderComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->AddToPipeline(frustum, pipeline, PassMask);
	}
}

void Actor::UpdateLod(const my::Vector3 & ViewedPos, const my::Vector3 & TargetPos)
{
	ComponentPtrList::iterator cmp_iter = m_Cmps.begin();
	for (; cmp_iter != m_Cmps.end(); cmp_iter++)
	{
		(*cmp_iter)->UpdateLod(ViewedPos, TargetPos);
	}
}
