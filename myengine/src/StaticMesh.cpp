#include "StaticMesh.h"
#include "Actor.h"
#include "myEffect.h"
#include "Material.h"
#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT(StaticMesh)

template<class Archive>
void StaticMesh::save(Archive& ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(MeshComponent);
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	DWORD ChunkSize = m_Chunks.size();
	ar << BOOST_SERIALIZATION_NVP(ChunkSize);
	StaticMeshChunkList::const_iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		ar << boost::serialization::make_nvp("m_chunk_submesh_id", chunk_iter->m_SubMeshId);
		ar << boost::serialization::make_nvp("m_chunk_aabb", *chunk_iter->m_OctAabb);
	}
}

template<class Archive>
void StaticMesh::load(Archive& ar, const unsigned int version)
{
	ClearAllEntity();
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(MeshComponent);
	ar >> BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	DWORD ChunkSize;
	ar >> BOOST_SERIALIZATION_NVP(ChunkSize);
	for (int i = 0; i < (int)ChunkSize; i++)
	{
		int SubMeshId; AABB aabb;
		ar >> boost::serialization::make_nvp("m_chunk_submesh_id", SubMeshId);
		ar >> boost::serialization::make_nvp("m_chunk_aabb", aabb);
		m_Chunks.push_back(StaticMeshChunk(SubMeshId));
		AddEntity(&m_Chunks.back(), aabb, m_ChunkWidth, 0.01f);
	}
}

void StaticMesh::OnSetShader(IDirect3DDevice9* pd3dDevice, my::Effect* shader, LPARAM lparam)
{
	_ASSERT(m_Actor);

	shader->SetMatrix(handle_World, m_Actor->m_World);

	shader->SetVector(handle_MeshColor, m_MeshColor);
}

void StaticMesh::Update(float fElapsedTime)
{
}

my::AABB StaticMesh::CalculateAABB(void) const
{
	if (!m_Mesh || m_Chunks.empty())
	{
		return Component::CalculateAABB();
	}

	AABB ret(AABB::Invalid());
	StaticMeshChunkList::const_iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		ret.unionSelf(m_Mesh->CalculateAABB(chunk_iter->m_SubMeshId));
	}
	return ret;
}

void StaticMesh::AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos)
{
	_ASSERT(m_Actor);

	if (m_Mesh)
	{
		if (m_Material && (m_Material->m_PassMask & PassMask))
		{
			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				if (RenderPipeline::PassTypeToMask(PassID) & (m_Material->m_PassMask & PassMask))
				{
					my::Effect* shader = pipeline->QueryShader(RenderPipeline::MeshTypeMesh, NULL, m_Material->m_Shader.c_str(), PassID);
					if (shader)
					{
						if (!handle_World)
						{
							BOOST_VERIFY(handle_World = shader->GetParameterByName(NULL, "g_World"));
							BOOST_VERIFY(handle_MeshColor = shader->GetParameterByName(NULL, "g_MeshColor"));
						}

						StaticMeshChunkList::const_iterator chunk_iter = m_Chunks.begin();
						for (; chunk_iter != m_Chunks.end(); chunk_iter++)
						{
							pipeline->PushMeshBatch(PassID, m_Mesh.get(), chunk_iter->m_SubMeshId, shader, this, m_Material.get(), 0);
						}
					}
				}
			}
		}
	}
}
