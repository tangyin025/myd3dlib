#include "StaticMesh.h"
#include "Actor.h"
#include "myEffect.h"
#include "Material.h"
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/export.hpp>

BOOST_CLASS_EXPORT(StaticMesh)

template<class Archive>
void StaticMesh::save(Archive& ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(MeshComponent);
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	DWORD ChunkSize = m_Chunks.size();
	ar << BOOST_SERIALIZATION_NVP(ChunkSize);
	ChunkMap::const_iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		ar << boost::serialization::make_nvp("m_chunk_submesh_id", chunk_iter->first);
		ar << boost::serialization::make_nvp("m_chunk_aabb", *chunk_iter->second.m_OctAabb);
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
		AddChunk(SubMeshId, aabb);
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
	ChunkMap::const_iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		ret.unionSelf(m_Mesh->CalculateAABB(chunk_iter->first));
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

						ChunkMap::const_iterator chunk_iter = m_Chunks.begin();
						for (; chunk_iter != m_Chunks.end(); chunk_iter++)
						{
							pipeline->PushMeshBatch(PassID, m_Mesh.get(), chunk_iter->first, shader, this, m_Material.get(), 0);
						}
					}
				}
			}
		}
	}
}

void StaticMesh::AddChunk(int SubMeshId, const my::AABB& aabb)
{
	std::pair<ChunkMap::iterator, bool> chunk_res = m_Chunks.insert(std::make_pair(SubMeshId, StaticMeshChunk(SubMeshId)));
	_ASSERT(chunk_res.second);
	AddEntity(&chunk_res.first->second, aabb, m_ChunkWidth, 0.01f);
}
