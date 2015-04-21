#include "StdAfx.h"
#include "ActorComponent.h"
#include "Animator.h"

using namespace my;

void MeshComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage, RenderPipeline::MeshType mesh_type)
{
	if (m_Mesh)
	{
		for (DWORD i = 0; i < m_MaterialList.size(); i++)
		{
			if (m_MaterialList[i])
			{
				my::Effect * shader = m_MaterialList[i]->QueryShader(pipeline, stage, mesh_type, m_bInstance);
				if (shader)
				{
					if (m_bInstance)
					{
						pipeline->PushOpaqueMeshInstance(m_Mesh.get(), i, m_World, shader, this);
					}
					else
					{
						pipeline->PushOpaqueMesh(m_Mesh.get(), i, shader, this);
					}
				}
			}
		}
	}
}

void MeshComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	QueryMesh(pipeline, stage, RenderPipeline::MeshTypeStatic);
}

void MeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(m_Mesh);
	_ASSERT(AttribId < m_MaterialList.size());
	shader->SetMatrix("g_World", m_World);
	m_MaterialList[AttribId]->OnSetShader(shader, AttribId);
}

void SkeletonMeshComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	MeshComponent::QueryMesh(pipeline, stage, RenderPipeline::MeshTypeAnimation);
}

void SkeletonMeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(m_Owner);
	if (m_Owner->m_Animator)
	{
		shader->SetMatrixArray("g_dualquat", &m_Owner->m_Animator->m_DualQuats[0], m_Owner->m_Animator->m_DualQuats.size());
	}

	MeshComponent::OnSetShader(shader, AttribId);
}

void IndexdPrimitiveUPComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	for (unsigned int i = 0; i < m_AttribTable.size(); i++)
	{
		_ASSERT(!m_VertexData.empty());
		_ASSERT(!m_IndexData.empty());
		_ASSERT(0 != m_VertexStride);
		if (m_MaterialList[i])
		{
			my::Effect * shader = m_MaterialList[i]->QueryShader(pipeline, stage, RenderPipeline::MeshTypeStatic, false);
			if (shader)
			{
				pipeline->PushOpaqueIndexedPrimitiveUP(m_Decl, D3DPT_TRIANGLELIST,
					m_AttribTable[i].VertexStart,
					m_AttribTable[i].VertexCount,
					m_AttribTable[i].FaceCount,
					&m_IndexData[m_AttribTable[i].FaceStart * 3],
					D3DFMT_INDEX16,
					&m_VertexData[0],
					m_VertexStride, i, shader, this);
			}
		}
	}
}

void IndexdPrimitiveUPComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(!m_VertexData.empty());
	_ASSERT(AttribId < m_AttribTable.size());
	shader->SetMatrix("g_World", m_World);
	m_MaterialList[AttribId]->OnSetShader(shader, AttribId);
}

void ClothComponent::OnPxThreadSubstep(float fElapsedTime)
{
	_ASSERT(m_Owner);
	if (m_Owner->m_Animator)
	{
		UpdateCloth(m_Owner->m_Animator->m_DualQuats);
	}
}

void ClothComponent::UpdateCloth(const my::TransformList & dualQuaternionList)
{
	if (m_Cloth)
	{
		_ASSERT(m_particles.size() == m_VertexData.size() / m_VertexStride);
		PxClothReadData * readData = m_Cloth->lockClothReadData();
		if (readData)
		{
			unsigned char * pVertices = &m_VertexData[0];
			const DWORD NbParticles = m_Cloth->getNbParticles();
			m_NewParticles.resize(NbParticles);
			for (unsigned int i = 0; i < NbParticles; i++)
			{
				void * pVertex = pVertices + i * m_VertexStride;
				m_NewParticles[i].invWeight = readData->particles[i].invWeight;
				if (0 == m_NewParticles[i].invWeight)
				{
					my::Vector3 pos;
					my::TransformList::TransformVertexWithDualQuaternionList(pos,
						(my::Vector3 &)m_particles[i].pos,
						m_VertexElems.GetBlendIndices(pVertex),
						m_VertexElems.GetBlendWeight(pVertex), dualQuaternionList);
					m_NewParticles[i].pos = (PxVec3 &)pos;
				}
				else
				{
					m_NewParticles[i].pos = readData->particles[i].pos;
				}
				m_VertexElems.SetPosition(pVertex, (my::Vector3 &)m_NewParticles[i].pos);
			}
			readData->unlock();
			m_Cloth->setParticles(&m_NewParticles[0], NULL);
			m_Cloth->setTargetPose(PxTransform((PxMat44 &)m_World));

			my::OgreMesh::ComputeNormalFrame(
				pVertices, NbParticles, m_VertexStride, &m_IndexData[0], true, m_IndexData.size() / 3, m_VertexElems);

			my::OgreMesh::ComputeTangentFrame(
				pVertices, NbParticles, m_VertexStride, &m_IndexData[0], true, m_IndexData.size() / 3, m_VertexElems);
		}
	}
}

void EmitterComponent::Update(float fElapsedTime)
{
	if (m_Emitter)
	{
		m_Emitter->Update(fElapsedTime);
	}
}

void EmitterComponent::QueryMesh(RenderPipeline * pipeline, RenderPipeline::DrawStage stage)
{
	if (m_Material && m_Emitter)
	{
		my::Effect * shader = m_Material->QueryShader(pipeline, stage, RenderPipeline::MeshTypeParticle, false);
		if (shader)
		{
			pipeline->PushOpaqueEmitter(m_Emitter.get(), 0, shader, this);
		}
	}
}

void EmitterComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
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

	_ASSERT(0 == AttribId);
	m_Material->OnSetShader(shader, AttribId);
}
