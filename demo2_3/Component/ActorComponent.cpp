#include "StdAfx.h"
#include "ActorComponent.h"
#include "Animator.h"

using namespace my;

void RenderComponent::OnQueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
}

void RenderComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
}

void MeshComponent::OnQueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	pipeline->PushComponent<MeshComponent>(this, m_Animator ? RenderPipeline::MeshTypeAnimation : RenderPipeline::MeshTypeStatic, PassMask);
}

void MeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(m_Mesh);
	_ASSERT(AttribId < m_MaterialList.size());

	shader->SetMatrix("g_World", m_World);

	if (m_Animator && !m_Animator->m_DualQuats.empty())
	{
		shader->SetMatrixArray("g_dualquat", &m_Animator->m_DualQuats[0], m_Animator->m_DualQuats.size());
	}

	m_MaterialList[AttribId]->OnSetShader(shader, AttribId);
}

my::RayResult MeshComponent::RayTest(const my::Ray & ray) const
{
	if (m_Mesh)
	{
		my::Ray local_ray = ray.transform(m_World.inverse());
		if (m_Animator && !m_Animator->m_DualQuats.empty())
		{
			std::vector<Vector3> vertices(m_Mesh->GetNumVertices());
			my::D3DVertexElementSet elems;
			elems.InsertPositionElement(0);
			OgreMesh::ComputeDualQuaternionSkinnedVertices(
				&vertices[0],
				vertices.size(),
				sizeof(vertices[0]),
				elems,
				m_Mesh->LockVertexBuffer(),
				m_Mesh->GetNumBytesPerVertex(),
				m_Mesh->m_VertexElems,
				m_Animator->m_DualQuats);
			my::RayResult ret = OgreMesh::RayTest(local_ray,
				&vertices[0],
				vertices.size(),
				sizeof(vertices[0]),
				m_Mesh->LockIndexBuffer(),
				!(m_Mesh->GetOptions() & D3DXMESH_32BIT),
				m_Mesh->GetNumFaces(),
				elems);
			m_Mesh->UnlockVertexBuffer();
			m_Mesh->UnlockIndexBuffer();
			ret.second = (local_ray.d * ret.second).transformNormal(m_World).magnitude();
			return ret;
		}
		else
		{
			my::RayResult ret = OgreMesh::RayTest(local_ray,
				m_Mesh->LockVertexBuffer(),
				m_Mesh->GetNumVertices(),
				m_Mesh->GetNumBytesPerVertex(),
				m_Mesh->LockIndexBuffer(),
				!(m_Mesh->GetOptions() & D3DXMESH_32BIT),
				m_Mesh->GetNumFaces(),
				m_Mesh->m_VertexElems);
			m_Mesh->UnlockVertexBuffer();
			m_Mesh->UnlockIndexBuffer();
			ret.second = (local_ray.d * ret.second).transformNormal(m_World).magnitude();
			return ret;
		}
	}
	return my::RayResult(false, FLT_MAX);
}

bool MeshComponent::FrustumTest(const my::Frustum & frustum) const
{
	if (m_Mesh)
	{
		my::Frustum local_ftm = frustum.transform(m_World.transpose());
		if (m_Animator && !m_Animator->m_DualQuats.empty())
		{
			std::vector<Vector3> vertices(m_Mesh->GetNumVertices());
			my::D3DVertexElementSet elems;
			elems.InsertPositionElement(0);
			OgreMesh::ComputeDualQuaternionSkinnedVertices(
				&vertices[0],
				vertices.size(),
				sizeof(vertices[0]),
				elems,
				m_Mesh->LockVertexBuffer(),
				m_Mesh->GetNumBytesPerVertex(),
				m_Mesh->m_VertexElems,
				m_Animator->m_DualQuats);
			bool ret = OgreMesh::FrustumTest(local_ftm,
				&vertices[0],
				vertices.size(),
				sizeof(vertices[0]),
				m_Mesh->LockIndexBuffer(),
				!(m_Mesh->GetOptions() & D3DXMESH_32BIT),
				m_Mesh->GetNumFaces(),
				elems);
			m_Mesh->UnlockVertexBuffer();
			m_Mesh->UnlockIndexBuffer();
			return ret;
		}
		else
		{
			bool ret = OgreMesh::FrustumTest(local_ftm,
				m_Mesh->LockVertexBuffer(),
				m_Mesh->GetNumVertices(),
				m_Mesh->GetNumBytesPerVertex(),
				m_Mesh->LockIndexBuffer(),
				!(m_Mesh->GetOptions() & D3DXMESH_32BIT),
				m_Mesh->GetNumFaces(),
				m_Mesh->m_VertexElems);
			m_Mesh->UnlockVertexBuffer();
			m_Mesh->UnlockIndexBuffer();
			return ret;
		}
	}
	return false;
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

void IndexdPrimitiveUPComponent::OnQueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	pipeline->PushComponent<IndexdPrimitiveUPComponent>(this, m_Animator ? RenderPipeline::MeshTypeAnimation : RenderPipeline::MeshTypeStatic, PassMask);
}

void IndexdPrimitiveUPComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(!m_VertexData.empty());
	_ASSERT(AttribId < m_AttribTable.size());

	shader->SetMatrix("g_World", m_World);

	if (m_Animator && !m_Animator->m_DualQuats.empty())
	{
		shader->SetMatrixArray("g_dualquat", &m_Animator->m_DualQuats[0], m_Animator->m_DualQuats.size());
	}

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

void ClothComponent::Update(float fElapsedTime)
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
					my::Vector3 pos = dualQuaternionList.TransformVertexWithDualQuaternionList(
						(my::Vector3 &)m_particles[i].pos, // ! hack position for binding pos
						m_VertexElems.GetBlendIndices(pVertex),
						m_VertexElems.GetBlendWeight(pVertex));
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

void EmitterComponent::OnQueryComponent(const my::Frustum & frustum, RenderPipeline * pipeline, unsigned int PassMask)
{
	pipeline->PushComponent(this, RenderPipeline::MeshTypeParticle, PassMask);
}

void EmitterComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(0 == AttribId);
	shader->SetMatrix("g_World", m_World);
	m_Material->OnSetShader(shader, AttribId);
}
