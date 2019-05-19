#include "QuadTerrain.h"
#include "myDxutApp.h"
#include "Actor.h"
#include "RenderPipeline.h"
#include <boost/multi_array.hpp>

using namespace my;

QuadTerrainNode::QuadTerrainNode(QuadTerrain * Owner, int iStart, int jStart, int NodeSize)
	: m_Owner(Owner)
	, m_iStart(iStart)
	, m_jStart(jStart)
	, m_NodeSize(NodeSize)
{
	m_aabb = my::AABB(iStart, m_Owner->m_aabb.m_min.y, jStart, iStart + NodeSize, m_Owner->m_aabb.m_max.y, jStart + NodeSize);
}

void QuadTerrainNode::Build(void)
{
	//const Vector3 extent = m_aabb.Extent();
	//if (extent.x > (2 - EPSILON_E3) && extent.y > (2 - EPSILON_E3))
	//{
	//	const Vector3 center = m_aabb.Center();
	//	m_Childs[0].reset(new QuadTerrainNode(AABB(m_aabb.m_min.x, m_aabb.m_min.y, m_aabb.m_min.z, center.x, center.y, m_aabb.m_max.z)));
	//	m_Childs[1].reset(new QuadTerrainNode(AABB(center.x, m_aabb.m_min.y, m_aabb.m_min.z, m_aabb.m_max.x, center.y, m_aabb.m_max.z)));
	//	m_Childs[2].reset(new QuadTerrainNode(AABB(m_aabb.m_min.x, center.y, m_aabb.m_min.z, center.x, m_aabb.m_max.y, m_aabb.m_max.z)));
	//	m_Childs[3].reset(new QuadTerrainNode(AABB(center.x, center.y, m_aabb.m_min.z, m_aabb.m_max.x, m_aabb.m_max.y, m_aabb.m_max.z)));
	//}
}

bool QuadTerrainNode::OnQuery(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	const int ibuff[] =
	{
		m_iStart * (m_Owner->m_Size + 1) + m_jStart,
		m_iStart * (m_Owner->m_Size + 1) + m_jStart + m_NodeSize / 2,
		m_iStart * (m_Owner->m_Size + 1) + m_jStart + m_NodeSize,

		(m_iStart + m_NodeSize / 2) * (m_Owner->m_Size + 1) + m_jStart,
		(m_iStart + m_NodeSize / 2) * (m_Owner->m_Size + 1) + m_jStart + m_NodeSize / 2,
		(m_iStart + m_NodeSize / 2) * (m_Owner->m_Size + 1) + m_jStart + m_NodeSize,

		(m_iStart + m_NodeSize) * (m_Owner->m_Size + 1) + m_jStart,
		(m_iStart + m_NodeSize) * (m_Owner->m_Size + 1) + m_jStart + m_NodeSize / 2,
		(m_iStart + m_NodeSize) * (m_Owner->m_Size + 1) + m_jStart + m_NodeSize,
	};

	pib[nib++] = ibuff[0]; pib[nib++] = ibuff[4]; pib[nib++] = ibuff[1];
	pib[nib++] = ibuff[1]; pib[nib++] = ibuff[4]; pib[nib++] = ibuff[2];
	pib[nib++] = ibuff[2]; pib[nib++] = ibuff[4]; pib[nib++] = ibuff[5];
	pib[nib++] = ibuff[5]; pib[nib++] = ibuff[4]; pib[nib++] = ibuff[8];
	pib[nib++] = ibuff[8]; pib[nib++] = ibuff[4]; pib[nib++] = ibuff[7];
	pib[nib++] = ibuff[7]; pib[nib++] = ibuff[4]; pib[nib++] = ibuff[6];
	pib[nib++] = ibuff[6]; pib[nib++] = ibuff[4]; pib[nib++] = ibuff[3];
	pib[nib++] = ibuff[3]; pib[nib++] = ibuff[4]; pib[nib++] = ibuff[0];
	return true;
}

