#include "StdAfx.h"
#include "ActorComponent.h"
#include "Animator.h"

using namespace my;

void MeshComponent::QueryMesh(RenderPipeline * pipeline, unsigned int PassMask)
{
	pipeline->PushComponent<MeshComponent>(this, RenderPipeline::MeshTypeStatic, PassMask);
}

void MeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(m_Mesh);
	_ASSERT(AttribId < m_MaterialList.size());
	shader->SetMatrix("g_World", m_World);
	m_MaterialList[AttribId]->OnSetShader(shader, AttribId);
}

void SkeletonMeshComponent::QueryMesh(RenderPipeline * pipeline, unsigned int PassMask)
{
	pipeline->PushComponent<MeshComponent>(this, RenderPipeline::MeshTypeAnimation, PassMask);
}

void SkeletonMeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	if (m_Animator && !m_Animator->m_DualQuats.empty())
	{
		shader->SetMatrixArray("g_dualquat", &m_Animator->m_DualQuats[0], m_Animator->m_DualQuats.size());
	}

	MeshComponent::OnSetShader(shader, AttribId);
}

void IndexdPrimitiveUPComponent::OnResetDevice(void)
{
}

void IndexdPrimitiveUPComponent::OnLostDevice(void)
{
}

void IndexdPrimitiveUPComponent::OnDestroyDevice(void)
{
	m_Decl.Release();
}

void IndexdPrimitiveUPComponent::QueryMesh(RenderPipeline * pipeline, unsigned int PassMask)
{
	pipeline->PushComponent<IndexdPrimitiveUPComponent>(this, RenderPipeline::MeshTypeStatic, PassMask);
}

void IndexdPrimitiveUPComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(!m_VertexData.empty());
	_ASSERT(AttribId < m_AttribTable.size());
	shader->SetMatrix("g_World", m_World);
	m_MaterialList[AttribId]->OnSetShader(shader, AttribId);
}

void ClothComponent::OnResetDevice(void)
{
	IndexdPrimitiveUPComponent::OnResetDevice();
}

void ClothComponent::OnLostDevice(void)
{
	IndexdPrimitiveUPComponent::OnLostDevice();
}

void ClothComponent::OnDestroyDevice(void)
{
	m_Cloth.reset();
	IndexdPrimitiveUPComponent::OnDestroyDevice();
}

void ClothComponent::OnPxThreadSubstep(float fElapsedTime)
{
	if (m_Animator && !m_Animator->m_DualQuats.empty())
	{
		UpdateCloth(m_Animator->m_DualQuats);
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

void EmitterComponent::QueryMesh(RenderPipeline * pipeline, unsigned int PassMask)
{
	pipeline->PushComponent(this, RenderPipeline::MeshTypeParticle, PassMask);
}

void EmitterComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(0 == AttribId);
	shader->SetMatrix("g_World", m_World);
	m_Material->OnSetShader(shader, AttribId);
}
