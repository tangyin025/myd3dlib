#include "StdAfx.h"
#include "ActorComponent.h"
#include "Animator.h"

using namespace my;

void MeshComponent::QueryMeshWithMeshType(RenderPipeline * pipeline, unsigned int PassMask, Material::MeshType mesh_type)
{
	if (m_Mesh)
	{
		for (DWORD i = 0; i < m_MaterialList.size(); i++)
		{
			if (m_MaterialList[i] && (PassMask &= m_MaterialList[i]->m_PassMask))
			{
				for (unsigned int PassID = 0; PassID < Material::PassTypeNum; PassID++)
				{
					if (Material::PassIDToMask(PassID) & PassMask)
					{
						my::Effect * shader = pipeline->QueryShader(mesh_type, PassID, m_bInstance, m_MaterialList[i].get());
						if (shader)
						{
							if (m_bInstance)
							{
								pipeline->PushMeshInstance(PassID, m_Mesh.get(), i, m_World, shader, this);
							}
							else
							{
								pipeline->PushMesh(PassID, m_Mesh.get(), i, shader, this);
							}
						}
					}
				}
			}
		}
	}
}

void MeshComponent::QueryMesh(RenderPipeline * pipeline, unsigned int PassMask)
{
	QueryMeshWithMeshType(pipeline, PassMask, Material::MeshTypeStatic);
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
	QueryMeshWithMeshType(pipeline, PassMask, Material::MeshTypeAnimation);
}

void SkeletonMeshComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	_ASSERT(m_Owner);
	if (m_AnimId < m_Owner->m_AnimatorList.size())
	{
		const TransformList & DualQuats = m_Owner->m_AnimatorList[m_AnimId]->m_DualQuats;
		if (!DualQuats.empty())
		{
			shader->SetMatrixArray("g_dualquat", &DualQuats[0], DualQuats.size());
		}
	}

	MeshComponent::OnSetShader(shader, AttribId);
}

void IndexdPrimitiveUPComponent::QueryMesh(RenderPipeline * pipeline, unsigned int PassMask)
{
	for (unsigned int i = 0; i < m_AttribTable.size(); i++)
	{
		_ASSERT(!m_VertexData.empty());
		_ASSERT(!m_IndexData.empty());
		_ASSERT(0 != m_VertexStride);
		if (m_MaterialList[i] && (PassMask &= m_MaterialList[i]->m_PassMask))
		{
			for (unsigned int PassID = 0; PassID < Material::PassTypeNum; PassID++)
			{
				if (Material::PassIDToMask(PassID) & PassMask)
				{
					my::Effect * shader = pipeline->QueryShader(Material::MeshTypeStatic, PassID, false, m_MaterialList[i].get());
					if (shader)
					{
						pipeline->PushIndexedPrimitiveUP(PassID, m_Decl, D3DPT_TRIANGLELIST,
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
	if (m_AnimId < m_Owner->m_AnimatorList.size())
	{
		const TransformList & DualQuats = m_Owner->m_AnimatorList[m_AnimId]->m_DualQuats;
		if (!DualQuats.empty())
		{
			UpdateCloth(DualQuats);
		}
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
	if (m_Material && m_Emitter && (PassMask &= m_Material->m_PassMask))
	{
		for (unsigned int PassID = 0; PassID < Material::PassTypeNum; PassID++)
		{
			if (Material::PassIDToMask(PassID) & PassMask)
			{
				my::Effect * shader = pipeline->QueryShader(Material::MeshTypeParticle, PassID, false, m_Material.get());
				if (shader)
				{
					pipeline->PushEmitter(PassID, m_Emitter.get(), 0, shader, this);
				}
			}
		}
	}
}

void EmitterComponent::OnSetShader(my::Effect * shader, DWORD AttribId)
{
	shader->SetMatrix("g_World", m_World);

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
