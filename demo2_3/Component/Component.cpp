#include "StdAfx.h"
#include "Component.h"
#include "Animator.h"
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

using namespace my;

BOOST_CLASS_EXPORT(Material::ParameterValue)

BOOST_CLASS_EXPORT(Material::ParameterValueTexture)

BOOST_CLASS_EXPORT(Component)

BOOST_CLASS_EXPORT(RenderComponent)

BOOST_CLASS_EXPORT(MeshComponent)

BOOST_CLASS_EXPORT(EmitterComponent)

Material::Material(void)
	: m_PassMask(RenderPipeline::PassMaskNone)
{
}

Material::~Material(void)
{
}

void Material::ParameterValueTexture::OnSetShader(my::Effect * shader, DWORD AttribId, const char * name)
{
	shader->SetTexture(name, m_Res.get());
}

void Material::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	ParameterList::iterator param_iter = m_Params.begin();
	for (; param_iter != m_Params.end(); param_iter++)
	{
		param_iter->second->OnSetShader(shader, AttribId, param_iter->first.c_str());
	}
}

void Material::RequestResource(void)
{
	ParameterList::iterator param_iter = m_Params.begin();
	for (; param_iter != m_Params.end(); param_iter++)
	{
		switch(param_iter->second->m_Type)
		{
		case ParameterValue::ParameterValueTypeTexture:
			boost::dynamic_pointer_cast<ParameterValueTexture>(param_iter->second)->RequestResource();
		}
	}
}

void Material::ReleaseResource(void)
{
	ParameterList::iterator param_iter = m_Params.begin();
	for (; param_iter != m_Params.end(); param_iter++)
	{
		switch(param_iter->second->m_Type)
		{
		case ParameterValue::ParameterValueTypeTexture:
			boost::dynamic_pointer_cast<ParameterValueTexture>(param_iter->second)->ReleaseResource();
		}
	}
}

void MeshComponent::RequestResource(void)
{
	RenderComponent::RequestResource();

	m_MeshRes.RequestResource();
	MaterialPtrList::iterator mat_iter = m_MaterialList.begin();
	for (; mat_iter != m_MaterialList.end(); mat_iter++)
	{
		(*mat_iter)->RequestResource();
	}

	if (m_Animator)
	{
		m_Animator->RequestResource();
	}
}

void MeshComponent::ReleaseResource(void)
{
	RenderComponent::ReleaseResource();

	m_MeshRes.ReleaseResource();
	MaterialPtrList::iterator mat_iter = m_MaterialList.begin();
	for (; mat_iter != m_MaterialList.end(); mat_iter++)
	{
		(*mat_iter)->ReleaseResource();
	}

	if (m_Animator)
	{
		m_Animator->ReleaseResource();
	}
}

void MeshComponent::Update(float fElapsedTime)
{
	if (m_Animator)
	{
		m_Animator->Update(fElapsedTime);
	}
}

void MeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(AttribId < m_MaterialList.size());

	shader->SetMatrix("g_World", m_World);

	if (m_Animator && !m_Animator->m_DualQuats.empty())
	{
		shader->SetMatrixArray("g_dualquat", &m_Animator->m_DualQuats[0], m_Animator->m_DualQuats.size());
	}

	m_MaterialList[AttribId]->OnSetShader(shader, AttribId);
}

void MeshComponent::AddToPipeline(RenderPipeline * pipeline, unsigned int PassMask)
{
	if (m_MeshRes.m_Res)
	{
		for (DWORD i = 0; i < m_MaterialList.size(); i++)
		{
			if (m_MaterialList[i] && (m_MaterialList[i]->m_PassMask & PassMask))
			{
				for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
				{
					if (RenderPipeline::PassTypeToMask(PassID) & (m_MaterialList[i]->m_PassMask & PassMask))
					{
						my::Effect * shader = pipeline->QueryShader(m_Animator ? RenderPipeline::MeshTypeAnimation : RenderPipeline::MeshTypeStatic, m_bInstance, m_MaterialList[i].get(), PassID);
						if (shader)
						{
							if (m_bInstance)
							{
								pipeline->PushMeshInstance(PassID, m_MeshRes.m_Res.get(), i, m_World, shader, this);
							}
							else
							{
								pipeline->PushMesh(PassID, m_MeshRes.m_Res.get(), i, shader, this);
							}
						}
					}
				}
			}
		}
	}
}

