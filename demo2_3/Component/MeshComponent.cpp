#include "StdAfx.h"
#include "MeshComponent.h"

using namespace my;

void MeshComponent::LOD::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage, RenderPipeline::MeshType mesh_type)
{
	if (m_Mesh)
	{
		for (DWORD i = 0; i < m_Materials.size(); i++)
		{
			if (m_Materials[i])
			{
				m_Materials[i]->OnQueryMesh(pipeline, stage, mesh_type, m_Mesh.get(), i, m_owner);
			}
		}
	}
}

void MeshComponent::LOD::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	if (m_Mesh)
	{
		if (AttribId < m_Materials.size())
		{
			m_Materials[AttribId]->OnSetShader(shader, AttribId);
		}
	}
}

void MeshComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	if (m_lodId >= 0 && m_lodId < m_lods.size())
	{
		m_lods[m_lodId]->QueryMesh(pipeline, stage, RenderPipeline::MeshTypeStatic);
	}
}

void MeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	shader->SetMatrix("g_World", m_World);
	if (m_lodId >= 0 && m_lodId < m_lods.size())
	{
		m_lods[m_lodId]->OnSetShader(shader, AttribId);
	}
}

void SkeletonMeshComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	if (m_lodId >= 0 && m_lodId < m_lods.size())
	{
		m_lods[m_lodId]->QueryMesh(pipeline, stage, RenderPipeline::MeshTypeAnimation);
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
