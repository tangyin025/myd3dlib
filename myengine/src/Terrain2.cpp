#include "Terrain2.h"
#include "myDxutApp.h"
#include "Actor.h"
#include "RenderPipeline.h"
#include <boost/multi_array.hpp>

using namespace my;

TerrainNode::TerrainNode(Terrain2 * Owner, int iStart, int jStart, int NodeSize)
	: m_Owner(Owner)
	, m_iStart(iStart)
	, m_jStart(jStart)
	, m_NodeSize(NodeSize)
{
	m_aabb = my::AABB(iStart, m_Owner->m_aabb.m_min.y, jStart, iStart + NodeSize, m_Owner->m_aabb.m_max.y, jStart + NodeSize);
}

void TerrainNode::Build(void)
{
	//const Vector3 extent = m_aabb.Extent();
	//if (extent.x > (2 - EPSILON_E3) && extent.y > (2 - EPSILON_E3))
	//{
	//	const Vector3 center = m_aabb.Center();
	//	m_Childs[0].reset(new TerrainNode(AABB(m_aabb.m_min.x, m_aabb.m_min.y, m_aabb.m_min.z, center.x, center.y, m_aabb.m_max.z)));
	//	m_Childs[1].reset(new TerrainNode(AABB(center.x, m_aabb.m_min.y, m_aabb.m_min.z, m_aabb.m_max.x, center.y, m_aabb.m_max.z)));
	//	m_Childs[2].reset(new TerrainNode(AABB(m_aabb.m_min.x, center.y, m_aabb.m_min.z, center.x, m_aabb.m_max.y, m_aabb.m_max.z)));
	//	m_Childs[3].reset(new TerrainNode(AABB(center.x, center.y, m_aabb.m_min.z, m_aabb.m_max.x, m_aabb.m_max.y, m_aabb.m_max.z)));
	//}
}

bool TerrainNode::OnQuery(const my::Frustum & frustum, DWORD * pib, int & nib, unsigned int PassMask, const my::Vector3 & ViewPos, const my::Vector3 & TargetPos)
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
	, TerrainNode(this, 0, 0, Size)
	, m_Size(Size)
	, m_VertexStride(0)
	, m_HeightScale(1)
	, technique_RenderScene(NULL)
	, handle_World(NULL)
	, handle_HeightScale(NULL)
	, handle_HeightTexSize(NULL)
	, handle_ChunkId(NULL)
	, handle_ChunkSize(NULL)
	, handle_HeightTexture(NULL)
{
	CreateHeightMap();
	UpdateHeightMapNormal();
	CreateElements();
}

Terrain2::Terrain2(void)
	: Component(ComponentTypeTerrain2)
	, TerrainNode(this, 0, 0, 0)
	, m_Size(0)
	, m_VertexStride(0)
	, m_HeightScale(1)
	, technique_RenderScene(NULL)
	, handle_World(NULL)
	, handle_HeightScale(NULL)
	, handle_HeightTexSize(NULL)
	, handle_ChunkId(NULL)
	, handle_ChunkSize(NULL)
	, handle_HeightTexture(NULL)
{
}

Terrain2::~Terrain2(void)
{
	m_HeightMap.OnDestroyDevice();
	m_Decl.Release();
	m_vb.OnDestroyDevice();
	m_ib.OnDestroyDevice();
}

void Terrain2::CreateHeightMap(void)
{
	m_HeightMap.OnDestroyDevice();
	m_HeightMap.CreateTexture(m_Size + 1, m_Size + 1, 1, 0, D3DFMT_A8R8G8B8);
}

void Terrain2::UpdateVertices(void)
{
	VOID * pVertices = m_vb.Lock(0, 0, 0);
	if (pVertices)
	{
		for (unsigned int i = 0; i < (m_Size + 1); i++)
		{
			for (unsigned int j = 0; j < (m_Size + 1); j++)
			{
				unsigned char * pVertex = (unsigned char *)pVertices + (i * (m_Size + 1) + j) * m_VertexStride;
				short * pIndices = m_VertexElems.GetVertexValue<short>(pVertex, D3DDECLUSAGE_TEXCOORD, 0);
				pIndices[0] = i;
				pIndices[1] = j;
				pIndices[2] = 0;
				pIndices[3] = 0;
			}
		}
		m_vb.Unlock();
	}
}

D3DCOLOR Terrain2::GetSampleValue(void * pBits, int pitch, int i, int j) const
{
	i = Clamp<int>(i, 0, m_Size);
	j = Clamp<int>(j, 0, m_Size);
	D3DCOLOR * Bits = (D3DCOLOR *)((unsigned char *)pBits + i * pitch + j * sizeof(D3DCOLOR));
	return *Bits;
}

float Terrain2::GetSampleHeight(void * pBits, int pitch, int i, int j) const
{
	return m_HeightScale * GetCValue(GetSampleValue(pBits, pitch, i, j));
}

my::Vector3 Terrain2::GetSamplePos(void * pBits, int pitch, int i, int j) const
{
	return Vector3((float)j, GetSampleHeight(pBits, pitch, i, j), (float)i);
}

