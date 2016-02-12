#include "StdAfx.h"
#include "Terrain.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(Terrain)

TerrainChunk::TerrainChunk(Terrain * Owner, const my::Vector2 & PosStart, const my::Vector2 & PosEnd, const my::Vector2 & TexStart, const my::Vector2 & TexEnd)
	: RenderComponent(my::AABB(Vector3(PosStart.x, -1, PosStart.y), Vector3(PosEnd.x, 1, PosEnd.y)), ComponentTypeTerrainChunk)
	, m_Owner(Owner)
	, m_PosStart(PosStart)
	, m_PosEnd(PosEnd)
	, m_TexStart(TexStart)
	, m_TexEnd(TexEnd)
{
	m_VertexElems.InsertPositionElement(0);
	WORD offset = sizeof(Vector3);
	m_VertexElems.InsertNormalElement(offset);
	offset += sizeof(Vector3);
	m_VertexElems.InsertTangentElement(offset);
	offset += sizeof(Vector3);
	m_VertexElems.InsertTexcoordElement(offset, 0);
	offset += sizeof(Vector2);
	m_VertexStride = offset;
}

TerrainChunk::~TerrainChunk(void)
{
	if (IsRequested())
	{
		ReleaseResource();
	}
}

void TerrainChunk::CreateVertices(void)
{
	_ASSERT(!m_Decl);
	IDirect3DDevice9 * pd3dDevice = D3DContext::getSingleton().m_d3dDevice;
	std::vector<D3DVERTEXELEMENT9> elems = m_VertexElems.BuildVertexElementList(0);
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	elems.push_back(ve_end);
	HRESULT hr;
	V(pd3dDevice->CreateVertexDeclaration(&elems[0], &m_Decl));

	m_vb.CreateVertexBuffer(pd3dDevice, (m_Owner->m_ChunkRows + 1) * (m_Owner->m_ChunkCols + 1) * m_VertexStride, 0, 0, D3DPOOL_MANAGED);
	VOID * pVertices = m_vb.Lock(0, 0, 0);
	if (pVertices)
	{
		for (unsigned int j = 0; j <= m_Owner->m_ChunkCols; j++)
		{
			for (unsigned int i = 0; i <= m_Owner->m_ChunkRows; i++)
			{
				unsigned char * pVertex = (unsigned char *)pVertices + (j * (m_Owner->m_ChunkRows + 1) + i) * m_VertexStride;
				Vector3 & Position = m_VertexElems.GetPosition(pVertex);
				Position.x = (i == 0 ? m_PosStart.x : (i == m_Owner->m_ChunkRows ? m_PosEnd.x : my::Lerp(m_PosStart.x, m_PosEnd.x, (float)i / m_Owner->m_ChunkRows)));
				Position.y = 0;
				Position.z = (j == 0 ? m_PosStart.y : (j == m_Owner->m_ChunkCols ? m_PosEnd.y : my::Lerp(m_PosStart.y, m_PosEnd.y, (float)j / m_Owner->m_ChunkCols)));

				Vector3 & Normal = m_VertexElems.GetNormal(pVertex);
				Normal.x = 0;
				Normal.y = 1;
				Normal.z = 0;

				Vector3 & Tangent = m_VertexElems.GetTangent(pVertex);
				Tangent.x = 1;
				Tangent.y = 0;
				Tangent.z = 0;

				Vector2 & Texcoord = m_VertexElems.GetTexcoord(pVertex, 0);
				Texcoord.x = (i == 0 ? m_TexStart.x : (i == m_Owner->m_ChunkRows ? m_TexEnd.x : my::Lerp(m_TexStart.x, m_TexEnd.x, (float)i / m_Owner->m_ChunkRows)));
				Texcoord.y = (j == 0 ? m_TexStart.y : (j == m_Owner->m_ChunkCols ? m_TexEnd.y : my::Lerp(m_TexStart.y, m_TexEnd.y, (float)j / m_Owner->m_ChunkCols)));
			}
		}
		m_vb.Unlock();
	}

	m_ib.CreateIndexBuffer(pd3dDevice, sizeof(WORD) * m_Owner->m_ChunkRows * m_Owner->m_ChunkCols * 6, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED);
	VOID * pIndices = m_ib.Lock(0, 0, 0);
	if (pIndices)
	{
		for (unsigned int j = 0; j < m_Owner->m_ChunkCols; j++)
		{
			for (unsigned int i = 0; i < m_Owner->m_ChunkRows; i++)
			{
				int index = (j * m_Owner->m_ChunkRows + i) * 6;
				*((WORD *)pIndices + index + 0) = (j + 0) * (m_Owner->m_ChunkRows + 1) + (i + 0);
				*((WORD *)pIndices + index + 1) = (j + 1) * (m_Owner->m_ChunkRows + 1) + (i + 0);
				*((WORD *)pIndices + index + 2) = (j + 1) * (m_Owner->m_ChunkRows + 1) + (i + 1);

				*((WORD *)pIndices + index + 3) = (j + 0) * (m_Owner->m_ChunkRows + 1) + (i + 0);
				*((WORD *)pIndices + index + 4) = (j + 1) * (m_Owner->m_ChunkRows + 1) + (i + 1);
				*((WORD *)pIndices + index + 5) = (j + 0) * (m_Owner->m_ChunkRows + 1) + (i + 1);
			}
		}
		m_ib.Unlock();
	}
}

