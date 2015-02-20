#include "StdAfx.h"
#include "MeshComponent.h"

using namespace my;

void Material::OnQueryMesh(
	RenderPipeline * pipeline,
	RenderPipeline::DrawStage stage,
	RenderPipeline::MeshType mesh_type,
	my::Mesh * mesh,
	DWORD AttribId,
	RenderPipeline::IShaderSetter * setter)
{
	my::Effect * shader = pipeline->QueryShader(mesh_type, stage, this);
	if (shader)
	{
		pipeline->PushOpaqueMesh(mesh, AttribId, shader, setter);
	}
}

void MeshComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	if (m_lodId >= 0 && m_lodId < m_lods.size())
	{
		LOD * lod = m_lods[m_lodId].get();
		for (DWORD i = 0; i < lod->m_Materials.size(); i++)
		{
			lod->m_Materials[i]->OnQueryMesh(pipeline, stage, RenderPipeline::MeshTypeStatic, lod->m_Mesh.get(), i, this);
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
			lod->m_Materials[i]->OnQueryMesh(pipeline, stage, RenderPipeline::MeshTypeAnimation, lod->m_Mesh.get(), i, this);
		}
	}
}

void SkeletonMeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	shader->SetMatrixArray("g_dualquat", &m_DualQuats[0], m_DualQuats.size());

	MeshComponent::OnSetShader(shader, AttribId);
}
