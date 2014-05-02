#include "StdAfx.h"
#include "Octree.h"

using namespace my;

void OctreeNodeX::PushComponent(RenderComponentPtr comp)
{
	if (comp->m_AABB.Max.x < m_X)
	{
		PushComponentToChild<OctreeNodeY, 0>(comp, my::AABB(m_AABB.Min, Vector3(m_X, m_AABB.Max.y, m_AABB.Max.z)));
	}
	else if (comp->m_AABB.Min.x > m_X)
	{
		PushComponentToChild<OctreeNodeY, 1>(comp, my::AABB(Vector3(m_X, m_AABB.Min.y, m_AABB.Min.z), m_AABB.Max));
	}
	else
	{
		m_ComponentList.push_back(comp);
	}
}

void OctreeNodeY::PushComponent(RenderComponentPtr comp)
{
	if (comp->m_AABB.Max.y < m_Y)
	{
		PushComponentToChild<OctreeNodeZ, 0>(comp, my::AABB(m_AABB.Min, Vector3(m_AABB.Max.x, m_Y, m_AABB.Max.z)));
	}
	else if (comp->m_AABB.Min.y > m_Y)
	{
		PushComponentToChild<OctreeNodeZ, 1>(comp, my::AABB(Vector3(m_AABB.Min.x, m_Y, m_AABB.Min.z), m_AABB.Max));
	}
	else
	{
		m_ComponentList.push_back(comp);
	}
}

void OctreeNodeZ::PushComponent(RenderComponentPtr comp)
{
	if (comp->m_AABB.Max.z < m_Z)
	{
		PushComponentToChild<OctreeNodeX, 0>(comp, my::AABB(m_AABB.Min, Vector3(m_AABB.Max.x, m_AABB.Max.y, m_Z)));
	}
	else if (comp->m_AABB.Min.z > m_Z)
	{
		PushComponentToChild<OctreeNodeX, 1>(comp, my::AABB(Vector3(m_AABB.Min.x, m_AABB.Min.y, m_Z), m_AABB.Max));
	}
	else
	{
		m_ComponentList.push_back(comp);
	}
}
