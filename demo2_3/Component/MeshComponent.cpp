#include "StdAfx.h"
#include "MeshComponent.h"

using namespace my;

void MeshComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	if (m_lodId >= 0 && m_lodId < m_lods.size())
	{
		LOD * lod = m_lods[m_lodId].get();
		for (DWORD i = 0; i < lod->m_Materials.size(); i++)
		{
			my::Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeStatic, stage, lod->m_Materials[i].get());
			if (shader)
			{
				pipeline->PushOpaqueMesh(lod->m_Mesh.get(), i, shader, this);
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
		if (AttribId < lod->m_Materials.size())
		{
			shader->SetTexture("g_MeshTexture", lod->m_Materials[AttribId]->m_DiffuseTexture);
		}
	}
}

void SkeletonMeshComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	if (m_lodId >= 0 && m_lodId < m_lods.size())
	{
		LOD * lod = m_lods[m_lodId].get();
		for (DWORD i = 0; i < lod->m_Materials.size(); i++)
		{
			my::Effect * shader = pipeline->QueryShader(RenderPipeline::MeshTypeAnimation, stage, lod->m_Materials[i].get());
			if (shader)
			{
				pipeline->PushOpaqueMesh(lod->m_Mesh.get(), i, shader, this);
			}
		}
	}
}

void SkeletonMeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	shader->SetMatrixArray("g_dualquat", &m_DualQuats[0], m_DualQuats.size());

	MeshComponent::OnSetShader(shader, AttribId);
}
