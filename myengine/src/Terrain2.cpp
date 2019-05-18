#include "Terrain2.h"
#include "myDxutApp.h"
#include "Actor.h"
#include "RenderPipeline.h"
#include <boost/multi_array.hpp>

using namespace my;

TerrainNode::TerrainNode(const my::AABB & aabb)
	: m_aabb(aabb)
{
}

void TerrainNode::Build(void)
{
	const Vector3 extent = m_aabb.Extent();
	if (extent.x > (2 - EPSILON_E3) && extent.y > (2 - EPSILON_E3))
	{
		const Vector3 center = m_aabb.Center();
		m_Childs[0].reset(new TerrainNode(AABB(m_aabb.m_min.x, m_aabb.m_min.y, m_aabb.m_min.z, center.x, center.y, m_aabb.m_max.z)));
		m_Childs[1].reset(new TerrainNode(AABB(center.x, m_aabb.m_min.y, m_aabb.m_min.z, m_aabb.m_max.x, center.y, m_aabb.m_max.z)));
		m_Childs[2].reset(new TerrainNode(AABB(m_aabb.m_min.x, center.y, m_aabb.m_min.z, center.x, m_aabb.m_max.y, m_aabb.m_max.z)));
		m_Childs[3].reset(new TerrainNode(AABB(center.x, center.y, m_aabb.m_min.z, m_aabb.m_max.x, m_aabb.m_max.y, m_aabb.m_max.z)));
	}
}

bool TerrainNode::OnQuery(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	return true;
}

void TerrainNode::QueryAll(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	if (!OnQuery(frustum, pib, nib, PassMask, ViewPos, TargetPos))
	{
		ChildArray::iterator node_iter = m_Childs.begin();
		for (; node_iter != m_Childs.end(); node_iter++)
		{
			(*node_iter)->QueryAll(frustum, pib, nib, PassMask, ViewPos, TargetPos);
		}
	}
}

void TerrainNode::Query(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	if (!OnQuery(frustum, pib, nib, PassMask, ViewPos, TargetPos))
	{
		ChildArray::iterator node_iter = m_Childs.begin();
		for (; node_iter != m_Childs.end(); node_iter++)
		{
			switch (IntersectionTests::IntersectAABBAndFrustum((*node_iter)->m_aabb, frustum))
			{
			case IntersectionTests::IntersectionTypeInside:
				(*node_iter)->QueryAll(frustum, pib, nib, PassMask, ViewPos, TargetPos);
				break;

			case IntersectionTests::IntersectionTypeIntersect:
				(*node_iter)->Query(frustum, pib, nib, PassMask, ViewPos, TargetPos);
				break;
			}
		}
	}
}

Terrain2::Terrain2(int Size)
	: Component(ComponentTypeTerrain2)
	, TerrainNode(my::AABB(0, (int)Size))
	, m_Size(Size)
{
	m_VertexElems.InsertVertexElement(0, D3DDECLTYPE_SHORT2, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLMETHOD_DEFAULT);
	DWORD offset = sizeof(short) * 2;
	m_VertexStride = offset;
}

Terrain2::Terrain2(void)
	: Component(ComponentTypeTerrain2)
	, TerrainNode(my::AABB::Invalid())
	, m_Size(0)
	, m_VertexStride(0)
{
}

void Terrain2::UpdateVertices(void)
{
	VOID * pVertices = m_vb.Lock(0, 0, 0);
	_ASSERT(m_VertexStride == sizeof(DWORD));
	boost::multi_array_ref<DWORD, 2> verts((DWORD *)pVertices, boost::extents[m_Size + 1][m_Size + 1]);
	if (pVertices)
	{
		for (int i = 0; i < m_Size + 1; i++)
		{
			for (int j = 0; j < m_Size + 1; j++)
			{
				verts[i][j] = MAKELONG(j, i);
			}
		}
		m_vb.Unlock();
	}
}

void Terrain2::RequestResource(void)
{
	_ASSERT((m_Size & m_Size - 1) == 0);

	Component::RequestResource();

	if (!m_Decl)
	{
		IDirect3DDevice9 * pd3dDevice = D3DContext::getSingleton().m_d3dDevice;
		std::vector<D3DVERTEXELEMENT9> elems = m_VertexElems.BuildVertexElementList(0);
		D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
		elems.push_back(ve_end);
		HRESULT hr;
		V(pd3dDevice->CreateVertexDeclaration(&elems[0], &m_Decl));
	}

	if (!m_vb.m_ptr)
	{
		m_vb.CreateVertexBuffer((m_Size + 1) * (m_Size + 1) * m_VertexStride, 0, 0, D3DPOOL_MANAGED);
		UpdateVertices();
	}

	if (!m_ib.m_ptr)
	{
		m_ib.CreateIndexBuffer((m_Size / 2) * (m_Size / 2) * 12, 0, D3DFMT_INDEX32, D3DPOOL_MANAGED);
	}
}

void Terrain2::ReleaseResource(void)
{
	m_Decl.Release();
	m_vb.OnDestroyDevice();
	m_ib.OnDestroyDevice();
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
	if (m_vb.m_ptr)
	{
		// ! do not use m_World for level offset
		Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
		Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
		DWORD * pib = (DWORD *)m_ib.Lock(0, 0, 0);
		int nib = 0;
		TerrainNode::Query(LocalFrustum, pib, nib, PassMask, LocalViewPos, TargetPos);
		m_ib.Unlock();
		if (nib > 0)
		{
			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				pipeline->PushIndexedPrimitive(PassID, m_Decl, m_vb.m_ptr, m_ib.m_ptr, D3DPT_TRIANGLELIST,
					0, 0, (m_Size + 1) * (m_Size + 1), m_VertexStride, 0, nib / 3, NULL, this, NULL, 0);
			}
		}
	}
}

void Terrain2::ClearShape(void)
{
	Component::ClearShape();
}
