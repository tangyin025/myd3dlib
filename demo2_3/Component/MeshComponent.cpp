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
				my::Effect * shader = m_Materials[i]->QueryShader(pipeline, stage, mesh_type, m_bInstance);
				if (shader)
				{
					if (m_bInstance)
					{
						pipeline->PushOpaqueMeshInstance(m_Mesh.get(), i, m_owner->m_World, shader, m_owner);
					}
					else
					{
						pipeline->PushOpaqueMesh(m_Mesh.get(), i, shader, m_owner);
					}
				}
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

void EmitterMeshComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	if (m_Emitter)
	{
		if (m_Material)
		{
			my::Effect * shader = m_Material->QueryShader(pipeline, stage, RenderPipeline::MeshTypeParticle, true);
			if (shader)
			{
				pipeline->PushOpaqueEmitter(m_Emitter.get(), shader, this);
			}
		}
	}
}

void EmitterMeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	switch (m_Emitter->m_WorldType)
	{
	case my::Emitter::WorldTypeLocal:
		shader->SetMatrix("g_World", Matrix4::Compose(Vector3(1,1,1), m_Emitter->m_Orientation, m_Emitter->m_Position));
		break;
	default:
		shader->SetMatrix("g_World", Matrix4::identity);
		break;
	}

	const Matrix4 View(Matrix4::Identity());
	Vector3 Up, Right, Dir;
	switch (m_Emitter->m_DirectionType)
	{
	case my::Emitter::DirectionTypeCamera:
		Up = View.column<2>().xyz;
		Right = View.column<1>().xyz;
		Dir = View.column<0>().xyz;
		break;

	case my::Emitter::DirectionTypeVertical:
		Up = Vector3(0,1,0);
		Right = Up.cross(Vector3(View._13,View._23,View._33));
		Dir = Right.cross(Up);
		break;
	}
	shader->SetVector("g_ParticleDir", Dir);
	shader->SetVector("g_ParticleUp", Up);
	shader->SetVector("g_ParticleRight", Right);

	m_Material->OnSetShader(shader, AttribId);
}
