#include "StdAfx.h"
#include "MeshComponent.h"

using namespace my;

void MeshComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	if (m_lodId >= 0 && m_lodId < m_lods.size())
	{
		LOD * lod = m_lods[m_lodId].get();
		if (lod->m_Mesh)
		{
			for (DWORD i = 0; i < lod->m_Materials.size(); i++)
			{
				if (lod->m_Materials[i])
				{
					lod->m_Materials[i]->OnQueryMesh(pipeline, stage, RenderPipeline::MeshTypeStatic, lod->m_Mesh.get(), i, this);
				}
			}
		}
	}
}

void MeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	shader->SetMatrix("g_World", m_World);
	if (m_lodId >= 0 && m_lodId < m_lods.size())
	{
		LOD * lod = m_lods[m_lodId].get();
		if (lod->m_Mesh)
		{
			if (AttribId < lod->m_Materials.size())
			{
				lod->m_Materials[AttribId]->OnSetShader(shader, AttribId);
			}
		}
	}
}

void SkeletonMeshComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	if (m_lodId >= 0 && m_lodId < m_lods.size())
	{
		LOD * lod = m_lods[m_lodId].get();
		if (lod->m_Mesh)
		{
			for (DWORD i = 0; i < lod->m_Materials.size(); i++)
			{
				if (lod->m_Materials[i])
				{
					lod->m_Materials[i]->OnQueryMesh(pipeline, stage, RenderPipeline::MeshTypeAnimation, lod->m_Mesh.get(), i, this);
				}
			}
		}
	}
}

void SkeletonMeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	if (m_Animator)
	{
		shader->SetMatrixArray("g_dualquat", m_Animator->GetDualQuats(), m_Animator->GetDualQuatsNum());
	}

	MeshComponent::OnSetShader(shader, AttribId);
}

void DeformationMeshComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	if (!m_VertexData.empty())
	{
		_ASSERT(!m_IndexData.empty());
		_ASSERT(m_VertexStride > 0);
		_ASSERT(m_Decl);
		for (DWORD i = 0; i < m_AttribTable.size(); i++)
		{
			if (i < m_Materials.size() && m_Materials[i])
			{
				m_Materials[i]->OnQueryIndexedPrimitiveUP(pipeline, stage, m_Decl, D3DPT_TRIANGLELIST,
					m_AttribTable[i].VertexStart,
					m_AttribTable[i].VertexCount,
					m_AttribTable[i].FaceCount,
					&m_IndexData[m_AttribTable[i].FaceStart * 3],
					D3DFMT_INDEX16,
					&m_VertexData[0],
					m_VertexStride, i, this);
			}
		}
	}
}

void DeformationMeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	shader->SetMatrix("g_World", m_World);
	m_Materials[AttribId]->OnSetShader(shader, AttribId);
}

void DeformationMeshComponent::CreateFromOgreMeshWithoutMaterials(IDirect3DDevice9 * pd3dDevice, my::OgreMeshPtr mesh)
{
	m_VertexData.resize(mesh->GetNumVertices() * mesh->GetNumBytesPerVertex());
	memcpy(&m_VertexData[0], mesh->LockVertexBuffer(), m_VertexData.size());
	mesh->UnlockVertexBuffer();

	m_IndexData.resize(mesh->GetNumFaces() * 3);
	if (m_IndexData.size() > USHRT_MAX)
	{
		THROW_CUSEXCEPTION(str_printf(_T("create deformation mesh with invalid index size %u"), m_IndexData.size()));
	}
	VOID * pIndices = mesh->LockIndexBuffer();
	for (unsigned int face_i = 0; face_i < mesh->GetNumFaces(); face_i++)
	{
		if(mesh->GetOptions() & D3DXMESH_32BIT)
		{
			m_IndexData[face_i * 3 + 0] = (WORD)*((DWORD *)pIndices + face_i * 3 + 0);
			m_IndexData[face_i * 3 + 1] = (WORD)*((DWORD *)pIndices + face_i * 3 + 1);
			m_IndexData[face_i * 3 + 2] = (WORD)*((DWORD *)pIndices + face_i * 3 + 2);
		}
		else
		{
			m_IndexData[face_i * 3 + 0] = *((WORD *)pIndices + face_i * 3 + 0);
			m_IndexData[face_i * 3 + 1] = *((WORD *)pIndices + face_i * 3 + 1);
			m_IndexData[face_i * 3 + 2] = *((WORD *)pIndices + face_i * 3 + 2);
		}
	}
	mesh->UnlockIndexBuffer();

	DWORD submeshes = 0;
	mesh->GetAttributeTable(NULL, &submeshes);
	m_AttribTable.resize(submeshes);
	mesh->GetAttributeTable(&m_AttribTable[0], &submeshes);
	m_VertexElems = mesh->m_VertexElems;
	m_VertexStride = mesh->GetNumBytesPerVertex();

	std::vector<D3DVERTEXELEMENT9> ielist(MAX_FVF_DECL_SIZE);
	mesh->GetDeclaration(&ielist[0]);
	HRESULT hr;
	if (FAILED(hr = pd3dDevice->CreateVertexDeclaration(&ielist[0], &m_Decl)))
	{
		THROW_D3DEXCEPTION(hr);
	}
}
