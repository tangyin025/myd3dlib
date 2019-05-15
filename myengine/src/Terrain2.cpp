#include "Terrain2.h"

using namespace my;

TerrainNode::TerrainNode(const my::AABB & aabb)
	: m_aabb(aabb)
{
	Vector3 extent = m_aabb.Extent();
	if (extent.x > (2 - EPSILON_E3) && extent.y > (2 - EPSILON_E3))
	{
		Vector3 center = m_aabb.Center();
		m_Childs[0].reset(new TerrainNode(AABB(m_aabb.m_min.x, m_aabb.m_min.y, m_aabb.m_min.z, center.x, center.y, m_aabb.m_max.z)));
		m_Childs[1].reset(new TerrainNode(AABB(center.x, m_aabb.m_min.y, m_aabb.m_min.z, m_aabb.m_max.x, center.y, m_aabb.m_max.z)));
		m_Childs[2].reset(new TerrainNode(AABB(m_aabb.m_min.x, center.y, m_aabb.m_min.z, center.x, m_aabb.m_max.y, m_aabb.m_max.z)));
		m_Childs[3].reset(new TerrainNode(AABB(center.x, center.y, m_aabb.m_min.z, m_aabb.m_max.x, m_aabb.m_max.y, m_aabb.m_max.z)));
	}
}

bool TerrainNode::OnQuery(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	return true;
}

void TerrainNode::QueryAll(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	if (!OnQuery(frustum, pipeline, PassMask, ViewPos, TargetPos))
	{
		ChildArray::iterator node_iter = m_Childs.begin();
		for (; node_iter != m_Childs.end(); node_iter++)
		{
			(*node_iter)->QueryAll(frustum, pipeline, PassMask, ViewPos, TargetPos);
		}
	}
}

void TerrainNode::Query(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	if (!OnQuery(frustum, pipeline, PassMask, ViewPos, TargetPos))
	{
		ChildArray::iterator node_iter = m_Childs.begin();
		for (; node_iter != m_Childs.end(); node_iter++)
		{
			switch (IntersectionTests::IntersectAABBAndFrustum((*node_iter)->m_aabb, frustum))
			{
			case IntersectionTests::IntersectionTypeInside:
				(*node_iter)->QueryAll(frustum, pipeline, PassMask, ViewPos, TargetPos);
				break;

			case IntersectionTests::IntersectionTypeIntersect:
				(*node_iter)->Query(frustum, pipeline, PassMask, ViewPos, TargetPos);
				break;
			}
		}
	}
}

void Terrain2::RequestResource(void)
{
	Component::RequestResource();
}

void Terrain2::ReleaseResource(void)
{
	Component::ReleaseResource();
}

void Terrain2::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{

}

void Terrain2::OnShaderChanged(void)
{

}

void Terrain2::Update(float fElapsedTime)
{

}

void Terrain2::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	TerrainNode::Query(frustum, pipeline, PassMask, ViewPos, TargetPos);
}

void Terrain2::ClearShape(void)
{
	Component::ClearShape();
}
