#include "StdAfx.h"
#include "ActorComponent.h"

using namespace my;

void MeshComponent::MeshLOD::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage, RenderPipeline::MeshType mesh_type)
{
	if (m_Mesh)
	{
		if (m_Material)
		{
			my::Effect * shader = m_Material->QueryShader(pipeline, stage, mesh_type, m_bInstance);
			if (shader)
			{
				if (m_bInstance)
				{
					pipeline->PushOpaqueMeshInstance(m_Mesh.get(), m_AttribId, m_owner->m_World, shader, m_owner);
				}
				else
				{
					pipeline->PushOpaqueMesh(m_Mesh.get(), m_AttribId, shader, m_owner);
				}
			}
		}
	}
}

void MeshComponent::MeshLOD::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(m_Mesh);
	_ASSERT(AttribId == m_AttribId);
	m_Material->OnSetShader(shader, AttribId);
}

void MeshComponent::IndexdPrimitiveUPLOD::OnResetDevice(void)
{
}

void MeshComponent::IndexdPrimitiveUPLOD::OnLostDevice(void)
{
}

void MeshComponent::IndexdPrimitiveUPLOD::OnDestroyDevice(void)
{
	m_Decl.Release();
}

void MeshComponent::IndexdPrimitiveUPLOD::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage, RenderPipeline::MeshType mesh_type)
{
	if (!m_VertexData.empty())
	{
		_ASSERT(0 != m_VertexStride);
		_ASSERT(!m_IndexData.empty());
		if (m_Material)
		{
			my::Effect * shader = m_Material->QueryShader(pipeline, stage, mesh_type, false);
			if (shader)
			{
				pipeline->PushOpaqueIndexedPrimitiveUP(m_Decl, D3DPT_TRIANGLELIST,
					m_AttribRange.VertexStart,
					m_AttribRange.VertexCount,
					m_AttribRange.FaceCount,
					&m_IndexData[m_AttribRange.FaceStart * 3],
					D3DFMT_INDEX16,
					&m_VertexData[0],
					m_VertexStride,
					m_AttribRange.AttribId, shader, m_owner);
			}
		}
	}
}

void MeshComponent::IndexdPrimitiveUPLOD::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(!m_VertexData.empty());
	_ASSERT(AttribId == m_AttribRange.AttribId);
	m_Material->OnSetShader(shader, AttribId);
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
	switch (m_WorldType)
	{
	case WorldTypeLocal:
		shader->SetMatrix("g_World", m_World);
		break;

	default:
		shader->SetMatrix("g_World", Matrix4::identity);
		break;
	}

	Vector3 Up, Right, Dir;
	const Matrix4 View = shader->GetMatrix("g_View");
	switch (m_DirectionType)
	{
	case DirectionTypeCamera:
		Dir = View.column<2>().xyz;
		Up = View.column<1>().xyz;
		Right = View.column<0>().xyz;
		break;

	case DirectionTypeVertical:
		Up = Vector3(0,1,0);
		Right = Up.cross(View.column<2>().xyz);
		Dir = Right.cross(Up);
		break;
	}

	shader->SetVector("g_ParticleDir", Dir);
	shader->SetVector("g_ParticleUp", Up);
	shader->SetVector("g_ParticleRight", Right);

	m_Material->OnSetShader(shader, AttribId);
}
