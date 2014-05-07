#include "StdAfx.h"
#include "myOctree.h"

using namespace my;

void OctreeNodeX::PushComponent(ComponentPtr comp, float threshold)
{
	if (comp->m_aabb.Max.x < m_X + threshold)
	{
		if (!m_Childs[0])
		{
			m_Childs[0].reset(new OctreeNodeY(my::AABB(m_aabb.Min, Vector3(m_X, m_aabb.Max.y, m_aabb.Max.z))));
		}
		m_Childs[0]->PushComponent(comp, threshold);
	}
	else if (comp->m_aabb.Min.x > m_X - threshold)
	{
		if (!m_Childs[1])
		{
			m_Childs[1].reset(new OctreeNodeY(my::AABB(Vector3(m_X, m_aabb.Min.y, m_aabb.Min.z), m_aabb.Max)));
		}
		m_Childs[1]->PushComponent(comp, threshold);
	}
	else
	{
		m_ComponentList.push_back(comp);
	}
}

void OctreeNodeY::PushComponent(ComponentPtr comp, float threshold)
{
	if (comp->m_aabb.Max.y < m_Y + threshold)
	{
		if (!m_Childs[0])
		{
			m_Childs[0].reset(new OctreeNodeZ(my::AABB(m_aabb.Min, Vector3(m_aabb.Max.x, m_Y, m_aabb.Max.z))));
		}
		m_Childs[0]->PushComponent(comp, threshold);
	}
	else if (comp->m_aabb.Min.y > m_Y - threshold)
	{
		if (!m_Childs[1])
		{
			m_Childs[1].reset(new OctreeNodeZ(my::AABB(Vector3(m_aabb.Min.x, m_Y, m_aabb.Min.z), m_aabb.Max)));
		}
		m_Childs[1]->PushComponent(comp, threshold);
	}
	else
	{
		m_ComponentList.push_back(comp);
	}
}

void OctreeNodeZ::PushComponent(ComponentPtr comp, float threshold)
{
	if (comp->m_aabb.Max.z < m_Z + threshold)
	{
		if (!m_Childs[0])
		{
			m_Childs[0].reset(new OctreeNodeX(my::AABB(m_aabb.Min, Vector3(m_aabb.Max.x, m_aabb.Max.y, m_Z))));
		}
		m_Childs[0]->PushComponent(comp, threshold);
	}
	else if (comp->m_aabb.Min.z > m_Z - threshold)
	{
		if (!m_Childs[1])
		{
			m_Childs[1].reset(new OctreeNodeX(my::AABB(Vector3(m_aabb.Min.x, m_aabb.Min.y, m_Z), m_aabb.Max)));
		}
		m_Childs[1]->PushComponent(comp, threshold);
	}
	else
	{
		m_ComponentList.push_back(comp);
	}
}