float Terrain2::GetPosHeight(void * pBits, int pitch, float x, float z) const
{
	int x0 = (int)floor(x);
	int z0 = (int)floor(z);
	Vector3 v0 = GetSamplePos(pBits, pitch, z0, x0);
	Vector3 v1 = GetSamplePos(pBits, pitch, z0 + 1, x0);
	Vector3 v2 = GetSamplePos(pBits, pitch, z0, x0 + 1);
	Vector3 v3 = GetSamplePos(pBits, pitch, z0 + 1, x0 + 1);
	Ray ray(Vector3(x, m_aabb.m_max.y, z), Vector3(0, -1, 0));
	RayResult result = CollisionDetector::rayAndTriangle(ray.p, ray.d, v0, v1, v2);
	if (result.first)
	{
		return ray.p.y - result.second;
	}

	result = CollisionDetector::rayAndTriangle(ray.p, ray.d, v2, v1, v3);
	if (result.first)
	{
		return ray.p.y - result.second;
	}

	if (x < x0 + 0.5f)
	{
		if (z < z0 + 0.5f)
		{
			return GetSampleHeight(pBits, pitch, z0, x0);
		}
		return GetSampleHeight(pBits, pitch, z0 + 1, x0);
	}

	if (z < z0 + 0.5f)
	{
		return GetSampleHeight(pBits, pitch, z0, x0 + 1);
	}
	return GetSampleHeight(pBits, pitch, z0 + 1, x0 + 1);
}

void Terrain2::CreateElements(void)
{
	m_VertexElems.InsertVertexElement(0, D3DDECLTYPE_SHORT4, D3DDECLUSAGE_TEXCOORD, 0, D3DDECLMETHOD_DEFAULT);
	DWORD offset = sizeof(short) * 4;
	m_VertexStride = offset;
}

void Terrain2::UpdateHeightMapNormal(void)
{
	D3DLOCKED_RECT lrc = m_HeightMap.LockRect(NULL, D3DLOCK_READONLY, 0);
	for (int i = 0; i < m_Size + 1; i++)
	{
		for (int j = 0; j < m_Size + 1; j++)
		{
			D3DCOLOR * Bits = (D3DCOLOR *)((unsigned char *)lrc.pBits + i * lrc.Pitch + j * sizeof(D3DCOLOR));
			Vector3 Pos = GetSamplePos(lrc.pBits, lrc.Pitch, i, j);
			const Vector3 Dirs[4] = {
				GetSamplePos(lrc.pBits, lrc.Pitch, i - 1, j) - Pos,
				GetSamplePos(lrc.pBits, lrc.Pitch, i, j - 1) - Pos,
				GetSamplePos(lrc.pBits, lrc.Pitch, i + 1, j) - Pos,
				GetSamplePos(lrc.pBits, lrc.Pitch, i, j + 1) - Pos,
			};
			const Vector3 Nors[4] = {
				Dirs[0].cross(Dirs[1]).normalize(),
				Dirs[1].cross(Dirs[2]).normalize(),
				Dirs[2].cross(Dirs[3]).normalize(),
				Dirs[3].cross(Dirs[0]).normalize(),
			};
			Vector3 Normal = (Nors[0] + Nors[1] + Nors[2] + Nors[3]).normalize();
			Normal = (Normal + 1.0f) / 2.0f * 255.0f;
			*Bits = D3DCOLOR_ARGB(GetCValue(*Bits), (unsigned char)Normal.x, (unsigned char)Normal.y, (unsigned char)Normal.z);
		}
	}
	m_HeightMap.UnlockRect(0);
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

	m_Material->RequestResource();
}

void Terrain2::ReleaseResource(void)
{
	m_Decl.Release();
	m_vb.OnDestroyDevice();
	m_ib.OnDestroyDevice();
	m_Material->ReleaseResource();
	Component::ReleaseResource();
}

void Terrain2::OnSetShader(IDirect3DDevice9 * pd3dDevice, my::Effect * shader, LPARAM lparam)
{
	_ASSERT(m_Actor);
	shader->SetTechnique(technique_RenderScene);
	shader->SetMatrix(handle_World, m_Actor->m_World);
	shader->SetFloat(handle_HeightScale, m_HeightScale);
	shader->SetVector(handle_HeightTexSize, Vector2(m_Size + 1, m_Size + 1));
	int ChunkId[2] = { GetRValue(lparam), GetGValue(lparam) };
	shader->SetIntArray(handle_ChunkId, ChunkId, 2);
	shader->SetInt(handle_ChunkSize, m_Size);
	shader->SetTexture(handle_HeightTexture, &m_HeightMap);
}

void Terrain2::OnShaderChanged(void)
{
	technique_RenderScene = NULL;
	handle_World = NULL;
	handle_HeightScale = NULL;
	handle_HeightTexSize = NULL;
	handle_ChunkId = NULL;
	handle_ChunkSize = NULL;
	handle_HeightTexture = NULL;
	m_Material->ParseShaderParameters();
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
		if (nib > 0 && m_Material && (m_Material->m_PassMask & PassMask))
		{
			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
				{
					Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeTerrain, NULL, m_Material->m_Shader.c_str(), PassID);
					if (shader)
					{
						if (!technique_RenderScene)
						{
							BOOST_VERIFY(technique_RenderScene = shader->GetTechniqueByName("RenderScene"));
							BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
							BOOST_VERIFY(handle_HeightScale = shader->GetParameterByName(NULL, "g_HeightScale"));
							BOOST_VERIFY(handle_HeightTexSize = shader->GetParameterByName(NULL, "g_HeightTexSize"));
							BOOST_VERIFY(handle_ChunkId = shader->GetParameterByName(NULL, "g_ChunkId"));
							BOOST_VERIFY(handle_ChunkSize = shader->GetParameterByName(NULL, "g_ChunkSize"));
							BOOST_VERIFY(handle_HeightTexture = shader->GetParameterByName(NULL, "g_HeightTexture"));
						}

						pipeline->PushIndexedPrimitive(PassID, m_Decl, m_vb.m_ptr, m_ib.m_ptr, D3DPT_TRIANGLELIST,
							0, 0, (m_Size + 1) * (m_Size + 1), m_VertexStride, 0, nib / 3, shader, this, m_Material.get(), 0);
					}
				}
			}
		}
	}
}

void Terrain2::ClearShape(void)
{
	Component::ClearShape();
}
