#pragma once

#include "RenderComponent.h"
#include <boost/array.hpp>

class OctreeNode
{
public:
	const my::AABB m_AABB;

	typedef std::vector<RenderComponentPtr> RenderComponentList;

	RenderComponentList m_ComponentList;

	typedef boost::array<boost::shared_ptr<OctreeNode>, 2> ChildArray;

	ChildArray m_Childs;

public:
	OctreeNode(const my::AABB & AABB)
		: m_AABB(AABB)
	{
	}

	template <typename ChildType, std::size_t N>
	void PushComponentToChild(RenderComponentPtr comp, const my::AABB & AABB)
	{
		if (!m_Childs[N])
		{
			m_Childs[N] = boost::shared_ptr<ChildType>(new ChildType(AABB));
		}
		m_Childs[N]->PushComponent(comp);
	}

	virtual void PushComponent(RenderComponentPtr comp) = 0;
};

class OctreeNodeX : public OctreeNode
{
public:
	const float m_X;

public:
	OctreeNodeX(const my::AABB & AABB)
		: OctreeNode(AABB)
		, m_X((AABB.Min.x + AABB.Max.x) * 0.5f)
	{
	}

	virtual void PushComponent(RenderComponentPtr comp);
};

class OctreeNodeY : public OctreeNode
{
public:
	const float m_Y;

public:
	OctreeNodeY(const my::AABB & AABB)
		: OctreeNode(AABB)
		, m_Y((AABB.Min.y + AABB.Max.y) * 0.5f)
	{
	}

	virtual void PushComponent(RenderComponentPtr comp);
};

class OctreeNodeZ : public OctreeNode
{
public:
	const float m_Z;

public:
	OctreeNodeZ(const my::AABB & AABB)
		: OctreeNode(AABB)
		, m_Z((AABB.Min.z + AABB.Max.z) * 0.5f)
	{
	}

	virtual void PushComponent(RenderComponentPtr comp);
};

typedef OctreeNodeX OctreeRoot;
