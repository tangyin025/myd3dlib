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

using namespace my;

StaticMeshChunk::StaticMeshChunk(int SubMeshId)
	: m_SubMeshId(SubMeshId)
	, m_Lod(StaticMesh::LastLod - 1)
{
}

template<class Archive>
void StaticMesh::save(Archive& ar, const unsigned int version) const
{
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(MeshComponent);
	ar << BOOST_SERIALIZATION_BASE_OBJECT_NVP(OctRoot);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkWidth);
	ar << BOOST_SERIALIZATION_NVP(m_ChunkLodScale);
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
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkWidth);
	ar >> BOOST_SERIALIZATION_NVP(m_ChunkLodScale);
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
	AABB ret(AABB::Invalid());
	ChunkMap::const_iterator chunk_iter = m_Chunks.begin();
	for (; chunk_iter != m_Chunks.end(); chunk_iter++)
	{
		ret.unionSelf(*chunk_iter->second.m_OctAabb);
	}
	return ret;
}

void StaticMesh::AddToPipeline(const my::Frustum& frustum, RenderPipeline* pipeline, unsigned int PassMask, const my::Vector3& ViewPos, const my::Vector3& TargetPos)
{
	_ASSERT(m_Actor);

	struct Callback : public my::OctNode::QueryCallback
	{
		RenderPipeline* pipeline;
		unsigned int PassMask;
		const Vector3& LocalViewPos;
		StaticMesh* mesh_cmp;
		Callback(RenderPipeline* _pipeline, unsigned int _PassMask, const Vector3& _LocalViewPos, StaticMesh* _mesh_cmp)
			: pipeline(_pipeline)
			, PassMask(_PassMask)
			, LocalViewPos(_LocalViewPos)
			, mesh_cmp(_mesh_cmp)
		{
		}
		virtual bool OnQueryEntity(my::OctEntity* oct_entity, const my::AABB& aabb, my::IntersectionTests::IntersectionType)
		{
			StaticMeshChunk* chunk = dynamic_cast<StaticMeshChunk*>(oct_entity);
			if (PassMask & RenderPipeline::PassTypeToMask(RenderPipeline::PassTypeNormal))
			{
				chunk->m_Lod = mesh_cmp->m_Actor->CalculateLod((chunk->m_OctAabb->Center() - LocalViewPos).magnitude() / mesh_cmp->m_ChunkLodScale);
			}

			if (chunk->m_Lod >= LastLod)
			{
				return true;
			}

			for (unsigned int PassID = 0; PassID < RenderPipeline::PassTypeNum; PassID++)
			{
				if (RenderPipeline::PassTypeToMask(PassID) & (mesh_cmp->m_Material->m_PassMask & PassMask))
				{
					my::Effect* shader = pipeline->QueryShader(RenderPipeline::MeshTypeMesh, NULL, mesh_cmp->m_Material->m_Shader.c_str(), PassID);
					if (shader)
					{
						if (!mesh_cmp->handle_World)
						{
							BOOST_VERIFY(mesh_cmp->handle_World = shader->GetParameterByName(NULL, "g_World"));
							BOOST_VERIFY(mesh_cmp->handle_MeshColor = shader->GetParameterByName(NULL, "g_MeshColor"));
						}

						pipeline->PushMeshBatch(PassID, mesh_cmp->m_Mesh.get(), chunk->m_SubMeshId, shader, mesh_cmp, mesh_cmp->m_Material.get(), 0);
					}
				}
			}
			return true;
		}
	};

	if (m_Mesh)
	{
		if (m_Material && (m_Material->m_PassMask & PassMask))
		{
			Frustum LocalFrustum = frustum.transform(m_Actor->m_World.transpose());
			Vector3 LocalViewPos = TargetPos.transformCoord(m_Actor->m_World.inverse());
			Callback cb(pipeline, PassMask, LocalViewPos, this);
			QueryEntity(LocalFrustum, &cb);
		}
	}
}

void StaticMesh::AddChunk(int SubMeshId, const my::AABB& aabb)
{
	std::pair<ChunkMap::iterator, bool> chunk_res = m_Chunks.insert(std::make_pair(SubMeshId, StaticMeshChunk(SubMeshId)));
	_ASSERT(chunk_res.second);
	AddEntity(&chunk_res.first->second, aabb, m_ChunkWidth, 0.01f);
}