void EmitterComponent::RequestResource(void)
{
	RenderComponent::RequestResource();
	if (m_Material)
	{
		m_Material->RequestResource();
	}
}

void EmitterComponent::ReleaseResource(void)
{
	if (m_Material)
	{
		m_Material->ReleaseResource();
	}
	RenderComponent::ReleaseResource();
}

void EmitterComponent::Update(float fElapsedTime)
{
	if (m_Emitter)
	{
		m_Emitter->Update(fElapsedTime);
	}
}

void EmitterComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(0 == AttribId);

	shader->SetMatrix("g_World", m_World);

	if (m_Material)
	{
		m_Material->OnSetShader(shader, AttribId);
	}
}

void EmitterComponent::AddToPipeline(RenderPipeline * pipeline, unsigned int PassMask)
{
	if (m_Material && m_Emitter && (m_Material->m_PassMask & PassMask))
	{
		for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
		{
			if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
			{
				my::Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeParticle, false, m_Material.get(), PassID);
				if (shader)
				{
					pipeline->PushEmitter(PassID, m_Emitter.get(), 0, shader, this);
				}
			}
		}
	}
}

TerrainComponent::TerrainComponent(const my::AABB & aabb, const my::Matrix4 & World)
	: RenderComponent(aabb, World, ComponentTypeTerrain)
	, m_XDivision(20)
	, m_ZDivision(20)
	, m_PosStart(-5,-5)
	, m_PosEnd(5,5)
	, m_TexStart(0,0)
	, m_TexEnd(1,1)
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

TerrainComponent::TerrainComponent(void)
	: RenderComponent(my::AABB(-FLT_MAX,FLT_MAX), my::Matrix4::Identity(), ComponentTypeTerrain)
	, m_XDivision(20)
	, m_ZDivision(20)
	, m_PosStart(-5,-5)
	, m_PosEnd(5,5)
	, m_TexStart(0,0)
	, m_TexEnd(1,1)
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