void QuadTerrainNode::QueryAll(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
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

void QuadTerrainNode::Query(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
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

QuadTerrain::QuadTerrain(int Size)
	: Component(ComponentTypeTerrain2)
	, QuadTerrainNode(this, 0, 0, Size)
	, m_Size(Size)
	, m_VertexStride(0)
	, technique_RenderScene(NULL)
	, handle_World(NULL)
{
	CreateElements();
}

QuadTerrain::QuadTerrain(void)
	: Component(ComponentTypeTerrain2)
	, QuadTerrainNode(this, 0, 0, 0)
	, m_Size(0)
	, m_VertexStride(0)
	, technique_RenderScene(NULL)
	, handle_World(NULL)
{
}

QuadTerrain::~QuadTerrain(void)
{
	m_Decl.Release();
	m_vb.OnDestroyDevice();
	m_ib.OnDestroyDevice();
}

void QuadTerrain::CreateElements(void)
{
	m_VertexElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);
	m_VertexElems.InsertNormalElement(offset);
	offset += sizeof(Vector3);
	m_VertexElems.InsertTangentElement(offset);
	offset += sizeof(Vector3);
	m_VertexElems.InsertTexcoordElement(offset);
	offset += sizeof(Vector2);
	m_VertexStride = offset;
}

void QuadTerrain::UpdateVertices(void)
{
	VOID * pVertices = m_vb.Lock(0, 0, 0);
	if (pVertices)
	{
		for (unsigned int i = 0; i < (m_Size + 1); i++)
		{
			for (unsigned int j = 0; j < (m_Size + 1); j++)
			{
				unsigned char * pVertex = (unsigned char *)pVertices + (i * (m_Size + 1) + j) * m_VertexStride;
				m_VertexElems.SetPosition(pVertex, Vector3(j, 0, i));
				m_VertexElems.SetNormal(pVertex, Vector3(0, 1, 0));
				m_VertexElems.SetTangent(pVertex, Vector3(1, 0, 0));
				m_VertexElems.SetTexcoord(pVertex, Vector2((float)j / m_Size, (float)i / m_Size));
			}
		}
		m_vb.Unlock();
	}
}

void QuadTerrain::RequestResource(void)
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

	m_Material->RequestResource();
}

void QuadTerrain::ReleaseResource(void)
{
	m_Decl.Release();
	m_vb.OnDestroyDevice();
	m_ib.OnDestroyDevice();
	m_Material->ReleaseResource();
	Component::ReleaseResource();
}

void QuadTerrain::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);
	shader->SetTechnique(technique_RenderScene);
	shader->SetMatrix(handle_World, m_Actor->m_World);
}

void QuadTerrain::OnShaderChanged(void)
{
	technique_RenderScene = NULL;
	handle_World = NULL;
	m_Material->ParseShaderParameters();
}

void QuadTerrain::Update(float fElapsedTime)
{

}

void QuadTerrain::AddToPipeline(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
{
	if (m_vb.m_ptr)
	{
		// ! do not use m_World for level offset
		Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
		Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
		DWORD * pib = (DWORD *)m_ib.Lock(0, 0, 0);
		int nib = 0;
		QuadTerrainNode::Query(LocalFrustum, pib, nib, PassMask, LocalViewPos, TargetPos);
		m_ib.Unlock();
		if (nib > 0 && m_Material && (m_Material->m_PassMask & PassMask))
		{
			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
				{
					Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeMesh, NULL, m_Material->m_Shader.c_str(), PassID);
					if (shader)
					{
						if (!technique_RenderScene)
						{
							BOOST_VERIFY(technique_RenderScene = shader->GetTechniqueByName("RenderScene"));
							BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
						}

						pipeline->PushIndexedPrimitive(PassID, m_Decl, m_vb.m_ptr, m_ib.m_ptr, D3DPT_TRIANGLELIST,
							0, 0, (m_Size + 1) * (m_Size + 1), m_VertexStride, 0, nib / 3, shader, this, m_Material.get(), 0);
					}
				}
			}
		}
	}
}

void QuadTerrain::ClearShape(void)
{
	Component::ClearShape();
}