void TerrainChunk::DestroyVertices(void)
{
	m_Decl.Release();
	m_vb.OnDestroyDevice();
	m_ib.OnDestroyDevice();
}

void TerrainChunk::RequestResource(void)
{
	RenderComponent::RequestResource();
	CreateVertices();
}

void TerrainChunk::ReleaseResource(void)
{
	DestroyVertices();
	RenderComponent::ReleaseResource();
}

void TerrainChunk::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	shader->SetFloat("g_Time", (float)D3DContext::getSingleton().m_fAbsoluteTime);

	shader->SetMatrix("g_World", m_Owner->m_World);

	m_Owner->m_Material->OnSetShader(shader, AttribId);
}

void TerrainChunk::AddToPipeline(RenderPipeline * pipeline, unsigned int PassMask)
{
	if (m_Owner->m_Material && (m_Owner->m_Material->m_PassMask & PassMask))
	{
		for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
		{
			if (RenderPipeline::PassTypeToMask(PassID) & (m_Owner->m_Material->m_PassMask & PassMask))
			{
				my::Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeStatic, false, m_Owner->m_Material.get(), PassID);
				if (shader)
				{
					pipeline->PushIndexedPrimitive(
						PassID, m_Decl, m_vb.m_ptr, m_ib.m_ptr, D3DPT_TRIANGLELIST, 0, 0, (m_Owner->m_ChunkRows + 1) * (m_Owner->m_ChunkCols + 1), m_VertexStride, 0, m_Owner->m_ChunkRows * m_Owner->m_ChunkCols * 2, 0, shader, this);
				}
			}
		}
	}
}

Terrain::Terrain(DWORD RowChunks, DWORD ColChunks, DWORD ChunkRows, DWORD ChunkCols, float HeightScale, float RowScale, float ColScale)
	: m_World(my::Matrix4::Translation(RowChunks * ChunkRows * RowScale * -0.5f, 0, ColChunks * ChunkCols * ColScale * -0.5f))
	, m_RowChunks(RowChunks)
	, m_ColChunks(ColChunks)
	, m_ChunkRows(ChunkRows)
	, m_ChunkCols(ChunkCols)
	, m_HeightScale(HeightScale)
	, m_RowScale(RowScale)
	, m_ColScale(ColScale)
	, m_Requested(false)
{
	CreateChunks();
}

Terrain::Terrain(void)
	: m_World(my::Matrix4::Identity())
	, m_RowChunks(0)
	, m_ColChunks(0)
	, m_ChunkRows(0)
	, m_ChunkCols(0)
	, m_HeightScale(0)
	, m_RowScale(0)
	, m_ColScale(0)
	, m_Requested(false)
{
}

Terrain::~Terrain(void)
{
	m_Material.reset();
	m_Chunks.clear();
}

void Terrain::CreateChunks(void)
{
	m_Chunks.resize(m_RowChunks * m_ColChunks);
	for (unsigned int i = 0; i < m_RowChunks; i++)
	{
		for (unsigned int j = 0; j < m_ColChunks; j++)
		{
			m_Chunks[i * m_ColChunks + j].reset(new TerrainChunk(this,
				my::Vector2((i + 0) * m_ChunkRows * m_RowScale, (j + 0) * m_ChunkCols * m_ColScale),
				my::Vector2((i + 1) * m_ChunkRows * m_RowScale, (j + 1) * m_ChunkCols * m_ColScale),
				my::Vector2(0,0), my::Vector2(1,1)));
		}
	}
}

template<>
void Terrain::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_NVP(m_World);
	ar << BOOST_SERIALIZATION_NVP(m_RowChunks);
	ar << BOOST_SERIALIZATION_NVP(m_ColChunks);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkRows);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkCols);
	ar << BOOST_SERIALIZATION_NVP(m_HeightScale);
	ar << BOOST_SERIALIZATION_NVP(m_RowScale);
	ar << BOOST_SERIALIZATION_NVP(m_ColScale);
	ar << BOOST_SERIALIZATION_NVP(m_Material);
}

template<>
void Terrain::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar, const unsigned int version)
{
	ar >> BOOST_SERIALIZATION_NVP(m_World);
	ar >> BOOST_SERIALIZATION_NVP(m_RowChunks);
	ar >> BOOST_SERIALIZATION_NVP(m_ColChunks);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkRows);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkCols);
	ar >> BOOST_SERIALIZATION_NVP(m_HeightScale);
	ar >> BOOST_SERIALIZATION_NVP(m_RowScale);
	ar >> BOOST_SERIALIZATION_NVP(m_ColScale);
	ar >> BOOST_SERIALIZATION_NVP(m_Material);
	CreateChunks();
}

void Terrain::RequestResource(void)
{
	if (!m_Requested)
	{
		m_Material->RequestResource();
		m_Requested = true;
	}
}

void Terrain::ReleaseResource(void)
{
	if (m_Requested)
	{
		m_Material->ReleaseResource();
		m_Requested = false;
	}
}