void TerrainComponent::OnResetDevice(void)
{
	_ASSERT(!m_Decl);
	IDirect3DDevice9 * pd3dDevice = D3DContext::getSingleton().m_d3dDevice;
	std::vector<D3DVERTEXELEMENT9> elems = m_VertexElems.BuildVertexElementList(0);
	D3DVERTEXELEMENT9 ve_end = D3DDECL_END();
	elems.push_back(ve_end);
	HRESULT hr;
	V(pd3dDevice->CreateVertexDeclaration(&elems[0], &m_Decl));

	m_vb.CreateVertexBuffer(pd3dDevice, (m_XDivision + 1) * (m_ZDivision + 1) * m_VertexStride, 0, 0, D3DPOOL_DEFAULT);
	VOID * pVertices = m_vb.Lock(0, 0, 0);
	if (pVertices)
	{
		for (unsigned int j = 0; j <= m_ZDivision; j++)
		{
			for (unsigned int i = 0; i <= m_XDivision; i++)
			{
				unsigned char * pVertex = (unsigned char *)pVertices + (j * (m_XDivision + 1) + i) * m_VertexStride;
				Vector3 & Position = m_VertexElems.GetPosition(pVertex);
				Position.x = (i == 0 ? m_PosStart.x : (i == m_XDivision ? m_PosEnd.x : my::Lerp(m_PosStart.x, m_PosEnd.x, (float)i / m_XDivision)));
				Position.y = 0;
				Position.z = (j == 0 ? m_PosStart.y : (j == m_ZDivision ? m_PosEnd.y : my::Lerp(m_PosStart.y, m_PosEnd.y, (float)j / m_ZDivision)));

				Vector3 & Normal = m_VertexElems.GetNormal(pVertex);
				Normal.x = 0;
				Normal.y = 1;
				Normal.z = 0;

				Vector3 & Tangent = m_VertexElems.GetTangent(pVertex);
				Tangent.x = 1;
				Tangent.y = 0;
				Tangent.z = 0;

				Vector2 & Texcoord = m_VertexElems.GetTexcoord(pVertex, 0);
				Texcoord.x = (i == 0 ? m_TexStart.x : (i == m_XDivision ? m_TexEnd.x : my::Lerp(m_TexStart.x, m_TexEnd.x, (float)i / m_XDivision)));
				Texcoord.y = (j == 0 ? m_TexStart.y : (j == m_ZDivision ? m_TexEnd.y : my::Lerp(m_TexStart.y, m_TexEnd.y, (float)j / m_ZDivision)));
			}
		}
		m_vb.Unlock();
	}

	m_ib.CreateIndexBuffer(pd3dDevice, sizeof(WORD) * m_XDivision * m_ZDivision * 6, D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_DEFAULT);
	VOID * pIndices = m_ib.Lock(0, 0, 0);
	if (pIndices)
	{
		for (unsigned int j = 0; j < m_ZDivision; j++)
		{
			for (unsigned int i = 0; i < m_XDivision; i++)
			{
				int index = (j * m_XDivision + i) * 6;
				*((WORD *)pIndices + index + 0) = (j + 0) * (m_XDivision + 1) + (i + 0);
				*((WORD *)pIndices + index + 1) = (j + 1) * (m_XDivision + 1) + (i + 0);
				*((WORD *)pIndices + index + 2) = (j + 1) * (m_XDivision + 1) + (i + 1);

				*((WORD *)pIndices + index + 3) = (j + 0) * (m_XDivision + 1) + (i + 0);
				*((WORD *)pIndices + index + 4) = (j + 1) * (m_XDivision + 1) + (i + 1);
				*((WORD *)pIndices + index + 5) = (j + 0) * (m_XDivision + 1) + (i + 1);
			}
		}
		m_ib.Unlock();
	}
}

void TerrainComponent::OnLostDevice(void)
{
	m_Decl.Release();
	m_vb.OnDestroyDevice();
	m_ib.OnDestroyDevice();
}

void TerrainComponent::OnDestroyDevice(void)
{
}

void TerrainComponent::RequestResource(void)
{
	RenderComponent::RequestResource();
	if (m_Material)
	{
		m_Material->RequestResource();
	}
}

void TerrainComponent::ReleaseResource(void)
{
	if (m_Material)
	{
		m_Material->ReleaseResource();
	}
	RenderComponent::ReleaseResource();
}

void TerrainComponent::Update(float fElapsedTime)
{
}

void TerrainComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(0 == AttribId);

	shader->SetMatrix("g_World", m_World);

	if (m_Material)
	{
		m_Material->OnSetShader(shader, AttribId);
	}
}

void TerrainComponent::AddToPipeline(RenderPipeline * pipeline, unsigned int PassMask)
{
	if (m_Material && (m_Material->m_PassMask & PassMask))
	{
		for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
		{
			if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
			{
				my::Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeStatic, false, m_Material.get(), PassID);
				if (shader)
				{
					pipeline->PushIndexedPrimitive(
						PassID, m_Decl, m_vb.m_ptr, m_ib.m_ptr, D3DPT_TRIANGLELIST, 0, 0, (m_XDivision + 1) * (m_ZDivision + 1), m_VertexStride, 0, m_XDivision * m_ZDivision * 2, 0, shader, this);
				}
			}
		}
	}
}
